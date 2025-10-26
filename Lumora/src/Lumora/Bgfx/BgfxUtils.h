#pragma once

#include <bgfx/bgfx.h>

namespace Lumora::BgfxUtils
{
	bool CheckAvailTransientBuffers(uint32_t numVertices, const bgfx::VertexLayout& layout, uint32_t numIndices);
}