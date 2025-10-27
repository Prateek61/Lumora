#pragma once

#include "imgui.h"

namespace Lumora::ImUI
{
	template <typename Base>
	class ScopeWrapper
	{
	public:
		using WrappedType = Base;
		using SelfType = ScopeWrapper<Base>;

		constexpr ScopeWrapper(bool ret, bool calledBegin = true) : m_Ret(ret), m_calledBegin(calledBegin)
		{
		}

		~ScopeWrapper()
		{
			if (m_calledBegin)
				Base::End();
		}

		template <typename Func>
		constexpr bool operator&&(const Func& func)
		{
			if (m_Ret)
				func();
			return m_Ret;
		}

		constexpr operator bool() const
		{
			return m_Ret;
		}

	protected:
		const bool m_Ret;
		const bool m_calledBegin;
	};

	class Begin : public ScopeWrapper<Begin>
	{
	public:
		Begin(const char* name, bool* open = nullptr, ImGuiWindowFlags flags = 0)
			: ScopeWrapper<Begin>(ImGui::Begin(name, open, flags))
		{
		}

		static void End()
		{
			ImGui::End();
		}

		static ScopeWrapper<Begin> If(bool condition, const char* name, bool* open = nullptr,
		                                    ImGuiWindowFlags flags = 0)
		{
			if (condition)
				return {ImGui::Begin(name, open, flags), true};

			return {false, false};
		}
	};
}
