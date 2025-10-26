#include "LMPCH.h"
#include "BgfxRendererContext.h"
#include "Lumora/Bgfx/BgfxCallback.h"

#include "bgfx/platform.h"

namespace
{
	Lumora::BgfxCallback s_BgfxCallback = {};

	std::string GetVendorName(uint16_t vendorId)
	{
		switch (vendorId)
		{
		case BGFX_PCI_ID_AMD: return "AMD";
		case BGFX_PCI_ID_APPLE: return "APPLE";
		case BGFX_PCI_ID_ARM: return "ARM";
		case BGFX_PCI_ID_INTEL: return "INTEL";
		case BGFX_PCI_ID_MICROSOFT: return "MICROSOFT";
		case BGFX_PCI_ID_NVIDIA: return "NVIDIA";
		case BGFX_PCI_ID_SOFTWARE_RASTERIZER: return "SOFTWARE_RASTERIZER";
		case BGFX_PCI_ID_NONE: return "NONE";
		default: return "UNKNOWN";
		}
	}
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

		LM_CORE_RENDERER_DEBUG("Initializing BGFX with API ({})", APIToString(GetAPI()));

		m_PlatformData = {};
		m_PlatformData.nwh = window.GetNativeWindow();
		m_PlatformData.ndt = nullptr;

#ifdef GLFW_EXPOSE_NATIVE_X11
		m_PlatformData.ndt = glfwGetX11Display();
#endif

		// bgfx::setPlatformData(m_PlatformData);

		// Init bgfx
		bgfx::Init init;
		init.type = ConvertToBGFXRendererType(GetAPI());
		init.resolution.width = window.GetWidth();
		init.resolution.height = window.GetHeight();
		init.resolution.reset = BGFX_RESET_VSYNC;
		init.platformData = m_PlatformData;

		init.callback = &s_BgfxCallback;

		bool status;

		{
			LM_PROFILE_SCOPE("bgfx::init()");
			status = bgfx::init(init);
		}

		LM_CORE_ASSERT(status, "BGFX not initialized successfully")

		auto caps = bgfx::getCaps();
		auto vendorName = GetVendorName(caps->vendorId);

		//LM_CORE_RENDERER_INFO("Initialized BGFX with API-({}) DEVICE_ID-({}) VENDOR_ID-({})", bgfx::getRendererName(bgfx::getRendererType()), bgfx::);
		LM_CORE_RENDERER_INFO("Initialized BGFX with API({}), VENDOR({}), DEVICE-ID({:X}), VENDOR-ID({:X})", bgfx::getRendererName(bgfx::getRendererType()), vendorName, caps->deviceId, caps->vendorId);

		bgfx::setViewRect(0, 0, 0, static_cast<uint16_t>(window.GetWidth()), static_cast<uint16_t>(window.GetHeight()));
		SetClearColor(0x000000ff);

		bgfx::setDebug(BGFX_DEBUG_TEXT);
	}

	void BgfxRendererContext::Shutdown()
	{
		LM_PROFILE_FUNCTION();

		bgfx::shutdown();
	}

	void BgfxRendererContext::BeginFrame()
	{
		LM_PROFILE_FUNCTION();

		bgfx::touch(0); // Ensure view 0 is cleared
	}

	void BgfxRendererContext::EndFrame()
	{
		LM_PROFILE_FUNCTION();

		bgfx::frame();
	}

	void BgfxRendererContext::Resize(uint32_t width, uint32_t height)
	{
		LM_PROFILE_FUNCTION();

		bgfx::reset(width, height, BGFX_RESET_VSYNC);
		bgfx::setViewRect(0, 0, 0, static_cast<uint16_t>(width), static_cast<uint16_t>(height));
	}

	void BgfxRendererContext::SetClearColor(uint32_t rgba)
	{
		LM_PROFILE_FUNCTION();

		bgfx::setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, rgba, 1.0f, 0);
	}
}