#include "LMPCH.h"
#include "Renderer2D.h"

#include "bgfx/bgfx.h"

namespace
{
	constexpr char vertexSrc[] = R"(
		$input a_position, a_color0
		$output v_color0
		#include "bgfx_shader.sh"
		
		void main()
		{
			v_color0 = a_color0;
			gl_Position = vec4(a_position, 0.0, 1.0);
		}
	)";

	constexpr char fragmentSrc[] = R"(
		$input v_color0
		#include "bgfx_shader.sh"
		
		void main()
		{
			gl_FragColor = v_color0;
		}
	)";

	struct Vertex
	{
		float x, y, z;
		uint32_t rgba;
		static void InitLayout(bgfx::VertexLayout& layout)
		{
			layout.begin()
				.add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
				.add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Uint8, true)
				.end();
		}
	};

	bgfx::VertexLayout s_Layout;
	bgfx::ProgramHandle s_Program = BGFX_INVALID_HANDLE;
}

namespace Lumora
{
	void Renderer2D::Init()
	{
		Vertex::InitLayout(s_Layout);
	}
}