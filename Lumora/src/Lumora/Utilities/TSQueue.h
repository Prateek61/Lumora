#pragma once

#include "Lumora/Common/Threading.h"
#include <queue>

namespace Lumora
{
	template<typename T>
	class TSQueue
	{
	public:
		TSQueue() = default;
		~TSQueue() = default;

		std::size_t Size();
		bool Empty();
		void Push(const T& item);
		void Push(T&& item);
		bool TryPop(T& item);
		T PopBlocking();

	private:
		Mutex m_Mutex;
		ConditionVariable m_CV;
		std::queue<T> m_Queue;
	};
}