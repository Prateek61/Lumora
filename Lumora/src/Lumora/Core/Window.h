#pragma once

#include "Lumora/Common/Base.h"
#include "Lumora/Core/Props.h"

namespace Lumora
{
	class Window
	{
	public:
		explicit Window(const WindowProps& props);
		~Window();

		void OnUpdate();

		uint32_t GetWidth() const { return m_Props.Width; }
		uint32_t GetHeight() const { return m_Props.Height; }

		void* GetNativeWindow() const { return m_NativeWindow; }

		void SetVSync(bool enabled);
		bool IsVSync() const;

	private:
		WindowProps m_Props;
		void* m_NativeWindow = nullptr;
	};
}