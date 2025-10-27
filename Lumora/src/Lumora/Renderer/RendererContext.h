#pragma once

#include "Lumora/Core/Window.h"

namespace Lumora
{
	class RendererContext
	{
	public:
		enum class API
		{
			DEFAULT = 0,
			OPENGL,
			DIRECTX11,
			VULKAN,
			DIRECTX12
		};
		static API GetAPI() { return s_API; }
		static void SetAPI(API api) { s_API = api; }
		static const char* APIToString(API api);
		static API StringToAPI(std::string_view api);

	public:
		virtual ~RendererContext() = default;
		virtual void Init(Window& window) = 0;
		virtual void Shutdown() = 0;
		virtual void BeginFrame() = 0;
		virtual void EndFrame() = 0;
		virtual void Resize(uint32_t width, uint32_t height) = 0;
		virtual void SetClearColor(uint32_t rgba) = 0;

		static Scope<RendererContext> Create(Window& window);
	private:
		static API s_API;
	};
}