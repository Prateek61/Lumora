#pragma once

// Platform detection using predefined macros
#ifdef _WIN32
    #ifdef _WIN64
        #define LM_PLATFORM_WINDOWS
    #else
        #error "x86 Builds are not supported!"
    #endif
#elif defined(__APPLE__) || defined(__MACH__)
    #include <TargetConditionals.h>
    #if TARGET_IPHONE_SIMULATOR == 1
        #define LM_PLATFORM_IOS
        #error "iOS simulator is not supported!"
    #elif TARGET_OS_IPHONE == 1
        #define LM_PLATFORM_IOS
        #error "iOS is not supported!"
    #elif TARGET_OS_MAC == 1
        #define LM_PLATFORM_MACOS
        #error "macOS is not supported!"
    #else
        #error "Unknown Apple platform!"
    #endif
#elif defined(__ANDROID__)
    #define LM_PLATFORM_ANDROID
    #error "Android is not supported!"
#elif defined(__linux__)
    #define LM_PLATFORM_LINUX
#else
    #error "Unknown platform!"
#endif