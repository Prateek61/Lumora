#pragma once

#include "Lumora/Glyph/ImGuiBackend.h"

struct GLFWwindow;

namespace Lumora::Lumen
{
	class VKRenderDevice;
}

namespace Lumora::Glyph
{
	class VulkanImGuiBackend final : public ImGuiBackend
	{
	public:
		void Init(Flux::Window& window, Lumen::RenderDevice& device) override;
		void Shutdown() override;

		void NewFrame() override;
		void RenderDrawData(ImDrawData* draw_data) override;
		void UpdateAndRenderPlatformWindows() override;
	private:
		// ImGui keeps these across frames, so its pool cannot be the device's per frame one
		static constexpr uint32_t DescriptorPoolSize = 64;

		GLFWwindow* m_GLFWWindow = nullptr;

		Lumen::VKRenderDevice* m_Device = nullptr;

		uint32_t m_MinImageCount = 0;
	};
}
