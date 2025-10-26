#include "LMPCH.h"
#include "BgfxUtils.h"

namespace Lumora::BgfxUtils
{
	bool CheckAvailTransientBuffers(uint32_t numVertices, const bgfx::VertexLayout& layout, uint32_t numIndices)
	{
		return numVertices == bgfx::getAvailTransientVertexBuffer(numVertices, layout) &&
			(0 == numIndices || numIndices == bgfx::getAvailTransientIndexBuffer(numIndices));
	}
}
