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
		
	}
}

namespace Lumora::Glyph
{
	class ImGuiBackend
	{
	public:
		virtual ~ImGuiBackend() = default;

		virtual void Init(Flux::Window& window) = 0;
		virtual void Shutdown() = 0;
		virtual void NewFrame() = 0;
		virtual void RenderDrawData(ImDrawData* draw_data) = 0;

		static Scope<ImGuiBackend> Create(Lumen::RenderAPI api);
	};
}
