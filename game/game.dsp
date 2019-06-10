# Microsoft Developer Studio Project File - Name="gamex86xp" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=gamex86xp - Win32 Debug Alpha
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "gamex86xp.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "gamex86xp.mak" CFG="gamex86xp - Win32 Debug Alpha"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "gamex86xp - Win32 Debug Alpha" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "gamex86xp - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "gamex86xp - Win32 Release Alpha" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "gamex86xp - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "gamex86xp - x64 Debug Alpha" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "gamex86xp - x64 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "gamex86xp - x64 Release Alpha" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "gamex86xp - x64 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "gamex86xp - Win32 Debug Alpha"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir ".\..\DebugAxp"
# PROP BASE Intermediate_Dir ".\DebugAxp"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir ".\..\DebugAxp"
# PROP Intermediate_Dir ".\DebugAxp"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /Zi /W3 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" PRECOMP_VC7_TOBEREMOVED /Fp".\DebugAxp/game.pch" /Fo".\DebugAxp/" /Fd".\DebugAxp/" /GZ /c /QA21164 /GX 
# ADD CPP /nologo /MTd /Zi /W3 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" PRECOMP_VC7_TOBEREMOVED /Fp".\DebugAxp/game.pch" /Fo".\DebugAxp/" /Fd".\DebugAxp/" /GZ /c /QA21164 /GX 
# ADD BASE MTL /nologo /D"_DEBUG" /mktyplib203 /tlb".\..\DebugAxp\game.tlb" /win32 
# ADD MTL /nologo /D"_DEBUG" /mktyplib203 /tlb".\..\DebugAxp\game.tlb" /win32 
# ADD BASE RSC /l 1033 /d "_DEBUG" 
# ADD RSC /l 1033 /d "_DEBUG" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo 
# ADD BSC32 /nologo 
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib odbc32.lib odbccp32.lib /nologo /dll /out:"..\DebugAxp\gameaxp.dll" /incremental:no /def:".\game.def" /debug /pdb:".\..\DebugAxp\gameaxp.pdb" /pdbtype:sept /subsystem:windows /base:"0x20000000" /implib:".\..\DebugAxp/gameaxp.lib" 
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib odbc32.lib odbccp32.lib /nologo /dll /out:"..\DebugAxp\gameaxp.dll" /incremental:no /def:".\game.def" /debug /pdb:".\..\DebugAxp\gameaxp.pdb" /pdbtype:sept /subsystem:windows /base:"0x20000000" /implib:".\..\DebugAxp/gameaxp.lib" 

!ELSEIF  "$(CFG)" == "gamex86xp - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir ".\..\debug"
# PROP BASE Intermediate_Dir ".\debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir ".\..\debug"
# PROP Intermediate_Dir ".\debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /Zi /W3 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "BUILDING_REF_GL" PRECOMP_VC7_TOBEREMOVED /Fp".\debug/game.pch" /Fo".\debug/" /Fd".\debug/" /FR /TC /GZ /c /GX 
# ADD CPP /nologo /MTd /Zi /W3 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "BUILDING_REF_GL" PRECOMP_VC7_TOBEREMOVED /Fp".\debug/game.pch" /Fo".\debug/" /Fd".\debug/" /FR /TC /GZ /c /GX 
# ADD BASE MTL /nologo /D"_DEBUG" /mktyplib203 /tlb".\..\debug\game.tlb" /win32 
# ADD MTL /nologo /D"_DEBUG" /mktyplib203 /tlb".\..\debug\game.tlb" /win32 
# ADD BASE RSC /l 1033 /d "_DEBUG" 
# ADD RSC /l 1033 /d "_DEBUG" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo 
# ADD BSC32 /nologo 
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib winmm.lib /nologo /dll /out:"c:\quake2\baseq2\gamex86xp.dll" /incremental:no /def:".\game.def" /debug /pdb:".\..\debug\gamex86.pdb" /pdbtype:sept /map:".\debug\gamex86.map" /subsystem:windows /base:"0x20000000" /implib:".\..\debug/gamex86.lib" /MACHINE:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib winmm.lib /nologo /dll /out:"c:\quake2\baseq2\gamex86xp.dll" /incremental:no /def:".\game.def" /debug /pdb:".\..\debug\gamex86.pdb" /pdbtype:sept /map:".\debug\gamex86.map" /subsystem:windows /base:"0x20000000" /implib:".\..\debug/gamex86.lib" /MACHINE:I386

!ELSEIF  "$(CFG)" == "gamex86xp - Win32 Release Alpha"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir ".\..\ReleaseAXP"
# PROP BASE Intermediate_Dir ".\ReleaseAXP"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir ".\..\ReleaseAXP"
# PROP Intermediate_Dir ".\ReleaseAXP"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /Ob1 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GF /Gy PRECOMP_VC7_TOBEREMOVED /Fp".\ReleaseAXP/game.pch" /Fo".\ReleaseAXP/" /Fd".\ReleaseAXP/" /c /QA21164 /GX 
# ADD CPP /nologo /MT /W3 /Ob1 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GF /Gy PRECOMP_VC7_TOBEREMOVED /Fp".\ReleaseAXP/game.pch" /Fo".\ReleaseAXP/" /Fd".\ReleaseAXP/" /c /QA21164 /GX 
# ADD BASE MTL /nologo /D"NDEBUG" /mktyplib203 /tlb".\..\ReleaseAXP\game.tlb" /win32 
# ADD MTL /nologo /D"NDEBUG" /mktyplib203 /tlb".\..\ReleaseAXP\game.tlb" /win32 
# ADD BASE RSC /l 1033 /d "NDEBUG" 
# ADD RSC /l 1033 /d "NDEBUG" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo 
# ADD BSC32 /nologo 
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib odbc32.lib odbccp32.lib /nologo /dll /out:"..\ReleaseAXP\gameaxp.dll" /incremental:no /def:".\game.def" /pdb:".\..\ReleaseAXP\gameaxp.pdb" /pdbtype:sept /subsystem:windows /base:"0x20000000" /implib:".\..\ReleaseAXP/gameaxp.lib" 
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib odbc32.lib odbccp32.lib /nologo /dll /out:"..\ReleaseAXP\gameaxp.dll" /incremental:no /def:".\game.def" /pdb:".\..\ReleaseAXP\gameaxp.pdb" /pdbtype:sept /subsystem:windows /base:"0x20000000" /implib:".\..\ReleaseAXP/gameaxp.lib" 

!ELSEIF  "$(CFG)" == "gamex86xp - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir ".\..\release"
# PROP BASE Intermediate_Dir ".\release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir ".\..\release"
# PROP Intermediate_Dir ".\release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /Ob1 /Oi /D "WIN32" /D "NDEBUG" /D "_WINDOWS" PRECOMP_VC7_TOBEREMOVED /Fp".\release/game.pch" /Fo".\release/" /Fd".\release/" /TC /c /GX 
# ADD CPP /nologo /MT /Ob1 /Oi /D "WIN32" /D "NDEBUG" /D "_WINDOWS" PRECOMP_VC7_TOBEREMOVED /Fp".\release/game.pch" /Fo".\release/" /Fd".\release/" /TC /c /GX 
# ADD BASE MTL /nologo /D"NDEBUG" /mktyplib203 /tlb".\..\release\game.tlb" /win32 
# ADD MTL /nologo /D"NDEBUG" /mktyplib203 /tlb".\..\release\game.tlb" /win32 
# ADD BASE RSC /l 1033 /d "NDEBUG" 
# ADD RSC /l 1033 /d "NDEBUG" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo 
# ADD BSC32 /nologo 
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib winmm.lib /nologo /dll /out:"c:\quake2\baseq2\gamex86xp.dll" /incremental:no /def:".\game.def" /pdb:".\..\release\gamex86.pdb" /pdbtype:sept /subsystem:windows /base:"0x20000000" /implib:".\..\release/gamex86.lib" /machine:ix86 /MACHINE:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib winmm.lib /nologo /dll /out:"c:\quake2\baseq2\gamex86xp.dll" /incremental:no /def:".\game.def" /pdb:".\..\release\gamex86.pdb" /pdbtype:sept /subsystem:windows /base:"0x20000000" /implib:".\..\release/gamex86.lib" /machine:ix86 /MACHINE:I386

!ELSEIF  "$(CFG)" == "gamex86xp - x64 Debug Alpha"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "$(PlatformName)\$(ConfigurationName)"
# PROP BASE Intermediate_Dir "$(PlatformName)\$(ConfigurationName)"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "$(PlatformName)\$(ConfigurationName)"
# PROP Intermediate_Dir "$(PlatformName)\$(ConfigurationName)"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /Zi /W3 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" PRECOMP_VC7_TOBEREMOVED /Fp".\DebugAxp/game.pch" /Fo".\DebugAxp/" /Fd".\DebugAxp/" /GZ /c /QA21164 /GX 
# ADD CPP /nologo /MTd /Zi /W3 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" PRECOMP_VC7_TOBEREMOVED /Fp".\DebugAxp/game.pch" /Fo".\DebugAxp/" /Fd".\DebugAxp/" /GZ /c /QA21164 /GX 
# ADD BASE MTL /nologo /D"_DEBUG" /mktyplib203 /tlb".\..\DebugAxp\game.tlb" /win32 
# ADD MTL /nologo /D"_DEBUG" /mktyplib203 /tlb".\..\DebugAxp\game.tlb" /win32 
# ADD BASE RSC /l 1033 /d "_DEBUG" 
# ADD RSC /l 1033 /d "_DEBUG" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo 
# ADD BSC32 /nologo 
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib odbc32.lib odbccp32.lib /nologo /dll /out:"..\DebugAxp\gameaxp.dll" /incremental:no /def:".\game.def" /debug /pdb:".\..\DebugAxp\gameaxp.pdb" /pdbtype:sept /subsystem:windows /base:"0x20000000" /implib:".\..\DebugAxp/gameaxp.lib" 
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib odbc32.lib odbccp32.lib /nologo /dll /out:"..\DebugAxp\gameaxp.dll" /incremental:no /def:".\game.def" /debug /pdb:".\..\DebugAxp\gameaxp.pdb" /pdbtype:sept /subsystem:windows /base:"0x20000000" /implib:".\..\DebugAxp/gameaxp.lib" 

!ELSEIF  "$(CFG)" == "gamex86xp - x64 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "$(PlatformName)\$(ConfigurationName)"
# PROP BASE Intermediate_Dir "$(PlatformName)\$(ConfigurationName)"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "$(PlatformName)\$(ConfigurationName)"
# PROP Intermediate_Dir "$(PlatformName)\$(ConfigurationName)"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /Zi /W3 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "BUILDING_REF_GL" PRECOMP_VC7_TOBEREMOVED /Fp".\debug/game.pch" /Fo".\debug/" /Fd".\debug/" /FR /TC /GZ /c /GX 
# ADD CPP /nologo /MTd /Zi /W3 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "BUILDING_REF_GL" PRECOMP_VC7_TOBEREMOVED /Fp".\debug/game.pch" /Fo".\debug/" /Fd".\debug/" /FR /TC /GZ /c /GX 
# ADD BASE MTL /nologo /D"_DEBUG" /mktyplib203 /tlb".\..\debug\game.tlb" /win32 
# ADD MTL /nologo /D"_DEBUG" /mktyplib203 /tlb".\..\debug\game.tlb" /win32 
# ADD BASE RSC /l 1033 /d "_DEBUG" 
# ADD RSC /l 1033 /d "_DEBUG" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo 
# ADD BSC32 /nologo 
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib winmm.lib /nologo /dll /out:"c:\quake2\baseq2\gamex86xp.dll" /incremental:no /def:".\game.def" /debug /pdb:".\..\debug\gamex86.pdb" /pdbtype:sept /map:".\debug\gamex86.map" /subsystem:windows /base:"0x20000000" /implib:".\..\debug/gamex86.lib" /MACHINE:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib winmm.lib /nologo /dll /out:"c:\quake2\baseq2\gamex86xp.dll" /incremental:no /def:".\game.def" /debug /pdb:".\..\debug\gamex86.pdb" /pdbtype:sept /map:".\debug\gamex86.map" /subsystem:windows /base:"0x20000000" /implib:".\..\debug/gamex86.lib" /MACHINE:I386

!ELSEIF  "$(CFG)" == "gamex86xp - x64 Release Alpha"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "$(PlatformName)\$(ConfigurationName)"
# PROP BASE Intermediate_Dir "$(PlatformName)\$(ConfigurationName)"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "$(PlatformName)\$(ConfigurationName)"
# PROP Intermediate_Dir "$(PlatformName)\$(ConfigurationName)"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /Ob1 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GF /Gy PRECOMP_VC7_TOBEREMOVED /Fp".\ReleaseAXP/game.pch" /Fo".\ReleaseAXP/" /Fd".\ReleaseAXP/" /c /QA21164 /GX 
# ADD CPP /nologo /MT /W3 /Ob1 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GF /Gy PRECOMP_VC7_TOBEREMOVED /Fp".\ReleaseAXP/game.pch" /Fo".\ReleaseAXP/" /Fd".\ReleaseAXP/" /c /QA21164 /GX 
# ADD BASE MTL /nologo /D"NDEBUG" /mktyplib203 /tlb".\..\ReleaseAXP\game.tlb" /win32 
# ADD MTL /nologo /D"NDEBUG" /mktyplib203 /tlb".\..\ReleaseAXP\game.tlb" /win32 
# ADD BASE RSC /l 1033 /d "NDEBUG" 
# ADD RSC /l 1033 /d "NDEBUG" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo 
# ADD BSC32 /nologo 
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib odbc32.lib odbccp32.lib /nologo /dll /out:"..\ReleaseAXP\gameaxp.dll" /incremental:no /def:".\game.def" /pdb:".\..\ReleaseAXP\gameaxp.pdb" /pdbtype:sept /subsystem:windows /base:"0x20000000" /implib:".\..\ReleaseAXP/gameaxp.lib" 
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib odbc32.lib odbccp32.lib /nologo /dll /out:"..\ReleaseAXP\gameaxp.dll" /incremental:no /def:".\game.def" /pdb:".\..\ReleaseAXP\gameaxp.pdb" /pdbtype:sept /subsystem:windows /base:"0x20000000" /implib:".\..\ReleaseAXP/gameaxp.lib" 

!ELSEIF  "$(CFG)" == "gamex86xp - x64 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "$(PlatformName)\$(ConfigurationName)"
# PROP BASE Intermediate_Dir "$(PlatformName)\$(ConfigurationName)"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "$(PlatformName)\$(ConfigurationName)"
# PROP Intermediate_Dir "$(PlatformName)\$(ConfigurationName)"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /Ob1 /Oi /D "WIN32" /D "NDEBUG" /D "_WINDOWS" PRECOMP_VC7_TOBEREMOVED /Fp".\release/game.pch" /Fo".\release/" /Fd".\release/" /TC /c /GX 
# ADD CPP /nologo /MT /Ob1 /Oi /D "WIN32" /D "NDEBUG" /D "_WINDOWS" PRECOMP_VC7_TOBEREMOVED /Fp".\release/game.pch" /Fo".\release/" /Fd".\release/" /TC /c /GX 
# ADD BASE MTL /nologo /D"NDEBUG" /mktyplib203 /tlb".\..\release\game.tlb" /win32 
# ADD MTL /nologo /D"NDEBUG" /mktyplib203 /tlb".\..\release\game.tlb" /win32 
# ADD BASE RSC /l 1033 /d "NDEBUG" 
# ADD RSC /l 1033 /d "NDEBUG" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo 
# ADD BSC32 /nologo 
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib winmm.lib /nologo /dll /out:"c:\quake2\baseq2\gamex86xp.dll" /incremental:no /def:".\game.def" /pdb:".\..\release\gamex86.pdb" /pdbtype:sept /subsystem:windows /base:"0x20000000" /implib:".\..\release/gamex86.lib" /MACHINE:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib winmm.lib /nologo /dll /out:"c:\quake2\baseq2\gamex86xp.dll" /incremental:no /def:".\game.def" /pdb:".\..\release\gamex86.pdb" /pdbtype:sept /subsystem:windows /base:"0x20000000" /implib:".\..\release/gamex86.lib" /MACHINE:I386

!ENDIF

# Begin Target

# Name "gamex86xp - Win32 Debug Alpha"
# Name "gamex86xp - Win32 Debug"
# Name "gamex86xp - Win32 Release Alpha"
# Name "gamex86xp - Win32 Release"
# Name "gamex86xp - x64 Debug Alpha"
# Name "gamex86xp - x64 Debug"
# Name "gamex86xp - x64 Release Alpha"
# Name "gamex86xp - x64 Release"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;hpj;bat;for;f90"
# Begin Source File

SOURCE=g_ai.c

!IF  "$(CFG)" == "gamex86xp - Win32 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" /GZ /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - Win32 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "BUILDING_REF_GL" /FR /GZ /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - Win32 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - Win32 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - x64 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" /GZ /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - x64 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "BUILDING_REF_GL" /FR /GZ /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - x64 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - x64 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ENDIF

# End Source File
# Begin Source File

SOURCE=g_chase.c

!IF  "$(CFG)" == "gamex86xp - Win32 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" /GZ /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - Win32 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "BUILDING_REF_GL" /FR /GZ /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - Win32 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - Win32 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - x64 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" /GZ /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - x64 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "BUILDING_REF_GL" /FR /GZ /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - x64 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - x64 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ENDIF

# End Source File
# Begin Source File

SOURCE=g_cmds.c

!IF  "$(CFG)" == "gamex86xp - Win32 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" /GZ /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - Win32 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "BUILDING_REF_GL" /FR /GZ /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - Win32 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - Win32 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - x64 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" /GZ /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - x64 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "BUILDING_REF_GL" /FR /GZ /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - x64 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - x64 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ENDIF

# End Source File
# Begin Source File

SOURCE=g_combat.c

!IF  "$(CFG)" == "gamex86xp - Win32 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" /GZ /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - Win32 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "BUILDING_REF_GL" /FR /GZ /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - Win32 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - Win32 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - x64 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" /GZ /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - x64 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "BUILDING_REF_GL" /FR /GZ /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - x64 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - x64 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ENDIF

# End Source File
# Begin Source File

SOURCE=g_func.c

!IF  "$(CFG)" == "gamex86xp - Win32 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" /GZ /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - Win32 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "BUILDING_REF_GL" /FR /GZ /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - Win32 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - Win32 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - x64 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" /GZ /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - x64 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "BUILDING_REF_GL" /FR /GZ /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - x64 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - x64 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ENDIF

# End Source File
# Begin Source File

SOURCE=g_items.c

!IF  "$(CFG)" == "gamex86xp - Win32 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" /GZ /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - Win32 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "BUILDING_REF_GL" /FR /GZ /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - Win32 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - Win32 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - x64 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" /GZ /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - x64 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "BUILDING_REF_GL" /FR /GZ /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - x64 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - x64 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ENDIF

# End Source File
# Begin Source File

SOURCE=g_main.c

!IF  "$(CFG)" == "gamex86xp - Win32 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" /GZ /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - Win32 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "BUILDING_REF_GL" /FR /GZ /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - Win32 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - Win32 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - x64 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" /GZ /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - x64 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "BUILDING_REF_GL" /FR /GZ /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - x64 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - x64 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ENDIF

# End Source File
# Begin Source File

SOURCE=g_misc.c

!IF  "$(CFG)" == "gamex86xp - Win32 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" /GZ /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - Win32 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "BUILDING_REF_GL" /FR /GZ /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - Win32 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - Win32 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - x64 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" /GZ /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - x64 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "BUILDING_REF_GL" /FR /GZ /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - x64 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - x64 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ENDIF

# End Source File
# Begin Source File

SOURCE=g_monster.c

!IF  "$(CFG)" == "gamex86xp - Win32 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" /GZ /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - Win32 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "BUILDING_REF_GL" /FR /GZ /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - Win32 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - Win32 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - x64 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" /GZ /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - x64 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "BUILDING_REF_GL" /FR /GZ /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - x64 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - x64 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ENDIF

# End Source File
# Begin Source File

SOURCE=g_phys.c

!IF  "$(CFG)" == "gamex86xp - Win32 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" /GZ /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - Win32 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "BUILDING_REF_GL" /FR /GZ /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - Win32 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - Win32 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - x64 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" /GZ /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - x64 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "BUILDING_REF_GL" /FR /GZ /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - x64 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - x64 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ENDIF

# End Source File
# Begin Source File

SOURCE=g_save.c

!IF  "$(CFG)" == "gamex86xp - Win32 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" /GZ /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - Win32 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "BUILDING_REF_GL" /FR /GZ /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - Win32 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - Win32 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - x64 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" /GZ /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - x64 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "BUILDING_REF_GL" /FR /GZ /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - x64 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - x64 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ENDIF

# End Source File
# Begin Source File

SOURCE=g_spawn.c

!IF  "$(CFG)" == "gamex86xp - Win32 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" /GZ /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - Win32 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "BUILDING_REF_GL" /FR /GZ /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - Win32 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - Win32 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - x64 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" /GZ /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - x64 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "BUILDING_REF_GL" /FR /GZ /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - x64 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - x64 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ENDIF

# End Source File
# Begin Source File

SOURCE=g_svcmds.c

!IF  "$(CFG)" == "gamex86xp - Win32 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" /GZ /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - Win32 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "BUILDING_REF_GL" /FR /GZ /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - Win32 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - Win32 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - x64 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" /GZ /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - x64 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "BUILDING_REF_GL" /FR /GZ /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - x64 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - x64 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ENDIF

# End Source File
# Begin Source File

SOURCE=g_target.c

!IF  "$(CFG)" == "gamex86xp - Win32 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" /GZ /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - Win32 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "BUILDING_REF_GL" /FR /GZ /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - Win32 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - Win32 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - x64 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" /GZ /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - x64 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "BUILDING_REF_GL" /FR /GZ /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - x64 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - x64 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ENDIF

# End Source File
# Begin Source File

SOURCE=g_trigger.c

!IF  "$(CFG)" == "gamex86xp - Win32 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" /GZ /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - Win32 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "BUILDING_REF_GL" /FR /GZ /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - Win32 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - Win32 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - x64 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" /GZ /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - x64 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "BUILDING_REF_GL" /FR /GZ /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - x64 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - x64 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ENDIF

# End Source File
# Begin Source File

SOURCE=g_turret.c

!IF  "$(CFG)" == "gamex86xp - Win32 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" /GZ /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - Win32 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "BUILDING_REF_GL" /FR /GZ /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - Win32 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - Win32 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - x64 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" /GZ /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - x64 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "BUILDING_REF_GL" /FR /GZ /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - x64 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - x64 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ENDIF

# End Source File
# Begin Source File

SOURCE=g_utils.c

!IF  "$(CFG)" == "gamex86xp - Win32 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" /GZ /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - Win32 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "BUILDING_REF_GL" /FR /GZ /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - Win32 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - Win32 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - x64 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" /GZ /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - x64 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "BUILDING_REF_GL" /FR /GZ /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - x64 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - x64 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ENDIF

# End Source File
# Begin Source File

SOURCE=g_weapon.c

!IF  "$(CFG)" == "gamex86xp - Win32 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" /GZ /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - Win32 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "BUILDING_REF_GL" /FR /GZ /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - Win32 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - Win32 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - x64 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" /GZ /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - x64 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "BUILDING_REF_GL" /FR /GZ /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - x64 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - x64 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ENDIF

# End Source File
# Begin Source File

SOURCE=m_actor.c

!IF  "$(CFG)" == "gamex86xp - Win32 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" /GZ /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - Win32 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "BUILDING_REF_GL" /FR /GZ /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - Win32 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - Win32 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - x64 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" /GZ /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - x64 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "BUILDING_REF_GL" /FR /GZ /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - x64 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - x64 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ENDIF

# End Source File
# Begin Source File

SOURCE=m_berserk.c

!IF  "$(CFG)" == "gamex86xp - Win32 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" /GZ /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - Win32 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "BUILDING_REF_GL" /FR /GZ /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - Win32 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - Win32 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - x64 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" /GZ /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - x64 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "BUILDING_REF_GL" /FR /GZ /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - x64 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - x64 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ENDIF

# End Source File
# Begin Source File

SOURCE=m_boss2.c

!IF  "$(CFG)" == "gamex86xp - Win32 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" /GZ /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - Win32 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "BUILDING_REF_GL" /FR /GZ /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - Win32 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - Win32 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - x64 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" /GZ /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - x64 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "BUILDING_REF_GL" /FR /GZ /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - x64 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - x64 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ENDIF

# End Source File
# Begin Source File

SOURCE=m_boss3.c

!IF  "$(CFG)" == "gamex86xp - Win32 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" /GZ /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - Win32 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "BUILDING_REF_GL" /FR /GZ /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - Win32 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - Win32 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - x64 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" /GZ /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - x64 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "BUILDING_REF_GL" /FR /GZ /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - x64 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - x64 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ENDIF

# End Source File
# Begin Source File

SOURCE=m_boss31.c

!IF  "$(CFG)" == "gamex86xp - Win32 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" /GZ /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - Win32 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "BUILDING_REF_GL" /FR /GZ /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - Win32 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - Win32 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - x64 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" /GZ /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - x64 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "BUILDING_REF_GL" /FR /GZ /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - x64 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - x64 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ENDIF

# End Source File
# Begin Source File

SOURCE=m_boss32.c

!IF  "$(CFG)" == "gamex86xp - Win32 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" /GZ /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - Win32 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "BUILDING_REF_GL" /FR /GZ /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - Win32 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - Win32 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - x64 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" /GZ /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - x64 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "BUILDING_REF_GL" /FR /GZ /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - x64 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - x64 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ENDIF

# End Source File
# Begin Source File

SOURCE=m_brain.c

!IF  "$(CFG)" == "gamex86xp - Win32 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" /GZ /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - Win32 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "BUILDING_REF_GL" /FR /GZ /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - Win32 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - Win32 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - x64 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" /GZ /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - x64 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "BUILDING_REF_GL" /FR /GZ /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - x64 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - x64 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ENDIF

# End Source File
# Begin Source File

SOURCE=m_chick.c

!IF  "$(CFG)" == "gamex86xp - Win32 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" /GZ /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - Win32 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "BUILDING_REF_GL" /FR /GZ /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - Win32 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - Win32 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - x64 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" /GZ /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - x64 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "BUILDING_REF_GL" /FR /GZ /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - x64 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - x64 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ENDIF

# End Source File
# Begin Source File

SOURCE=m_flash.c

!IF  "$(CFG)" == "gamex86xp - Win32 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" /GZ /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - Win32 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "BUILDING_REF_GL" /FR /GZ /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - Win32 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - Win32 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - x64 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" /GZ /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - x64 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "BUILDING_REF_GL" /FR /GZ /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - x64 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - x64 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ENDIF

# End Source File
# Begin Source File

SOURCE=m_flipper.c

!IF  "$(CFG)" == "gamex86xp - Win32 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" /GZ /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - Win32 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "BUILDING_REF_GL" /FR /GZ /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - Win32 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - Win32 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - x64 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" /GZ /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - x64 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "BUILDING_REF_GL" /FR /GZ /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - x64 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - x64 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ENDIF

# End Source File
# Begin Source File

SOURCE=m_float.c

!IF  "$(CFG)" == "gamex86xp - Win32 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" /GZ /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - Win32 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "BUILDING_REF_GL" /FR /GZ /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - Win32 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - Win32 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - x64 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" /GZ /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - x64 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "BUILDING_REF_GL" /FR /GZ /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - x64 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - x64 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ENDIF

# End Source File
# Begin Source File

SOURCE=m_flyer.c

!IF  "$(CFG)" == "gamex86xp - Win32 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" /GZ /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - Win32 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "BUILDING_REF_GL" /FR /GZ /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - Win32 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - Win32 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - x64 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" /GZ /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - x64 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "BUILDING_REF_GL" /FR /GZ /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - x64 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - x64 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ENDIF

# End Source File
# Begin Source File

SOURCE=m_gladiator.c

!IF  "$(CFG)" == "gamex86xp - Win32 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" /GZ /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - Win32 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "BUILDING_REF_GL" /FR /GZ /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - Win32 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - Win32 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - x64 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" /GZ /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - x64 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "BUILDING_REF_GL" /FR /GZ /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - x64 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - x64 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ENDIF

# End Source File
# Begin Source File

SOURCE=m_gunner.c

!IF  "$(CFG)" == "gamex86xp - Win32 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" /GZ /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - Win32 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "BUILDING_REF_GL" /FR /GZ /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - Win32 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - Win32 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - x64 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" /GZ /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - x64 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "BUILDING_REF_GL" /FR /GZ /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - x64 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - x64 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ENDIF

# End Source File
# Begin Source File

SOURCE=m_hover.c

!IF  "$(CFG)" == "gamex86xp - Win32 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" /GZ /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - Win32 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "BUILDING_REF_GL" /FR /GZ /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - Win32 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - Win32 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - x64 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" /GZ /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - x64 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "BUILDING_REF_GL" /FR /GZ /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - x64 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - x64 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ENDIF

# End Source File
# Begin Source File

SOURCE=m_infantry.c

!IF  "$(CFG)" == "gamex86xp - Win32 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" /GZ /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - Win32 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "BUILDING_REF_GL" /FR /GZ /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - Win32 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - Win32 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - x64 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" /GZ /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - x64 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "BUILDING_REF_GL" /FR /GZ /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - x64 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - x64 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ENDIF

# End Source File
# Begin Source File

SOURCE=m_insane.c

!IF  "$(CFG)" == "gamex86xp - Win32 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" /GZ /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - Win32 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "BUILDING_REF_GL" /FR /GZ /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - Win32 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - Win32 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - x64 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" /GZ /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - x64 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "BUILDING_REF_GL" /FR /GZ /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - x64 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - x64 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ENDIF

# End Source File
# Begin Source File

SOURCE=m_medic.c

!IF  "$(CFG)" == "gamex86xp - Win32 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" /GZ /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - Win32 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "BUILDING_REF_GL" /FR /GZ /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - Win32 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - Win32 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - x64 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" /GZ /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - x64 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "BUILDING_REF_GL" /FR /GZ /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - x64 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - x64 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ENDIF

# End Source File
# Begin Source File

SOURCE=m_move.c

!IF  "$(CFG)" == "gamex86xp - Win32 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" /GZ /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - Win32 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "BUILDING_REF_GL" /FR /GZ /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - Win32 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - Win32 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - x64 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" /GZ /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - x64 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "BUILDING_REF_GL" /FR /GZ /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - x64 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - x64 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ENDIF

# End Source File
# Begin Source File

SOURCE=m_mutant.c

!IF  "$(CFG)" == "gamex86xp - Win32 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" /GZ /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - Win32 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "BUILDING_REF_GL" /FR /GZ /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - Win32 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - Win32 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - x64 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" /GZ /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - x64 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "BUILDING_REF_GL" /FR /GZ /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - x64 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - x64 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ENDIF

# End Source File
# Begin Source File

SOURCE=m_parasite.c

!IF  "$(CFG)" == "gamex86xp - Win32 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" /GZ /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - Win32 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "BUILDING_REF_GL" /FR /GZ /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - Win32 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - Win32 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - x64 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" /GZ /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - x64 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "BUILDING_REF_GL" /FR /GZ /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - x64 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - x64 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ENDIF

# End Source File
# Begin Source File

SOURCE=m_soldier.c

!IF  "$(CFG)" == "gamex86xp - Win32 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" /GZ /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - Win32 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "BUILDING_REF_GL" /FR /GZ /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - Win32 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - Win32 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - x64 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" /GZ /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - x64 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "BUILDING_REF_GL" /FR /GZ /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - x64 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - x64 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ENDIF

# End Source File
# Begin Source File

SOURCE=m_supertank.c

!IF  "$(CFG)" == "gamex86xp - Win32 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" /GZ /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - Win32 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "BUILDING_REF_GL" /FR /GZ /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - Win32 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - Win32 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - x64 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" /GZ /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - x64 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "BUILDING_REF_GL" /FR /GZ /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - x64 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - x64 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ENDIF

# End Source File
# Begin Source File

SOURCE=m_tank.c

!IF  "$(CFG)" == "gamex86xp - Win32 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" /GZ /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - Win32 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "BUILDING_REF_GL" /FR /GZ /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - Win32 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - Win32 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - x64 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" /GZ /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - x64 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "BUILDING_REF_GL" /FR /GZ /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - x64 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - x64 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ENDIF

# End Source File
# Begin Source File

SOURCE=p_client.c

!IF  "$(CFG)" == "gamex86xp - Win32 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" /GZ /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - Win32 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "BUILDING_REF_GL" /FR /GZ /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - Win32 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - Win32 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - x64 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" /GZ /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - x64 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "BUILDING_REF_GL" /FR /GZ /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - x64 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - x64 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ENDIF

# End Source File
# Begin Source File

SOURCE=p_hud.c

!IF  "$(CFG)" == "gamex86xp - Win32 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" /GZ /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - Win32 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "BUILDING_REF_GL" /FR /GZ /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - Win32 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - Win32 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - x64 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" /GZ /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - x64 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "BUILDING_REF_GL" /FR /GZ /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - x64 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - x64 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ENDIF

# End Source File
# Begin Source File

SOURCE=p_trail.c

!IF  "$(CFG)" == "gamex86xp - Win32 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" /GZ /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - Win32 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "BUILDING_REF_GL" /FR /GZ /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - Win32 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - Win32 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - x64 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" /GZ /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - x64 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "BUILDING_REF_GL" /FR /GZ /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - x64 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - x64 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ENDIF

# End Source File
# Begin Source File

SOURCE=p_view.c

!IF  "$(CFG)" == "gamex86xp - Win32 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" /GZ /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - Win32 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "BUILDING_REF_GL" /FR /GZ /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - Win32 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - Win32 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - x64 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" /GZ /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - x64 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "BUILDING_REF_GL" /FR /GZ /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - x64 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - x64 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ENDIF

# End Source File
# Begin Source File

SOURCE=p_weapon.c

!IF  "$(CFG)" == "gamex86xp - Win32 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" /GZ /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - Win32 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "BUILDING_REF_GL" /FR /GZ /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - Win32 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - Win32 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - x64 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" /GZ /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - x64 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "BUILDING_REF_GL" /FR /GZ /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - x64 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - x64 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ENDIF

# End Source File
# Begin Source File

SOURCE=q_shared.c

!IF  "$(CFG)" == "gamex86xp - Win32 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" /GZ /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - Win32 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "BUILDING_REF_GL" /FR /GZ /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - Win32 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - Win32 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - x64 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" /GZ /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - x64 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "BUILDING_REF_GL" /FR /GZ /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - x64 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GX 
!ELSEIF  "$(CFG)" == "gamex86xp - x64 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ENDIF

# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl;fi;fd"
# Begin Source File

SOURCE=g_local.h
# End Source File
# Begin Source File

SOURCE=game.h
# End Source File
# Begin Source File

SOURCE=m_actor.h
# End Source File
# Begin Source File

SOURCE=m_berserk.h
# End Source File
# Begin Source File

SOURCE=m_boss2.h
# End Source File
# Begin Source File

SOURCE=m_boss31.h
# End Source File
# Begin Source File

SOURCE=m_boss32.h
# End Source File
# Begin Source File

SOURCE=m_brain.h
# End Source File
# Begin Source File

SOURCE=m_chick.h
# End Source File
# Begin Source File

SOURCE=m_flipper.h
# End Source File
# Begin Source File

SOURCE=m_float.h
# End Source File
# Begin Source File

SOURCE=m_flyer.h
# End Source File
# Begin Source File

SOURCE=m_gladiator.h
# End Source File
# Begin Source File

SOURCE=m_gunner.h
# End Source File
# Begin Source File

SOURCE=m_hover.h
# End Source File
# Begin Source File

SOURCE=m_infantry.h
# End Source File
# Begin Source File

SOURCE=m_insane.h
# End Source File
# Begin Source File

SOURCE=m_medic.h
# End Source File
# Begin Source File

SOURCE=m_mutant.h
# End Source File
# Begin Source File

SOURCE=m_parasite.h
# End Source File
# Begin Source File

SOURCE=m_player.h
# End Source File
# Begin Source File

SOURCE=m_soldier.h
# End Source File
# Begin Source File

SOURCE=m_supertank.h
# End Source File
# Begin Source File

SOURCE=m_tank.h
# End Source File
# Begin Source File

SOURCE=q_shared.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;cnt;rtf;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=game.def
# End Source File
# End Group
# End Target
# End Project

