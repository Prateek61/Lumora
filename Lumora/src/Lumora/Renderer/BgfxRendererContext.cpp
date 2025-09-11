#include "LMPCH.h"
#include "BgfxRendererContext.h"
#include "Lumora/Bgfx/BgfxCallback.h"

#include "bgfx/platform.h"

namespace
{
	Lumora::BgfxCallback s_BgfxCallback = {};
}

namespace
{
	// Helper function to convert RendererContext::API to bgfx::RendererType::Enum
	bgfx::RendererType::Enum ConvertToBGFXRendererType(Lumora::RendererContext::API api)
	{
		switch (api)
		{
		case Lumora::RendererContext::API::OPENGL:
			return bgfx::RendererType::OpenGL;
		case Lumora::RendererContext::API::DIRECTX11:
			return bgfx::RendererType::Direct3D11;
		case Lumora::RendererContext::API::DIRECTX12:
			return bgfx::RendererType::Direct3D12;
		case Lumora::RendererContext::API::VULKAN:
			return bgfx::RendererType::Vulkan;
		case Lumora::RendererContext::API::DEFAULT:
			return bgfx::RendererType::Count; // Let bgfx choose the default renderer
		}

		return bgfx::RendererType::Count;
	}
}

namespace Lumora
{
	void BgfxRendererContext::Init(Window& window)
	{
		LM_PROFILE_FUNCTION();

		m_PlatformData = {};
		m_PlatformData.nwh = window.GetNativeWindow();
		m_PlatformData.ndt = nullptr;

#ifdef GLFW_EXPOSE_NATIVE_X11
		m_PlatformData.ndt = glfwGetX11Display();
#endif

		//bgfx::setPlatformData(m_PlatformData);

		// Init bgfx
		bgfx::Init init;
		init.type = ConvertToBGFXRendererType(GetAPI());
		init.resolution.width = window.GetWidth();
		init.resolution.height = window.GetHeight();
		init.resolution.reset = BGFX_RESET_VSYNC;
		init.platformData = m_PlatformData;

		init.callback = &s_BgfxCallback;

		auto status = bgfx::init(init);

		LM_CORE_ASSERT(status, "BGFX not initialized successfully")

		bgfx::setViewRect(0, 0, 0, static_cast<uint16_t>(window.GetWidth()), static_cast<uint16_t>(window.GetHeight()));
		bgfx::setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0xffffffff, 1.0f, 0);
	}

	void BgfxRendererContext::Shutdown()
	{
		bgfx::shutdown();
	}

	void BgfxRendererContext::BeginFrame()
	{
		bgfx::touch(0); // Ensure view 0 is cleared
	}

	void BgfxRendererContext::EndFrame()
	{
		bgfx::frame();
	}

	void BgfxRendererContext::Resize(uint32_t width, uint32_t height)
	{
		bgfx::reset(width, height, BGFX_RESET_VSYNC);
		bgfx::setViewRect(0, 0, 0, static_cast<uint16_t>(width), static_cast<uint16_t>(height));
	}
}