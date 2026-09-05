#pragma once

#include "Lumora/Lumen/RenderAPI.h"
#include "Lumora/Core/SmartPointers.h"

// Forward Declaration - keep imgui.h out of public headers
struct ImDrawData;

namespace Lumora
{
	namespace Flux
	{
		class Window;
	}

	namespace Lumen
	{
		class RenderDevice;
	}
}

namespace Lumora::Glyph
{
	class ImGuiBackend
	{
	public:
		virtual ~ImGuiBackend() = default;

		virtual void Init(Flux::Window& window, Lumen::RenderDevice& device) = 0;
		virtual void Shutdown() = 0;
		virtual void NewFrame() = 0;
		virtual void RenderDrawData(ImDrawData* draw_data) = 0;

		virtual void UpdateAndRenderPlatformWindows() = 0;

		static Scope<ImGuiBackend> Create(Lumen::RenderAPI api);
	};
}
