#include "LMPCH.h"
#include "ImGuiContext.h"

#include <Lumora/Bgfx/BgfxUtils.h>

#include "bgfx/embedded_shader.h"
#include "bx/allocator.h"
#include <bx/math.h>

#include "Lumora/ImGui/EmbeddedAssets/fs_ocornut_imgui.bin.h"
#include "Lumora/ImGui/EmbeddedAssets/fs_imgui_image.bin.h"
#include "Lumora/ImGui/EmbeddedAssets/vs_ocornut_imgui.bin.h"
#include "Lumora/ImGui/EmbeddedAssets/vs_imgui_image.bin.h"


#define IMGUI_FLAGS_NONE        UINT8_C(0x00)
#define IMGUI_FLAGS_ALPHA_BLEND UINT8_C(0x01)

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

	Lumora::LumoraImGuiContext g_LumoraImGuiContext{};
}

namespace Lumora
{
	void LumoraImGuiContext::Init()
	{
		LM_PROFILE_FUNCTION();

		static bx::DefaultAllocator allocator;
		Allocator = &allocator;

		MainView = 255; // Reserve a high view ID for ImGui

		ImGui::SetAllocatorFunctions(MemAlloc, MemFree, nullptr);

		bgfx::RendererType::Enum type = bgfx::getRendererType();
		Program = bgfx::createProgram(
			bgfx::createEmbeddedShader(EmbeddedShaders, type, "vs_ocornut_imgui"),
			bgfx::createEmbeddedShader(EmbeddedShaders, type, "fs_ocornut_imgui"),
			true
		);

		ImageLodEnabled = bgfx::createUniform("u_imageLodEnabled", bgfx::UniformType::Vec4);
		ImageProgram = bgfx::createProgram(
			bgfx::createEmbeddedShader(EmbeddedShaders, type, "vs_imgui_image"),
			bgfx::createEmbeddedShader(EmbeddedShaders, type, "fs_imgui_image"),
			true
		);

		Layout.begin()
			.add(bgfx::Attrib::Position, 2, bgfx::AttribType::Float)
			.add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
			.add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Uint8, true)
			.end();
		Tex = bgfx::createUniform("s_tex", bgfx::UniformType::Sampler);

		ImGui = ImGui::CreateContext();
	}

	void LumoraImGuiContext::Shutdown()
	{
		LM_PROFILE_FUNCTION();

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

		ImGui::DestroyContext(ImGui);

		bgfx::destroy(Tex);
		bgfx::destroy(ImageLodEnabled);
		bgfx::destroy(ImageProgram);
		bgfx::destroy(Program);
	}

	void LumoraImGuiContext::Render(ImDrawData* drawData, bgfx::ViewId viewId)
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
						bx::gather(pix->data, texture_data->GetPixelsAt(rect.x, rect.y),
							texture_data->GetPitch(),
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

		bgfx::setViewName(viewId, "ImGui");
		bgfx::setViewMode(viewId, bgfx::ViewMode::Sequential);

		const bgfx::Caps* caps = bgfx::getCaps();
		{
			float ortho[16];
			float x = drawData->DisplayPos.x;
			float y = drawData->DisplayPos.y;
			float width = drawData->DisplaySize.x;
			float height = drawData->DisplaySize.y;

			bx::mtxOrtho(ortho, x, x + width, y + height, y, 0.0f, 1000.0f, 0.0f, caps->homogeneousDepth);
			bgfx::setViewTransform(viewId, nullptr, ortho);
			bgfx::setViewRect(viewId, 0, 0, static_cast<uint16_t>(width), static_cast<uint16_t>(height));
		}

		const ImVec2 clips = drawData->DisplayPos; // (0, 0) unless using multi-viewports
		const ImVec2 clipScale = drawData->FramebufferScale;
		// (1,1) unless using retina display which are often (2,2)

		// Render command lists
		for (int32_t ii = 0, num = drawData->CmdListsCount; ii < num; ++ii)
		{
			bgfx::TransientVertexBuffer tvb;
			bgfx::TransientIndexBuffer tib;

			const ImDrawList* draw_list = drawData->CmdLists[ii];
			uint32_t num_vertices = static_cast<uint32_t>(draw_list->VtxBuffer.Size);
			uint32_t num_indices = static_cast<uint32_t>(draw_list->IdxBuffer.Size);

			if (!Lumora::BgfxUtils::CheckAvailTransientBuffers(num_vertices, Layout, num_indices))
			{
				LM_CORE_RENDERER_WARN("Unable to allocate transient buffers for ImGui rendering.");
				break;
			}

			bgfx::allocTransientVertexBuffer(&tvb, num_vertices, Layout);
			bgfx::allocTransientIndexBuffer(&tib, num_indices, sizeof(ImDrawIdx) == 4);

			auto vertices = reinterpret_cast<ImDrawVert*>(tvb.data);
			bx::memCopy(vertices, draw_list->VtxBuffer.begin(), num_vertices * sizeof(ImDrawVert));

			auto indices = reinterpret_cast<ImDrawIdx*>(tib.data);
			bx::memCopy(indices, draw_list->IdxBuffer.begin(), num_indices * sizeof(ImDrawIdx));

			bgfx::Encoder* encoder = bgfx::begin();

			for (const ImDrawCmd* cmd = draw_list->CmdBuffer.begin(), *cmd_end = draw_list->CmdBuffer.end(); cmd !=
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
					bgfx::ProgramHandle program = Program;

					const ImTextureID tex_id = cmd->GetTexID();

					if (ImTextureID_Invalid != tex_id)
					{
						auto tex = bx::bitCast<TextureBgfx>(tex_id);

						state |= 0 != (tex.flags * IMGUI_FLAGS_ALPHA_BLEND)
							? BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_SRC_ALPHA,
								BGFX_STATE_BLEND_INV_SRC_ALPHA)
							: BGFX_STATE_NONE;
						th = tex.handle;

						if (0 != tex.mip)
						{
							const float lod_enabled[4] = { static_cast<float>(tex.mip), 1.0f, 0.0f, 0.0f };
							encoder->setUniform(ImageLodEnabled, lod_enabled);
							program = ImageProgram;
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

					if (clip_rect.x < static_cast<float>(display_width) &&
						clip_rect.y < static_cast<float>(display_height) &&
						clip_rect.z >= 0.0f &&
						clip_rect.w >= 0.0f)
					{
						const uint16_t xx = static_cast<uint16_t>(bx::max(clip_rect.x, 0.0f));
						const uint16_t yy = static_cast<uint16_t>(bx::max(clip_rect.y, 0.0f));
						encoder->setScissor(xx, yy,
							static_cast<uint16_t>(bx::min(clip_rect.z, 65535.0f) - static_cast<
								float>(
									xx)),
							static_cast<uint16_t>(bx::min(clip_rect.w, 65535.0f) - static_cast<
								float>(
									yy))
						);

						encoder->setState(state);
						encoder->setTexture(0, Tex, th);
						encoder->setVertexBuffer(0, &tvb, cmd->VtxOffset, num_vertices);
						encoder->setIndexBuffer(&tib, cmd->IdxOffset, cmd->ElemCount);
						encoder->submit(viewId, program);
					}
				}
			}

			bgfx::end(encoder);
		}
	}

	LumoraImGuiContext& LumoraImGuiContext::Get()
	{
		return g_LumoraImGuiContext;
	}

}
