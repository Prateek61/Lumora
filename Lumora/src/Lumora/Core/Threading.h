#pragma once

#include "Lumora/Core/Defines.h"
#include <thread>
#include <shared_mutex>
#include <atomic>
#include <condition_variable>

namespace Lumora
{
	// Type aliases
	using RWMutex = std::shared_mutex;
	using Mutex = std::mutex;

	template<typename M>
	using ReadLock = std::shared_lock<M>;

	template<typename M>
	using WriteLock = std::unique_lock<M>;

	template<typename T>
	using Atomic = std::atomic<T>;

	using ConditionVariable = std::condition_variable;
}

// Macros
#define LM_MUTEX(mutex) Lumora::RWMutex mutex
#define LM_LOCK_READ(mutex) Lumora::ReadLock<decltype(mutex)> LM_CONCAT(_lock_read_, __LINE__)(mutex)
#define LM_LOCK_WRITE(mutex) Lumora::WriteLock<decltype(mutex)> LM_CONCAT(_lock_write_, __LINE__)(mutex)

#define MUTEX_AUTO_NAME m_AutoMutex
#define LM_MUTEX_AUTO() LM_MUTEX(MUTEX_AUTO_NAME)
#define LM_LOCK_READ_AUTO() LM_LOCK_READ(MUTEX_AUTO_NAME)
#define LM_LOCK_WRITE_AUTO() LM_LOCK_WRITE(MUTEX_AUTO_NAME)
