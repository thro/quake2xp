/*
* This is an open source non-commercial project. Dear PVS-Studio, please check it.
* PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
*/

// NV API SDK

#include "../ref_gl/r_local.h"

NvPhysicalGpuHandle hPhysicalGpu[NVAPI_MAX_PHYSICAL_GPUS];
NvU32 physicalGpuCount = 0;

char *GLimp_NvApi_GetThermalController(NV_THERMAL_CONTROLLER tc)
{
	switch (tc)
	{
	case NVAPI_THERMAL_CONTROLLER_NONE: return "None";
	case NVAPI_THERMAL_CONTROLLER_GPU_INTERNAL: return "GPU Internal";
	case NVAPI_THERMAL_CONTROLLER_ADM1032: return "ADM1032";
	case NVAPI_THERMAL_CONTROLLER_MAX6649: return "MAX6649";
	case NVAPI_THERMAL_CONTROLLER_MAX1617: return "MAX1617";
	case NVAPI_THERMAL_CONTROLLER_LM99: return "LM99";
	case NVAPI_THERMAL_CONTROLLER_LM89: return "LM89";
	case NVAPI_THERMAL_CONTROLLER_LM64: return "LM64";
	case NVAPI_THERMAL_CONTROLLER_ADT7473: return "ADT7473";
	case NVAPI_THERMAL_CONTROLLER_SBMAX6649: return "SBMAX6649";
	case NVAPI_THERMAL_CONTROLLER_VBIOSEVT: return "VBIOSEVT";
	case NVAPI_THERMAL_CONTROLLER_OS: return "OS";
	default:
	case NVAPI_THERMAL_CONTROLLER_UNKNOWN: return "Unknown";
	}
}

void GLimp_InitNvApi() {

	NvAPI_Status ret = NVAPI_OK;
	NvAPI_ShortString ver, string;

	nvApiInit = qfalse;
	
	Com_Printf("\n==================================\n\n");

	Com_Printf("" S_COLOR_YELLOW "...Initializing NVIDIA API\n\n");

	// init nvapi
	ret = NvAPI_Initialize();

	if (ret != NVAPI_OK) { // check for nvapi error
		Com_Printf(S_COLOR_MAGENTA"...not supported\n");
		Com_Printf("\n==================================\n");
		return;
	}

	NvAPI_GetInterfaceVersionString(ver);
	Com_Printf("...use" S_COLOR_GREEN " %s\n", ver);

	// Enumerate the physical GPU handle
	ret = NvAPI_EnumPhysicalGPUs(hPhysicalGpu, &physicalGpuCount);

	if (ret != NVAPI_OK) {
		NvAPI_GetErrorMessage(ret, string);
		Com_Printf(S_COLOR_RED"...NvAPI_EnumPhysicalGPUs() fail: %s\n", string);
		return;
	}

	Com_Printf("...found " S_COLOR_GREEN "%i " S_COLOR_WHITE "physical gpu's\n", physicalGpuCount);

	nvApiInit = qtrue;

	Com_Printf("\n==================================\n\n");
}

#define NV_UTIL_DOMAIN_GPU  0
#define NV_UTIL_DOMAIN_FB   1
#define NV_UTIL_DOMAIN_VID  2 //video decoder don't needed
#define NV_UTIL_DOMAIN_BUS  3

extern qboolean adlInit;

void R_GpuInfo_f(void) {

	NvAPI_Status					ret = NVAPI_OK;
	NvU32							rpm = 0;
	NvAPI_ShortString				string;
	NV_GPU_THERMAL_SETTINGS			thermal;
	NV_GPU_DYNAMIC_PSTATES_INFO_EX	m_DynamicPStateInfo;
	NV_GPU_CLOCK_FREQUENCIES		clocks;

	if (adlInit) {
		ADL_PrintGpuInfo();
		return;
	}

	if (!nvApiInit) {
		Com_Printf(S_COLOR_RED"NVAPI not found!\n");
		return;
	}

	Com_Printf("\n==========================================================\n");
	ret = NvAPI_GPU_GetFullName(hPhysicalGpu[0], string);

	if (ret != NVAPI_OK) {
		NvAPI_GetErrorMessage(ret, string);
		Com_Printf(S_COLOR_RED"...NvAPI_GPU_GetFullName() fail: %\n", string);
	}
	else
		Com_Printf(S_COLOR_YELLOW"...Get GPU statistic from " S_COLOR_GREEN "%s\n", string);

	for (int i = 0; i < physicalGpuCount; i++) {

		Com_Printf("\n   GPU " S_COLOR_GREEN "%i" S_COLOR_WHITE ":\n", i);

		// get gpu temperature
		thermal.version = NV_GPU_THERMAL_SETTINGS_VER_2;
		ret = NvAPI_GPU_GetThermalSettings(hPhysicalGpu[i], 0, &thermal);

		if (ret != NVAPI_OK) {
			NvAPI_GetErrorMessage(ret, string);
			Com_Printf(S_COLOR_RED"...NvAPI_GPU_GetThermalSettings() fail: %\n", string);
		}
		else
			Com_Printf("...temperature: " S_COLOR_GREEN "%u" S_COLOR_WHITE " Celsius (%s)\n", thermal.sensor[0].currentTemp,
				GLimp_NvApi_GetThermalController(thermal.sensor[0].controller));

		// get fans speed
		ret = NvAPI_GPU_GetTachReading(hPhysicalGpu[i], &rpm);

		if (ret != NVAPI_OK)
		{
			NvAPI_GetErrorMessage(ret, string);
			Com_Printf(S_COLOR_RED"NvAPI_GPU_GetTachReading() fail: %s\n", string);
		}
		else
			Com_Printf("...fan speed: " S_COLOR_GREEN "%u" S_COLOR_WHITE " rpm\n", rpm);

		m_DynamicPStateInfo.version = NV_GPU_DYNAMIC_PSTATES_INFO_EX_VER;
		ret = NvAPI_GPU_GetDynamicPstatesInfoEx(hPhysicalGpu[i], &m_DynamicPStateInfo);
		if (ret != NVAPI_OK)
		{
			NvAPI_GetErrorMessage(ret, string);
			Com_Printf(S_COLOR_RED"NvAPI_GPU_GetDynamicPstatesInfoEx() fail: %s", string);
		}
		else
			Com_Printf("...utilization: " S_COLOR_YELLOW "Core " S_COLOR_GREEN "%u" S_COLOR_WHITE " %%, " S_COLOR_YELLOW "Frame Buffer " S_COLOR_GREEN "%u" S_COLOR_WHITE " %%, " S_COLOR_YELLOW "PCIe Bus " S_COLOR_GREEN "%u" S_COLOR_WHITE " %%\n",
				m_DynamicPStateInfo.utilization[NV_UTIL_DOMAIN_GPU].percentage,
				m_DynamicPStateInfo.utilization[NV_UTIL_DOMAIN_FB].percentage,
				m_DynamicPStateInfo.utilization[NV_UTIL_DOMAIN_BUS].percentage);

		// get gpu & vram frequencies
		clocks.version = NV_GPU_CLOCK_FREQUENCIES_VER;
		clocks.ClockType = NV_GPU_CLOCK_FREQUENCIES_CURRENT_FREQ;
		ret = NvAPI_GPU_GetAllClockFrequencies(hPhysicalGpu[i], &clocks);

		if (ret != NVAPI_OK) {
			NvAPI_GetErrorMessage(ret, string);
			Com_Printf(S_COLOR_RED "NvAPI_GPU_GetAllClockFrequencies() fail: %s\n", string);
		}
		else {
			if (clocks.domain[NVAPI_GPU_PUBLIC_CLOCK_GRAPHICS].bIsPresent && clocks.domain[NVAPI_GPU_PUBLIC_CLOCK_MEMORY].bIsPresent)
				Com_Printf("...frequencies: " S_COLOR_YELLOW "GPU: " S_COLOR_GREEN "%u" S_COLOR_WHITE " MHz " S_COLOR_YELLOW "VRAM: " S_COLOR_GREEN "%u" S_COLOR_WHITE " MHz\n",
				(NvU32)((clocks.domain[NVAPI_GPU_PUBLIC_CLOCK_GRAPHICS].frequency + 500) / 1000),
					(NvU32)((clocks.domain[NVAPI_GPU_PUBLIC_CLOCK_MEMORY].frequency + 500) / 1000));
		}


		Com_Printf("\n==========================================================\n");
	}
}
