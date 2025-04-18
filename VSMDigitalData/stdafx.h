// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#define _CRT_SECURE_NO_WARNINGS
#ifdef WIN32
#define VSM_API_VERSION 110
#endif
#define ARCH_64 1

#include "targetver.h"

#ifdef VSMDIGITALDISPLAY_EXPORTS
	#define DLLEXPORT __declspec(dllexport)
#else
	#define DLLEXPORT __declspec(dllimport)
#endif


#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files
#include <windows.h>
#include <stdlib.h>



// reference additional headers your program requires here
