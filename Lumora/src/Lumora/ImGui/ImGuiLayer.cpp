#include "LMPCH.h"
#include "ImGuiLayer.h"

#include "bgfx/embedded_shader.h"
#include "bx/allocator.h"
#include "bx/math.h"
#include "bx/timer.h"
#include "Lumora/Bgfx/BgfxUtils.h"
#include "Lumora/Core/Application.h"
#include "Lumora/Event/ApplicationEvent.h"
#include "Lumora/Event/KeyEvent.h"
#include "Lumora/Event/MouseEvent.h"

#include "Lumora/ImGui/EmbeddedAssets/fs_ocornut_imgui.bin.h"
#include "Lumora/ImGui/EmbeddedAssets/fs_imgui_image.bin.h"
#include "Lumora/ImGui/EmbeddedAssets/vs_ocornut_imgui.bin.h"
#include "Lumora/ImGui/EmbeddedAssets/vs_imgui_image.bin.h"

namespace
{
	const bgfx::EmbeddedShader EmbeddedShaders[] = {
		BGFX_EMBEDDED_SHADER(vs_ocornut_imgui),
		BGFX_EMBEDDED_SHADER(fs_ocornut_imgui),
		BGFX_EMBEDDED_SHADER(vs_imgui_image),
		BGFX_EMBEDDED_SHADER(fs_imgui_image),

		BGFX_EMBEDDED_SHADER_END()
	};

	bx::AllocatorI* Allocator;

	void* MemAlloc(size_t size, void* userData)
	{
		LM_PROFILE_FUNCTION();

		BX_UNUSED(userData)
		return bx::alloc(Allocator, size);
	}

	void MemFree(void* ptr, void* userData)
	{
		LM_PROFILE_FUNCTION();

		BX_UNUSED(userData)
		bx::realloc(Allocator, ptr, 0, 0);
	}

	struct TextureBgfx
	{
		bgfx::TextureHandle handle;
		uint8_t flags;
		uint8_t mip;
		uint32_t unused;
	};

	ImGuiKey MapKeyToImGuiKey(Lumora::KeyCode k);
}

namespace Lumora
{
	ImGuiLayer::ImGuiLayer(float fontSize)
	{
		LM_PROFILE_FUNCTION();

		static bx::DefaultAllocator allocator;
		Allocator = &allocator;

		m_ViewId = 255; // Reserve a high view ID for ImGui
		m_LastScroll = 0;
		m_Last = bx::getHPCounter();

		ImGui::SetAllocatorFunctions(MemAlloc, MemFree, nullptr);

		bgfx::RendererType::Enum type = bgfx::getRendererType();
		m_Program = bgfx::createProgram(
			bgfx::createEmbeddedShader(EmbeddedShaders, type, "vs_ocornut_imgui"),
			bgfx::createEmbeddedShader(EmbeddedShaders, type, "fs_ocornut_imgui"),
			true
		);

		m_ImageLodEnabled = bgfx::createUniform("u_imageLodEnabled", bgfx::UniformType::Vec4);
		m_ImageProgram = bgfx::createProgram(
			bgfx::createEmbeddedShader(EmbeddedShaders, type, "vs_imgui_image"),
			bgfx::createEmbeddedShader(EmbeddedShaders, type, "fs_imgui_image"),
			true
		);

		m_Layout.begin()
		        .add(bgfx::Attrib::Position, 2, bgfx::AttribType::Float)
		        .add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
		        .add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Uint8, true)
		        .end();

		m_Tex = bgfx::createUniform("s_tex", bgfx::UniformType::Sampler);

		m_ImGui = nullptr;
	}

	void ImGuiLayer::OnAttach()
	{
		LM_PROFILE_FUNCTION();

		IMGUI_CHECKVERSION();

		auto& app = Application::Get();
		auto& window = app.GetWindow();

		m_ImGui = ImGui::CreateContext();

		ImGuiIO& io = ImGui::GetIO();
		(void)io;
		io.DisplaySize = ImVec2(static_cast<float>(window.GetWidth()), static_cast<float>(window.GetHeight()));

		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad; // Enable Gamepad Controls
		io.IniFilename = nullptr; // Disable .ini file

		// Style
		SetStyle(true);


		io.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset | ImGuiBackendFlags_RendererHasTextures;
		io.ConfigDebugHighlightIdConflicts = false;
		DEBUG_ONLY
		(
			io.ConfigDebugHighlightIdConflicts = true;
		)

		InitializeDockSpace();
		// TODO: ImGuizmo
	}

	void ImGuiLayer::OnDetach()
	{
		LM_PROFILE_FUNCTION();

		// TODO: ImGuizmo

		for (ImTextureData* texture_data : ImGui::GetPlatformIO().Textures)
		{
			if (1 == texture_data->RefCount)
			{
				auto tex = bx::bitCast<TextureBgfx>(texture_data->GetTexID());
				bgfx::destroy(tex.handle);
				texture_data->SetTexID(ImTextureID_Invalid);
				texture_data->SetStatus(ImTextureStatus_Destroyed);
			}
		}

		ShutDownDockSpace();
		ImGui::DestroyContext(m_ImGui);

		bgfx::destroy(m_Tex);

		bgfx::destroy(m_ImageLodEnabled);
		bgfx::destroy(m_ImageProgram);
		bgfx::destroy(m_Program);

		Allocator = nullptr;
	}

	void ImGuiLayer::OnEvent(Event& e)
	{
		LM_PROFILE_FUNCTION();

		ImGuiIO& io = ImGui::GetIO();

		EventDispatcher dispatcher(e);

		dispatcher.Dispatch<KeyTypedEvent>([&](const KeyTypedEvent& ke)
		{
			io.AddInputCharacter(ke.GetKeyCode());
			return false;
		});
		dispatcher.Dispatch<KeyPressedEvent>([&](const KeyPressedEvent& ke)
		{
			return false;
		});

		dispatcher.Dispatch<MouseScrolledEvent>([&](const MouseScrolledEvent& me)
		{
			io.AddMouseWheelEvent(0.0f, me.GetYOffset());
			return false;
		});
		dispatcher.Dispatch<MouseMovedEvent>([&](const MouseMovedEvent& me)
		{
			io.AddMousePosEvent(me.GetX(), me.GetY());
			return false;
		});
		dispatcher.Dispatch<MouseButtonPressedEvent>([&](const MouseButtonPressedEvent& me)
		{
			io.AddMouseButtonEvent(me.GetMouseButton(), true);
			return false;
		});
		dispatcher.Dispatch<MouseButtonReleasedEvent>([&](const MouseButtonReleasedEvent& me)
		{
			io.AddMouseButtonEvent(me.GetMouseButton(), false);
			return false;
		});

		if (m_Block)
		{
			e.Handled |= e.IsInCategory(EventCategoryMouse) && io.WantCaptureMouse;
			e.Handled |= e.IsInCategory(EventCategoryKeyboard) && io.WantCaptureKeyboard;
		}
	}

	void ImGuiLayer::OnImGuiRender(TimeStep ts)
	{
		LM_PROFILE_FUNCTION();

		ImGuiIO& io = ImGui::GetIO();
		float del_time = ts.GetSeconds();
		io.DeltaTime = del_time > 0.0f ? del_time : (1.0f / 60.0f);

		static bool show = true;
		ImGui::ShowDemoWindow(&show);
	}

	void ImGuiLayer::BeginImGuiFrame()
	{
		LM_PROFILE_FUNCTION();

		ImGui::NewFrame();

		// TODO: ImGuizmo
	}

	void ImGuiLayer::EndImGuiFrame()
	{
		LM_PROFILE_FUNCTION();

		ImGui::Render();
		Render(ImGui::GetDrawData());
	}

	void ImGuiLayer::SetStyle(bool dark)
	{
		LM_PROFILE_FUNCTION();

		ImGuiStyle& style = ImGui::GetStyle();
		if (dark)
		{
			ImGui::StyleColorsDark(&style);
		}
		else
		{
			ImGui::StyleColorsLight(&style);
		}

		style.FrameRounding = 4.0f;
		style.WindowBorderSize = 0.0f;
	}

	uint32_t ImGuiLayer::GetActiveWidgetId()
	{
		return GImGui->ActiveId;
	}

	void ImGuiLayer::Render(ImDrawData* drawData)
	{
		LM_PROFILE_FUNCTION();

		// Avoid rendering when minimized, scale coordinates for retina display (screen coordinates != framebuffer coordinates)
		int32_t display_width = static_cast<int32_t>(drawData->DisplaySize.x * drawData->FramebufferScale.x);
		int32_t display_height = static_cast<int32_t>(drawData->DisplaySize.y * drawData->FramebufferScale.y);
		if (display_width <= 0 || display_height <= 0)
		{
			return;
		}

		if (nullptr != drawData->Textures)
		{
			for (ImTextureData* texture_data : *drawData->Textures)
			{
				switch (texture_data->Status)
				{
				case ImTextureStatus_WantCreate:
					{
						TextureBgfx tex = {
							.handle = bgfx::createTexture2D(
								static_cast<uint16_t>(texture_data->Width),
								static_cast<uint16_t>(texture_data->Height),
								false,
								1,
								bgfx::TextureFormat::BGRA8,
								0
							),
							.flags = IMGUI_FLAGS_ALPHA_BLEND,
							.mip = 0,
							.unused = 0
						};

						bgfx::setName(tex.handle, "ImGui Font Atlas");
						bgfx::updateTexture2D(tex.handle, 0, 0, 0, 0,
						                      bx::narrowCast<uint16_t>(texture_data->Width),
						                      bx::narrowCast<uint16_t>(texture_data->Height),
						                      bgfx::copy(texture_data->GetPixels(), texture_data->GetSizeInBytes())
						);

						texture_data->SetTexID(bx::bitCast<ImTextureID>(tex));
						texture_data->SetStatus(ImTextureStatus_OK);
					}
					break;

				case ImTextureStatus_WantDestroy:
					{
						auto tex = bx::bitCast<TextureBgfx>(texture_data->GetTexID());
						bgfx::destroy(tex.handle);
						texture_data->SetTexID(ImTextureID_Invalid);
						texture_data->SetStatus(ImTextureStatus_Destroyed);
					}
					break;

				case ImTextureStatus_WantUpdates:
					{
						auto tex = bx::bitCast<TextureBgfx>(texture_data->GetTexID());

						for (ImTextureRect& rect : texture_data->Updates)
						{
							const uint32_t bpp = texture_data->BytesPerPixel;
							const bgfx::Memory* pix = bgfx::alloc(rect.h * rect.w * bpp);
							bx::gather(pix->data, texture_data->GetPixelsAt(rect.x, rect.y), texture_data->GetPitch(),
							           rect.w * bpp, rect.h);
							bgfx::updateTexture2D(tex.handle, 0, 0, rect.x, rect.y, rect.w, rect.h, pix);
						}
					}
					break;
				default:
					break;
				}
			}
		}

		bgfx::setViewName(m_ViewId, "ImGui");
		bgfx::setViewMode(m_ViewId, bgfx::ViewMode::Sequential);

		const bgfx::Caps* caps = bgfx::getCaps();
		{
			float ortho[16];
			float x = drawData->DisplayPos.x;
			float y = drawData->DisplayPos.y;
			float width = drawData->DisplaySize.x;
			float height = drawData->DisplaySize.y;

			bx::mtxOrtho(ortho, x, x + width, y + height, y, 0.0f, 1000.0f, 0.0f, caps->homogeneousDepth);
			bgfx::setViewTransform(m_ViewId, nullptr, ortho);
			bgfx::setViewRect(m_ViewId, 0, 0, static_cast<uint16_t>(width), static_cast<uint16_t>(height));
		}

		const ImVec2 clips = drawData->DisplayPos; // (0, 0) unless using multi-viewports
		const ImVec2 clipScale = drawData->FramebufferScale; // (1,1) unless using retina display which are often (2,2)

		// Render command lists
		for (int32_t ii = 0, num = drawData->CmdListsCount; ii < num; ++ii)
		{
			bgfx::TransientVertexBuffer tvb;
			bgfx::TransientIndexBuffer tib;

			const ImDrawList* draw_list = drawData->CmdLists[ii];
			uint32_t num_vertices = static_cast<uint32_t>(draw_list->VtxBuffer.Size);
			uint32_t num_indices = static_cast<uint32_t>(draw_list->IdxBuffer.Size);

			if (!BgfxUtils::CheckAvailTransientBuffers(num_vertices, m_Layout, num_indices))
			{
				LM_CORE_RENDERER_WARN("Unable to allocate transient buffers for ImGui rendering.");
				break;
			}

			bgfx::allocTransientVertexBuffer(&tvb, num_vertices, m_Layout);
			bgfx::allocTransientIndexBuffer(&tib, num_indices, sizeof(ImDrawIdx) == 4);

			auto vertices = reinterpret_cast<ImDrawVert*>(tvb.data);
			bx::memCopy(vertices, draw_list->VtxBuffer.begin(), num_vertices * sizeof(ImDrawVert));

			auto indices = reinterpret_cast<ImDrawIdx*>(tib.data);
			bx::memCopy(indices, draw_list->IdxBuffer.begin(), num_indices * sizeof(ImDrawIdx));

			bgfx::Encoder* encoder = bgfx::begin();

			for (const ImDrawCmd *cmd = draw_list->CmdBuffer.begin(), *cmd_end = draw_list->CmdBuffer.end(); cmd !=
			     cmd_end; ++cmd)
			{
				if (cmd->UserCallback)
				{
					cmd->UserCallback(draw_list, cmd);
				}
				else if (0 != cmd->ElemCount)
				{
					uint64_t state = BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_MSAA;

					bgfx::TextureHandle th = BGFX_INVALID_HANDLE;
					bgfx::ProgramHandle program = m_Program;

					const ImTextureID tex_id = cmd->GetTexID();

					if (ImTextureID_Invalid != tex_id)
					{
						auto tex = bx::bitCast<TextureBgfx>(tex_id);

						state |= 0 != (tex.flags * IMGUI_FLAGS_ALPHA_BLEND)
							         ? BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_SRC_ALPHA, BGFX_STATE_BLEND_INV_SRC_ALPHA)
							         : BGFX_STATE_NONE;
						th = tex.handle;

						if (0 != tex.mip)
						{
							const float lod_enabled[4] = {static_cast<float>(tex.mip), 1.0f, 0.0f, 0.0f};
							encoder->setUniform(m_ImageLodEnabled, lod_enabled);
							program = m_ImageProgram;
						}
					}
					else
					{
						state |= BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_SRC_ALPHA, BGFX_STATE_BLEND_INV_SRC_ALPHA);
					}

					// Project scissor/clipping rectangles into framebuffer space
					ImVec4 clip_rect;
					clip_rect.x = (cmd->ClipRect.x - clips.x) * clipScale.x;
					clip_rect.y = (cmd->ClipRect.y - clips.y) * clipScale.y;
					clip_rect.z = (cmd->ClipRect.z - clips.x) * clipScale.x;
					clip_rect.w = (cmd->ClipRect.w - clips.y) * clipScale.y;

					if (clip_rect.x < display_width &&
						clip_rect.y < display_height &&
						clip_rect.z >= 0.0f &&
						clip_rect.w >= 0.0f)
					{
						const uint16_t xx = static_cast<uint16_t>(bx::max(clip_rect.x, 0.0f));
						const uint16_t yy = static_cast<uint16_t>(bx::max(clip_rect.y, 0.0f));
						encoder->setScissor(xx, yy,
						                    static_cast<uint16_t>(bx::min(clip_rect.z, 65535.0f) - static_cast<float>(
							                    xx)),
						                    static_cast<uint16_t>(bx::min(clip_rect.w, 65535.0f) - static_cast<float>(
							                    yy))
						);

						encoder->setState(state);
						encoder->setTexture(0, m_Tex, th);
						encoder->setVertexBuffer(0, &tvb, cmd->VtxOffset, num_vertices);
						encoder->setIndexBuffer(&tib, cmd->IdxOffset, cmd->ElemCount);
						encoder->submit(m_ViewId, program);
					}
				}
			}

			bgfx::end(encoder);
		}
	}
}


namespace
{
	using namespace Lumora;

	ImGuiKey MapKeyToImGuiKey(Lumora::KeyCode k)
	{
		switch (k)
		{
		case Key::A: return ImGuiKey_A;
		}
	}
}


