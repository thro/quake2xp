/*
* This is an open source non-commercial project. Dear PVS-Studio, please check it.
* PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
*/
// AMD Driver SDK
// Overdrive N 

#include "../ref_gl/r_local.h"


// Definitions of the used function pointers. Add more if you use other ADL APIs
typedef int(*ADL_MAIN_CONTROL_CREATE)(ADL_MAIN_MALLOC_CALLBACK, int);
typedef int(*ADL_MAIN_CONTROL_DESTROY)();
typedef int(*ADL_FLUSH_DRIVER_DATA)(int);
typedef int(*ADL2_ADAPTER_ACTIVE_GET) (ADL_CONTEXT_HANDLE, int, int*);
typedef int(*ADL_ADAPTER_ID_GET) (int iAdapterIndex, int *lpAdapterID);

typedef int(*ADL_ADAPTER_NUMBEROFADAPTERS_GET)		(int*);
typedef int(*ADL_ADAPTER_ADAPTERINFO_GET)			(LPAdapterInfo, int);
typedef int(*ADL_ADAPTERX2_CAPS)					(int, int*);
typedef int(*ADL2_OVERDRIVE_CAPS)					(ADL_CONTEXT_HANDLE context, int iAdapterIndex, int * iSupported, int * iEnabled, int * iVersion);
typedef int(*ADL2_OVERDRIVEN_CAPABILITIES_GET)		(ADL_CONTEXT_HANDLE, int, ADLODNCapabilities*);
typedef int(*ADL2_OVERDRIVEN_SYSTEMCLOCKS_GET)		(ADL_CONTEXT_HANDLE, int, ADLODNPerformanceLevels*);
typedef int(*ADL2_OVERDRIVEN_SYSTEMCLOCKS_SET)		(ADL_CONTEXT_HANDLE, int, ADLODNPerformanceLevels*);
typedef int(*ADL2_OVERDRIVEN_MEMORYCLOCKS_GET)		(ADL_CONTEXT_HANDLE, int, ADLODNPerformanceLevels*);
typedef int(*ADL2_OVERDRIVEN_MEMORYCLOCKS_SET)		(ADL_CONTEXT_HANDLE, int, ADLODNPerformanceLevels*);
typedef int(*ADL2_OVERDRIVEN_PERFORMANCESTATUS_GET) (ADL_CONTEXT_HANDLE, int, ADLODNPerformanceStatus*);
typedef int(*ADL2_OVERDRIVEN_FANCONTROL_GET)		(ADL_CONTEXT_HANDLE, int, ADLODNFanControl*);
typedef int(*ADL2_OVERDRIVEN_FANCONTROL_SET)		(ADL_CONTEXT_HANDLE, int, ADLODNFanControl*);
typedef int(*ADL2_OVERDRIVEN_POWERLIMIT_GET)		(ADL_CONTEXT_HANDLE, int, ADLODNPowerLimitSetting*);
typedef int(*ADL2_OVERDRIVEN_POWERLIMIT_SET)		(ADL_CONTEXT_HANDLE, int, ADLODNPowerLimitSetting*);
typedef int(*ADL2_OVERDRIVEN_TEMPERATURE_GET)		(ADL_CONTEXT_HANDLE, int, int, int*);
typedef int(*ADL2_ADAPTER_MEMORYINFO_GET)			(ADL_CONTEXT_HANDLE, int iAdapterIndex, ADLMemoryInfo *lpMemoryInfo);
HINSTANCE ati_hDLL;

ADL_MAIN_CONTROL_CREATE					ADL_Main_Control_Create = NULL;
ADL_MAIN_CONTROL_DESTROY				ADL_Main_Control_Destroy = NULL;
ADL_ADAPTER_NUMBEROFADAPTERS_GET		ADL_Adapter_NumberOfAdapters_Get = NULL;
ADL_ADAPTER_ADAPTERINFO_GET				ADL_Adapter_AdapterInfo_Get = NULL;
ADL_ADAPTERX2_CAPS						ADL_AdapterX2_Caps = NULL;
ADL_ADAPTER_ID_GET						ADL_Adapter_ID_Get = NULL;

ADL2_ADAPTER_ACTIVE_GET					ADL2_Adapter_Active_Get = NULL;
ADL2_OVERDRIVEN_CAPABILITIES_GET		ADL2_OverdriveN_Capabilities_Get = NULL;
ADL2_OVERDRIVEN_SYSTEMCLOCKS_GET		ADL2_OverdriveN_SystemClocks_Get = NULL;
ADL2_OVERDRIVEN_SYSTEMCLOCKS_SET		ADL2_OverdriveN_SystemClocks_Set = NULL;
ADL2_OVERDRIVEN_PERFORMANCESTATUS_GET	ADL2_OverdriveN_PerformanceStatus_Get = NULL;
ADL2_OVERDRIVEN_FANCONTROL_GET			ADL2_OverdriveN_FanControl_Get = NULL;
ADL2_OVERDRIVEN_FANCONTROL_SET			ADL2_OverdriveN_FanControl_Set = NULL;
ADL2_OVERDRIVEN_POWERLIMIT_GET			ADL2_OverdriveN_PowerLimit_Get = NULL;
ADL2_OVERDRIVEN_POWERLIMIT_SET			ADL2_OverdriveN_PowerLimit_Set = NULL;
ADL2_OVERDRIVEN_MEMORYCLOCKS_GET		ADL2_OverdriveN_MemoryClocks_Get = NULL;
ADL2_OVERDRIVEN_MEMORYCLOCKS_GET		ADL2_OverdriveN_MemoryClocks_Set = NULL;
ADL2_OVERDRIVE_CAPS						ADL2_Overdrive_Caps = NULL;
ADL2_OVERDRIVEN_TEMPERATURE_GET			ADL2_OverdriveN_Temperature_Get = NULL;
ADL2_ADAPTER_MEMORYINFO_GET				ADL2_Adapter_MemoryInfo_Get = NULL;
ADL_CONTEXT_HANDLE context = NULL;

LPAdapterInfo   adapterInfo = NULL;
int				atiPhysicalGpuCount, atiPhysicalAdapters;
qboolean		adlInit;
char			gpuNames[30][128]; // 5 outputs per adapter (x4)

#define AMDVENDORID				(1002)

// Memory allocation function
void* __stdcall ADL_Main_Memory_Alloc(int iSize)
{
	void* lpBuffer = malloc(iSize);
	return lpBuffer;
}

// Optional Memory de-allocation function
void __stdcall ADL_Main_Memory_Free(void** lpBuffer)
{
	if (NULL != *lpBuffer)
	{
		free(*lpBuffer);
		*lpBuffer = NULL;
	}
}

void ADL_Shutdown() {

	ADL_Main_Control_Destroy();
	FreeLibrary(ati_hDLL);
	atiPhysicalGpuCount = 0;
	atiPhysicalAdapters = 0;
}

void GLimp_InitADL(){

	int adapterID, seenids[100], i, ii;
	int seenids_num = 0;
	int iSupported, iEnabled, overDriveVer;
	adlInit = qfalse;
	atiPhysicalGpuCount = atiPhysicalAdapters = 0;

	Com_Printf("" S_COLOR_YELLOW "\n...Initializing AMD Display Library\n");

	// Load the ADL dll
	ati_hDLL = LoadLibrary("atiadlxx.dll");
	if (ati_hDLL == NULL)
	{
		// A 32 bit calling application on 64 bit OS will fail to LoadLibrary.
		// Try to load the 32 bit library (atiadlxy.dll) instead
		ati_hDLL = LoadLibrary("atiadlxy.dll");
	}
	
	if (NULL == ati_hDLL)
	{
		Com_Printf(S_COLOR_MAGENTA"...ADL not found or unsupported\n");
		return;
	}

	ADL_Main_Control_Create =				(ADL_MAIN_CONTROL_CREATE)GetProcAddress(ati_hDLL, "ADL_Main_Control_Create");
	ADL_Main_Control_Destroy =				(ADL_MAIN_CONTROL_DESTROY)GetProcAddress(ati_hDLL, "ADL_Main_Control_Destroy");
	ADL_Adapter_NumberOfAdapters_Get =		(ADL_ADAPTER_NUMBEROFADAPTERS_GET)GetProcAddress(ati_hDLL, "ADL_Adapter_NumberOfAdapters_Get");
	ADL_Adapter_AdapterInfo_Get =			(ADL_ADAPTER_ADAPTERINFO_GET)GetProcAddress(ati_hDLL, "ADL_Adapter_AdapterInfo_Get");
	ADL_AdapterX2_Caps =					(ADL_ADAPTERX2_CAPS)GetProcAddress(ati_hDLL, "ADL_AdapterX2_Caps");
	ADL2_Adapter_Active_Get =				(ADL2_ADAPTER_ACTIVE_GET)GetProcAddress(ati_hDLL, "ADL2_Adapter_Active_Get");
	ADL2_OverdriveN_Capabilities_Get =		(ADL2_OVERDRIVEN_CAPABILITIES_GET)GetProcAddress(ati_hDLL, "ADL2_OverdriveN_Capabilities_Get");
	ADL2_OverdriveN_SystemClocks_Get =		(ADL2_OVERDRIVEN_SYSTEMCLOCKS_GET)GetProcAddress(ati_hDLL, "ADL2_OverdriveN_SystemClocks_Get");
	ADL2_OverdriveN_SystemClocks_Set =		(ADL2_OVERDRIVEN_SYSTEMCLOCKS_SET)GetProcAddress(ati_hDLL, "ADL2_OverdriveN_SystemClocks_Set");
	ADL2_OverdriveN_MemoryClocks_Get =		(ADL2_OVERDRIVEN_MEMORYCLOCKS_GET)GetProcAddress(ati_hDLL, "ADL2_OverdriveN_MemoryClocks_Get");
	ADL2_OverdriveN_MemoryClocks_Set =		(ADL2_OVERDRIVEN_MEMORYCLOCKS_SET)GetProcAddress(ati_hDLL, "ADL2_OverdriveN_MemoryClocks_Set");
	ADL2_OverdriveN_PerformanceStatus_Get = (ADL2_OVERDRIVEN_PERFORMANCESTATUS_GET)GetProcAddress(ati_hDLL, "ADL2_OverdriveN_PerformanceStatus_Get");
	ADL2_OverdriveN_FanControl_Get =		(ADL2_OVERDRIVEN_FANCONTROL_GET)GetProcAddress(ati_hDLL, "ADL2_OverdriveN_FanControl_Get");
	ADL2_OverdriveN_FanControl_Set =		(ADL2_OVERDRIVEN_FANCONTROL_SET)GetProcAddress(ati_hDLL, "ADL2_OverdriveN_FanControl_Set");
	ADL2_OverdriveN_PowerLimit_Get =		(ADL2_OVERDRIVEN_POWERLIMIT_GET)GetProcAddress(ati_hDLL, "ADL2_OverdriveN_PowerLimit_Get");
	ADL2_OverdriveN_PowerLimit_Set =		(ADL2_OVERDRIVEN_POWERLIMIT_SET)GetProcAddress(ati_hDLL, "ADL2_OverdriveN_PowerLimit_Set");
	ADL2_OverdriveN_Temperature_Get =		(ADL2_OVERDRIVEN_TEMPERATURE_GET)GetProcAddress(ati_hDLL, "ADL2_OverdriveN_Temperature_Get");
	ADL2_Adapter_MemoryInfo_Get =			(ADL2_ADAPTER_MEMORYINFO_GET)GetProcAddress(ati_hDLL, "ADL2_Adapter_MemoryInfo_Get");
	ADL2_Overdrive_Caps =					(ADL2_OVERDRIVE_CAPS)GetProcAddress(ati_hDLL, "ADL2_Overdrive_Caps");
	ADL_Adapter_ID_Get =					(ADL_ADAPTER_ID_GET)GetProcAddress(ati_hDLL, "ADL_Adapter_ID_Get");

	if (NULL == ADL_Main_Control_Create ||
		NULL == ADL_Main_Control_Destroy ||
		NULL == ADL_Adapter_NumberOfAdapters_Get ||
		NULL == ADL_Adapter_AdapterInfo_Get ||
		NULL == ADL_AdapterX2_Caps ||
		NULL == ADL2_Adapter_Active_Get ||
		NULL == ADL2_OverdriveN_Capabilities_Get ||
		NULL == ADL2_OverdriveN_SystemClocks_Get ||
		NULL == ADL2_OverdriveN_SystemClocks_Set ||
		NULL == ADL2_OverdriveN_MemoryClocks_Get ||
		NULL == ADL2_OverdriveN_MemoryClocks_Set ||
		NULL == ADL2_OverdriveN_PerformanceStatus_Get ||
		NULL == ADL2_OverdriveN_FanControl_Get ||
		NULL == ADL2_OverdriveN_FanControl_Set ||
		NULL == ADL2_Overdrive_Caps ||
		NULL == ADL_Adapter_ID_Get ||
		NULL == ADL2_Adapter_MemoryInfo_Get
		)
	{
		Com_Printf(S_COLOR_RED"Failed to get ADL function pointers\n");
		return;
	}

	if (ADL_OK != ADL_Main_Control_Create(ADL_Main_Memory_Alloc, 1))
	{
		Com_Printf(S_COLOR_RED"Failed to initialize nested ADL2 context");
		return;
	}

	// Obtain the number of adapters for the system
	if (ADL_OK != ADL_Adapter_NumberOfAdapters_Get(&atiPhysicalGpuCount))
	{
		Com_Printf(S_COLOR_RED"Cannot get the number of adapters!\n");
		ADL_Shutdown();
		return;
	}

	if (atiPhysicalGpuCount > 0)
	{
		adapterInfo = (LPAdapterInfo)malloc(sizeof(AdapterInfo) * atiPhysicalGpuCount);
		memset(adapterInfo, '\0', sizeof(AdapterInfo) * atiPhysicalGpuCount);

		// Get the AdapterInfo structure for all adapters in the system
		if(adapterInfo)
		ADL_Adapter_AdapterInfo_Get(adapterInfo, sizeof(AdapterInfo) * atiPhysicalGpuCount);
		else
		{
			Com_Printf(S_COLOR_RED"Cannot get lpAdapterInfo!\n");
			ADL_Shutdown();
		}
	}

	//	Looking for overdrive version
	for (i = 0; i < atiPhysicalGpuCount; i++){
		
		if (adapterInfo[i].iBusNumber > -1)
			ADL2_Overdrive_Caps(context, adapterInfo[i].iAdapterIndex, &iSupported, &iEnabled, &overDriveVer);
			i = atiPhysicalGpuCount;
	}

	Com_Printf("\n...Looking for Overdrive version: ");

	if (overDriveVer == 7)
		Com_Printf("\n...Found" S_COLOR_YELLOW " OverdriveN\n");
	else {
		Com_Printf(S_COLOR_RED"Failed!\nIncompatible version %i\n", overDriveVer);
		ADL_Shutdown();
		return;
	}

	// Looking for all active ATI adapters in the system
	for (i = 0; i < atiPhysicalGpuCount; i++)
	{
		int adapterActive = 0;
		AdapterInfo adInfo = adapterInfo[i];
		ADL2_Adapter_Active_Get(context, adInfo.iAdapterIndex, &adapterActive);
		if (adapterActive && adInfo.iVendorID == AMDVENDORID)
		{
			ADL_Adapter_ID_Get(adInfo.iAdapterIndex, &adapterID);
			for (ii = 0; ii < seenids_num; ii++)
				if (seenids[ii] == adapterID)
					break;

			if (ii == seenids_num)
			{
				seenids[seenids_num] = adapterID;
				Com_Printf("...Adapter %i: " S_COLOR_GREEN "%s\n", seenids_num, adInfo.strAdapterName);
				seenids_num++;
				strcpy(gpuNames[i], adInfo.strAdapterName);
			}
		}
	}
	if (!seenids_num)
	{
		Com_Printf(S_COLOR_RED"...no active adapters found\n");
		ADL_Shutdown();
		return;
	}

	atiPhysicalAdapters = seenids_num;

	adlInit = qtrue;
}

void ADL_PrintGpuInfo(){

	Com_Printf(S_COLOR_YELLOW"============ AMD Radeon GPU Info ============\n\n");

	for (int i = 0; i < atiPhysicalAdapters; i++) {

		ADLODNCapabilities overdriveCapabilities;
		memset(&overdriveCapabilities, 0, sizeof(ADLODNCapabilities));
		
		if (ADL_OK != ADL2_OverdriveN_Capabilities_Get(context, adapterInfo[i].iAdapterIndex, &overdriveCapabilities))
			Com_Printf(S_COLOR_RED"ADL2_OverdriveN_Capabilities_Get is failed\n");
		
		Com_Printf("GPU " S_COLOR_GREEN "%i" S_COLOR_WHITE ":" S_COLOR_GREEN " %s\n", atiPhysicalAdapters - 1, gpuNames[i]);
		
		// vram info
		ADLMemoryInfo memInfo;
		memset(&memInfo, 0, sizeof(ADLMemoryInfo));
		ADL2_Adapter_MemoryInfo_Get(context, adapterInfo[i].iAdapterIndex, &memInfo);
		Com_Printf("GPU " S_COLOR_GREEN "%i" S_COLOR_WHITE ": Vram Info   : %lld Gb %s Bandwidth: %lld Gb/sec\n", atiPhysicalAdapters - 1, memInfo.iMemorySize >>30, memInfo.strMemoryType, memInfo.iMemoryBandwidth >>10);
		
		// temperature level
		ADLODNPowerLimitSetting odNPowerControl;
		memset(&odNPowerControl, 0, sizeof(ADLODNPowerLimitSetting));

		if (ADL_OK != ADL2_OverdriveN_PowerLimit_Get(context, adapterInfo[i].iAdapterIndex, &odNPowerControl)) {
			Com_Printf(S_COLOR_RED"ADL2_OverdriveN_PowerLimit_Get is failed\n");
		}
		else {
			int temp;
			ADL2_OverdriveN_Temperature_Get(context, adapterInfo[i].iAdapterIndex, 1, &temp);
			
			// Thermal sens or returns an integer up to the third character What the fuck is this accuracy???
			Com_Printf("GPU " S_COLOR_GREEN "%i" S_COLOR_WHITE ": Temperature : %3.2f Celsius\n", atiPhysicalAdapters-1, (float)temp / 1000);
		}

		// fan speed
		ADLODNFanControl odNFanControl;
		memset(&odNFanControl, 0, sizeof(ADLODNFanControl));

		if (ADL_OK != ADL2_OverdriveN_FanControl_Get(context, adapterInfo[i].iAdapterIndex, &odNFanControl)) {
			Com_Printf(S_COLOR_RED"ADL2_OverdriveN_FanControl_Get is failed\n");
		}
		else {
			Com_Printf("GPU " S_COLOR_GREEN "%i" S_COLOR_WHITE ": Fan Speed   : %i RPM\n", atiPhysicalAdapters - 1, odNFanControl.iCurrentFanSpeed);
		}

		// gpu-vram clock
		ADLODNPerformanceStatus odNPerformanceStatus;
		memset(&odNPerformanceStatus, 0, sizeof(ADLODNPerformanceStatus));

		if (ADL_OK != ADL2_OverdriveN_PerformanceStatus_Get(context, adapterInfo[i].iAdapterIndex, &odNPerformanceStatus)) {
			Com_Printf(S_COLOR_RED"ADL2_OverdriveN_PerformanceStatus_Get is failed\n");
		}
		else {
			Com_Printf("GPU " S_COLOR_GREEN "%i" S_COLOR_WHITE ": Activity    : %i Percent\n", 
						atiPhysicalAdapters - 1, odNPerformanceStatus.iGPUActivityPercent);
			Com_Printf("GPU " S_COLOR_GREEN "%i" S_COLOR_WHITE ": CoreClock   : %i Mhz\n", 
						atiPhysicalAdapters - 1, odNPerformanceStatus.iCoreClock / 100);
			Com_Printf("GPU " S_COLOR_GREEN "%i" S_COLOR_WHITE ": MemoryClock : %i (%i Effective) Mhz\n", 
						atiPhysicalAdapters - 1, odNPerformanceStatus.iMemoryClock / 100, (odNPerformanceStatus.iMemoryClock / 100) * 4);
		}
		Com_Printf(S_COLOR_YELLOW"\n=============================================\n\n");
	}
}