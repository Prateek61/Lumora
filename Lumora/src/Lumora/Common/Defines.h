#pragma once

#include "Lumora/Common/PlatformDetection.h"

#define LM_ENABLE_CORE_LOG
#define LM_ENABLE_CLIENT_LOG
#define LM_ENABLE_LUA_LOG

#define LM_ENABLE_ASSERTS
//#define LM_ENABLE_PERFORMANCE_PROFILING

#define SOL_ALL_SAFETIES_ON 1

#define BIT(x) (1 << (x))
#define PI 3.14159265359f

#define EXPAND_MACRO(x) x
#define STRINGIFY_MACRO(x) #x

#if defined(LM_DEBUG) || defined(LM_RELEASE)
#define DEBUG_ONLY(x) x
#else
#define DEBUG_ONLY(x)
#endif

// Debug Break
#ifndef LM_DIST
    #if defined(LM_PLATFORM_WINDOWS)
        #define LM_DEBUGBREAK() __debugbreak()
    #elif defined(LM_PLATFORM_LINUX)
        #include <signal.h>
        #define LM_DEBUGBREAK() raise(SIGTRAP)
    #else
        #error "Platform doesn't support debugbreak yet!"
    #endif
#else
    #define LM_DEBUGBREAK()
#endif

#define LM_CONCAT_IMPL(x, y) x##y
#define LM_CONCAT(x, y) LM_CONCAT_IMPL(x, y)