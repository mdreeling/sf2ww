//
//  sf2.h
//  MT2
//
//  Created by Ben Torkington on 3/10/14.
//
//

#ifndef MT2_sf2_h
#define MT2_sf2_h

#include <windows.h>

#include "sf2types.h"
#include "sf2macros.h"
#include "sf2const.h"

// Debugging Switches

//#define REDHAMMER_PLAYGROUND

// #define FISTBLUE_SKIP_COPYRIGHT

// Debug levels
#define FISTBLUE_DEBUG_SM 1
#define FISTBLUE_DEBUG_AI 0
#define FISTBLUE_DEBUG_GEN 2
#define FISTBLUE_DEBUG_SPRITE 3

// Override geographical location for ROMset
//#define FISTBLUE_SF2UA_JAP
//#define FISTBLUE_SF2UA_ETC

#define REDHAMMER 1

// Jumpers
#define FISTBLUE_JUMPER_0   0x00
#define FISTBLUE_JUMPER_1   0x00
#define FISTBLUE_JUMPER_2   0x00


#ifdef REDHAMMER
#include "strings.h"
#include <stdio.h>
#include "redhammer.h"

#ifdef _WIN32
#include <shlwapi.h>
#pragma comment(lib, "Shlwapi.lib")

// Replace basename with PathFindFileName for Windows
#define DEBUG_GEN(fmt, ...) \
do { if (FISTBLUE_DEBUG_GEN) fprintf(stderr, "%s:%d:%s():" fmt, \
PathFindFileName(__FILE__), __LINE__, __func__, __VA_ARGS__); } while (0)

#else
#include <libgen.h>  // For basename on Unix-like systems

#define DEBUG_GEN(fmt, ...) \
do { if (FISTBLUE_DEBUG_GEN) fprintf(stderr, "%s:%d:%s():" fmt, \
basename(__FILE__), __LINE__, __func__, __VA_ARGS__); } while (0)

#endif // _WIN32

#endif  // REDHAMMER



#endif  // MT2_sf2_h
