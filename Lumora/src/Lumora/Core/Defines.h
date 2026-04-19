#pragma once

#include "Lumora/Core/PlatformDetection.h"

#define LM_LOG_LEVEL 6

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

// GLFW Native Access
#ifdef LM_PLATFORM_WINDOWS
#define GLFW_EXPOSE_NATIVE_WIN32
#elif defined(LM_PLATFORM_LINUX)
    #define GLFW_EXPOSE_NATIVE_X11
#elif defined(LM_PLATFORM_MACOS)
        #define GLFW_EXPOSE_NATIVE_COCOA
#else
        #error "Not implemented!"
#endif

// Resolve which function signature macro will be used. Note that this only
// is resolved when the (pre)compiler starts, so the syntax highlighting
// could mark the wrong one in your editor!
#if defined(__GNUC__) || (defined(__MWERKS__) && (__MWERKS__ >= 0x3000)) || (defined(__ICC) && (__ICC >= 600)) || defined(__ghs__)
#define LM_FUNC_SIG __PRETTY_FUNCTION__
#elif defined(__DMC__) && (__DMC__ >= 0x810)
#define LM_FUNC_SIG __PRETTY_FUNCTION__
#elif (defined(__FUNCSIG__) || (_MSC_VER))
#define LM_FUNC_SIG __FUNCSIG__
#elif (defined(__INTEL_COMPILER) && (__INTEL_COMPILER >= 600)) || (defined(__IBMCPP__) && (__IBMCPP__ >= 500))
#define LM_FUNC_SIG __FUNCTION__
#elif defined(__BORLANDC__) && (__BORLANDC__ >= 0x550)
#define LM_FUNC_SIG __FUNC__
#elif defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901)
#define LM_FUNC_SIG __func__
#elif defined(__cplusplus) && (__cplusplus >= 201103)
#define LM_FUNC_SIG __func__
#else
#define LM_FUNC_SIG "LM_FUNC_SIG unknown!"
#endif
