#include "LMPCH.h"
#include "GLRenderDevice.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

namespace
{
	void OpenGLMessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length,
	                           const GLchar* message, const void* userParam)
	{
		switch (severity)
		{
		case GL_DEBUG_SEVERITY_HIGH: LM_CORE_FATAL(message);
			return;
		case GL_DEBUG_SEVERITY_MEDIUM: LM_CORE_ERROR(message);
			return;
		case GL_DEBUG_SEVERITY_LOW: LM_CORE_WARN(message);
			return;
		case GL_DEBUG_SEVERITY_NOTIFICATION: LM_CORE_TRACE(message);
			return;
		default:
			return;
		}
	}


	void CheckGLError(const spdlog::source_loc& loc)
	{
		GLenum err;
		while ((err = glGetError()) != GL_NO_ERROR)
		{
			Lumora::Log::GetCoreLogger()->log(loc, spdlog::level::err, "OpenGL Error: {:#x}", err);
		}
	}
}

namespace Lumora::Lumen
{
	GLRenderDevice::~GLRenderDevice()
	{
		GLRenderDevice::Shutdown();
	}

	void GLRenderDevice::Init(void* glfwWindowHandle, void* nativeWindowHandle)
	{
		LM_PROFILE_FUNCTION();

		m_GLFWWindowHandle = static_cast<GLFWwindow*>(glfwWindowHandle);
		glfwMakeContextCurrent(m_GLFWWindowHandle);
		int status = gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress));
		LM_CORE_ASSERT(status, "Failed to initialize OpenGL Context")

		std::string version = reinterpret_cast<const char*>(glGetString(GL_VERSION));
		std::string renderer = reinterpret_cast<const char*>(glGetString(GL_RENDERER));
		std::string vendor = reinterpret_cast<const char*>(glGetString(GL_VENDOR));
		//LM_CORE_INFO("OpenGL Version: {}, \n       Renderer: {}, \n       Vendor: {}", version, renderer, vendor);
		LM_CORE_INFO("OpenGL Version: {}", version);
		LM_CORE_INFO("Device: {}", renderer);
		LM_CORE_INFO("Vendor: {}", vendor);

		// Debug Stuff
		DEBUG_ONLY
		(
			glEnable(GL_DEBUG_OUTPUT);
			glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
			glDebugMessageCallback(OpenGLMessageCallback, nullptr);

			glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE,
				GL_DEBUG_SEVERITY_HIGH, 0, nullptr, GL_TRUE);
		)

		// Default GL State
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_LINE_SMOOTH);

		glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
	}

	void GLRenderDevice::Shutdown()
	{
		LM_PROFILE_FUNCTION();

		// Cleanup any remaining resources, with a warning
		for (auto& [handle, vb] : m_VertexBuffers)
		{
			LM_CORE_WARN("Leaked Vertex Buffer Handle: {}", handle);
			glDeleteVertexArrays(1, &vb.VAO);
			glDeleteBuffers(1, &vb.VBO);
		}
		for (auto& [handle, ib] : m_IndexBuffers)
		{
			LM_CORE_WARN("Leaked Index Buffer Handle: {}", handle);
			glDeleteBuffers(1, &ib.IBO);
		}
		for (auto& [handle, program] : m_Shaders)
		{
			LM_CORE_WARN("Leaked Shader Handle: {}", handle);
			glDeleteProgram(program);
		}
		for (auto& [handle, texture] : m_Textures)
		{
			LM_CORE_WARN("Leaked Texture Handle: {}", handle);
			glDeleteTextures(1, &texture);
		}
		for (auto& [handle, ubo] : m_UniformBuffers)
		{
			LM_CORE_WARN("Leaked Uniform Buffer Handle: {}", handle);
			glDeleteBuffers(1, &ubo.BufferID);
		}

		m_VertexBuffers.clear();
		m_IndexBuffers.clear();
		m_Shaders.clear();
		m_Textures.clear();
		m_UniformBuffers.clear();

		DEBUG_ONLY(CheckGLError(LM_IMPL_SPDLOG_SOURCE_LOC());)
	}

	void GLRenderDevice::BeginFrame()
	{
		Clear();
	}

	void GLRenderDevice::EndFrame()
	{
		LM_PROFILE_FUNCTION();

		glfwSwapBuffers(m_GLFWWindowHandle);
	}

	void GLRenderDevice::OnResize(uint32_t width, uint32_t height)
	{
		LM_PROFILE_FUNCTION();

		glViewport(0, 0, static_cast<GLsizei>(width), static_cast<GLsizei>(height));
		DEBUG_ONLY(CheckGLError(LM_IMPL_SPDLOG_SOURCE_LOC());)
	}

	void GLRenderDevice::SetClearColor(glm::vec4 color)
	{
		LM_PROFILE_FUNCTION();

		glClearColor(color.r, color.g, color.b, color.a);
		DEBUG_ONLY(CheckGLError(LM_IMPL_SPDLOG_SOURCE_LOC());)
	}

	void GLRenderDevice::Clear()
	{
		LM_PROFILE_FUNCTION();

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		DEBUG_ONLY(CheckGLError(LM_IMPL_SPDLOG_SOURCE_LOC());)
	}

	void GLRenderDevice::SetViewport(glm::vec<2, uint32_t> pos, glm::vec<2, uint32_t> size)
	{
		LM_PROFILE_FUNCTION();

		glViewport(static_cast<GLsizei>(pos.x), static_cast<GLsizei>(pos.y), static_cast<GLsizei>(size.x),
		           static_cast<GLsizei>(size.y));
		DEBUG_ONLY(CheckGLError(LM_IMPL_SPDLOG_SOURCE_LOC());)
	}

	BufferHandle GLRenderDevice::CreateVertexBuffer(const void* data, uint32_t size, const VertexLayout& layout,
	                                                bool dynamic)
	{
		LM_PROFILE_FUNCTION();

		GLVertexBuffer vb;
		vb.Layout = layout;

		glGenVertexArrays(1, &vb.VAO);
		glGenBuffers(1, &vb.VBO);

		glBindVertexArray(vb.VAO);
		glBindBuffer(GL_ARRAY_BUFFER, vb.VBO);
		glBufferData(GL_ARRAY_BUFFER, size, data, dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);

		SetupVertexAttributes(layout);

		glBindVertexArray(0);

		uint32_t id = AllocHandle();
		m_VertexBuffers[id] = vb;
		return {id};
		DEBUG_ONLY(CheckGLError(LM_IMPL_SPDLOG_SOURCE_LOC());)
	}

	void GLRenderDevice::UpdateVertexBuffer(BufferHandle buffer, const void* data, uint32_t size, uint32_t offset)
	{
		LM_PROFILE_FUNCTION();

		auto it = m_VertexBuffers.find(buffer.Id);
		if (it == m_VertexBuffers.end())
		{
			LM_CORE_ERROR("Invalid Vertex Buffer Handle: {}", buffer.Id);
			return;
		}

		glBindBuffer(GL_ARRAY_BUFFER, it->second.VBO);
		glBufferSubData(GL_ARRAY_BUFFER, offset, size, data);
		DEBUG_ONLY(CheckGLError(LM_IMPL_SPDLOG_SOURCE_LOC());)
	}

	BufferHandle GLRenderDevice::CreateIndexBuffer(const void* data, uint32_t count)
	{
		LM_PROFILE_FUNCTION();

		GLIndexBuffer ib;
		ib.Count = count;

		glGenBuffers(1, &ib.IBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ib.IBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(uint32_t), data, GL_STATIC_DRAW);

		uint32_t id = AllocHandle();
		m_IndexBuffers[id] = ib;
		return {id};
	}

	BufferHandle GLRenderDevice::CreateUniformBuffer(uint32_t size)
	{
		LM_PROFILE_FUNCTION();

		if (size == 0)
		{
			LM_CORE_ERROR("Cannot create uniform buffer with size 0");
			return {0};
		}

		GLUniformBuffer ubo;
		ubo.Size = size;

		glGenBuffers(1, &ubo.BufferID);
		glBindBuffer(GL_UNIFORM_BUFFER, ubo.BufferID);
		glBufferData(GL_UNIFORM_BUFFER, static_cast<GLsizeiptr>(size), nullptr, GL_DYNAMIC_DRAW);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);

		uint32_t id = AllocHandle();
		m_UniformBuffers[id] = ubo;

		DEBUG_ONLY(CheckGLError(LM_IMPL_SPDLOG_SOURCE_LOC());)
		return {id};
	}

	void GLRenderDevice::UpdateUniformBuffer(BufferHandle buffer, const void* data, uint32_t size, uint32_t offset)
	{
		LM_PROFILE_FUNCTION();

		auto it = m_UniformBuffers.find(buffer.Id);
		if (it == m_UniformBuffers.end())
		{
			LM_CORE_ERROR("Invalid Uniform Buffer Handle: {}", buffer.Id);
			return;
		}

		if (offset > it->second.Size || size > (it->second.Size - offset))
		{
			LM_CORE_ERROR("Uniform buffer update out of bounds. Handle: {}, Size: {}, Offset: {}, Capacity: {}",
			              buffer.Id, size, offset, it->second.Size);
			return;
		}

		glBindBuffer(GL_UNIFORM_BUFFER, it->second.BufferID);
		glBufferSubData(GL_UNIFORM_BUFFER, static_cast<GLintptr>(offset), static_cast<GLsizeiptr>(size), data);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);

		DEBUG_ONLY(CheckGLError(LM_IMPL_SPDLOG_SOURCE_LOC());)
	}

	void GLRenderDevice::DestroyBuffer(BufferHandle buffer)
	{
		LM_PROFILE_FUNCTION();

		auto vbIt = m_VertexBuffers.find(buffer.Id);
		if (vbIt != m_VertexBuffers.end())
		{
			glDeleteVertexArrays(1, &vbIt->second.VAO);
			glDeleteBuffers(1, &vbIt->second.VBO);
			m_VertexBuffers.erase(vbIt);
			return;
		}

		auto ibIt = m_IndexBuffers.find(buffer.Id);
		if (ibIt != m_IndexBuffers.end())
		{
			glDeleteBuffers(1, &ibIt->second.IBO);
			m_IndexBuffers.erase(ibIt);
			return;
		}

		auto uboIt = m_UniformBuffers.find(buffer.Id);
		if (uboIt != m_UniformBuffers.end())
		{
			glDeleteBuffers(1, &uboIt->second.BufferID);
			m_UniformBuffers.erase(uboIt);
			return;
		}

		LM_CORE_ERROR("Invalid Buffer Handle: {}", buffer.Id);
	}

	ShaderHandle GLRenderDevice::CreateShader(const char* vertexSource, const char* fragmentSource)
	{
		LM_PROFILE_FUNCTION();

		uint32_t vert = CompileShaderStage(GL_VERTEX_SHADER, vertexSource);
		uint32_t frag = CompileShaderStage(GL_FRAGMENT_SHADER, fragmentSource);

		if (vert == 0 || frag == 0)
		{
			if (vert != 0)
				glDeleteShader(vert);
			if (frag != 0)
				glDeleteShader(frag);
			return {0};
		}

		uint32_t program = LinkShaderProgram(vert, frag);

		// Shaders can be deleted after linking
		glDeleteShader(vert);
		glDeleteShader(frag);

		if (program == 0) return {0};

		uint32_t id = AllocHandle();
		m_Shaders[id] = program;
		return {id};
	}

	ShaderHandle GLRenderDevice::CreateComputeShader(const char* computeSource)
	{
		LM_PROFILE_FUNCTION();

		uint32_t cs = CompileShaderStage(GL_COMPUTE_SHADER, computeSource);
		if (cs == 0) return {0};

		uint32_t program = glCreateProgram();
		glAttachShader(program, cs);
		glLinkProgram(program);
		glDeleteShader(cs);

		GLint linked = 0;
		glGetProgramiv(program, GL_LINK_STATUS, &linked);
		if (!linked)
		{
			GLint len = 0;
			glGetProgramiv(program, GL_INFO_LOG_LENGTH, &len);
			std::string log(len, '\0');
			glGetProgramInfoLog(program, len, nullptr, log.data());
			LM_CORE_ERROR("Failed to Link Compute Shader Program: {}", log);
			glDeleteProgram(program);
			return {0};
		}

		uint32_t id = AllocHandle();
		m_Shaders[id] = program;
		return {id};
	}

	void GLRenderDevice::DestroyShader(ShaderHandle shader)
	{
		LM_PROFILE_FUNCTION();

		auto it = m_Shaders.find(shader.Id);
		if (it != m_Shaders.end())
		{
			glDeleteProgram(it->second);
			m_Shaders.erase(it);
		}
	}

	void GLRenderDevice::BindShader(ShaderHandle shader)
	{
		LM_PROFILE_FUNCTION();

		auto it = m_Shaders.find(shader.Id);
		if (it == m_Shaders.end())
		{
			LM_CORE_ERROR("Invalid Shader Handle: {}", shader.Id);
			return;
		}
		glUseProgram(it->second);
		m_BoundShader = it->second;
	}

	void GLRenderDevice::BindUniformBuffer(BufferHandle buffer, uint32_t slot)
	{
		LM_PROFILE_FUNCTION();

		auto it = m_UniformBuffers.find(buffer.Id);
		if (it == m_UniformBuffers.end())
		{
			LM_CORE_ERROR("Invalid Uniform Buffer Handle: {}", buffer.Id);
			return;
		}

		glBindBufferBase(GL_UNIFORM_BUFFER, slot, it->second.BufferID);

		DEBUG_ONLY(CheckGLError(LM_IMPL_SPDLOG_SOURCE_LOC());)
	}

	TextureHandle GLRenderDevice::CreateTexture2D(uint32_t width, uint32_t height, const void* pixelData,
	                                              uint32_t channels)
	{
		LM_PROFILE_FUNCTION();

		GLenum internalFormat = (channels == 4) ? GL_RGBA8 : GL_RGB8;
		GLenum format = (channels == 4) ? GL_RGBA : GL_RGB;

		GLuint tex = 0;
		glGenTextures(1, &tex);
		glBindTexture(GL_TEXTURE_2D, tex);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		glTexImage2D(GL_TEXTURE_2D, 0, static_cast<GLint>(internalFormat), static_cast<GLsizei>(width), static_cast<GLsizei>(height), 0,
		             format, GL_UNSIGNED_BYTE, pixelData);

		uint32_t id = AllocHandle();
		m_Textures[id] = tex;
		DEBUG_ONLY(CheckGLError(LM_IMPL_SPDLOG_SOURCE_LOC());)
		return {id};
	}

	void GLRenderDevice::BindTexture(TextureHandle texture, uint32_t slot)
	{
		LM_PROFILE_FUNCTION();

		auto it = m_Textures.find(texture.Id);
		if (it == m_Textures.end())
		{
			LM_CORE_ERROR("Invalid Texture Handle: {}", texture.Id);
			return;
		}
		glActiveTexture(GL_TEXTURE0 + slot);
		glBindTexture(GL_TEXTURE_2D, it->second);
	}

	void GLRenderDevice::DestroyTexture(TextureHandle texture)
	{
		LM_PROFILE_FUNCTION();

		auto it = m_Textures.find(texture.Id);
		if (it != m_Textures.end())
		{
			glDeleteTextures(1, &it->second);
			m_Textures.erase(it);
		}
	}

	void GLRenderDevice::DrawIndexed(BufferHandle vertexBuffer, BufferHandle indexBuffer, uint32_t indexCount)
	{
		LM_PROFILE_FUNCTION();

		auto vbIt = m_VertexBuffers.find(vertexBuffer.Id);
		auto ibIt = m_IndexBuffers.find(indexBuffer.Id);
		if (vbIt == m_VertexBuffers.end())
		{
			LM_CORE_ERROR("Invalid Vertex Buffer Handle: {}", vertexBuffer.Id);
			return;
		}

		glBindVertexArray(vbIt->second.VAO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibIt->second.IBO);
		glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indexCount), GL_UNSIGNED_INT, nullptr);
		glBindVertexArray(0);
	}

	void GLRenderDevice::DispatchCompute(ShaderHandle computeShader, uint32_t groupCountX, uint32_t groupCountY,
	                                     uint32_t groupCountZ)
	{
		BindShader(computeShader);
		glDispatchCompute(groupCountX, groupCountY, groupCountZ);
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_BUFFER_UPDATE_BARRIER_BIT);
	}

	uint32_t GLRenderDevice::CompileShaderStage(uint32_t type, const char* source)
	{
		LM_PROFILE_FUNCTION();

		GLuint shader = glCreateShader(type);
		glShaderSource(shader, 1, &source, nullptr);
		glCompileShader(shader);

		GLint compiled = 0;
		glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
		if (!compiled)
		{
			GLint len = 0;
			glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &len);
			std::string log(len, '\0');
			glGetShaderInfoLog(shader, len, nullptr, log.data());

			const char* typeStr =
				(type == GL_VERTEX_SHADER)
					? "Vertex"
					: (type == GL_FRAGMENT_SHADER)
					? "Fragment"
					: (type == GL_COMPUTE_SHADER)
					? "Compute"
					: "Unknown";

			LM_CORE_ERROR("{} shader compilation error\n{}", typeStr, log);
			glDeleteShader(shader);
			return 0;
		}

		return shader;
	}

	uint32_t GLRenderDevice::LinkShaderProgram(uint32_t vertexShader, uint32_t fragmentShader)
	{
		LM_PROFILE_FUNCTION();

		uint32_t program = glCreateProgram();
		glAttachShader(program, vertexShader);
		glAttachShader(program, fragmentShader);
		glLinkProgram(program);

		GLint linked = 0;
		glGetProgramiv(program, GL_LINK_STATUS, &linked);
		if (!linked)
		{
			GLint len = 0;
			glGetProgramiv(program, GL_INFO_LOG_LENGTH, &len);
			std::string log(len, '\0');
			glGetProgramInfoLog(program, len, nullptr, log.data());
			LM_CORE_ERROR("Failed to Link Shader Program: {}", log);
			glDeleteProgram(program);
			return 0;
		}
		return program;
	}

	void GLRenderDevice::SetupVertexAttributes(const VertexLayout& layout)
	{
		for (uint32_t i = 0; i < layout.Attributes.size(); i++)
		{
			const auto& attrib = layout.Attributes[i];

			GLenum glType = GL_FLOAT;
			GLint components = 1;
			GLboolean normalized = GL_FALSE;

			switch (attrib.Type)
			{
			case AttributeType::Float: glType = GL_FLOAT;
				components = 1;
				break;
			case AttributeType::Float2: glType = GL_FLOAT;
				components = 2;
				break;
			case AttributeType::Float3: glType = GL_FLOAT;
				components = 3;
				break;
			case AttributeType::Float4: glType = GL_FLOAT;
				components = 4;
				break;
			case AttributeType::Int: glType = GL_INT;
				components = 1;
				break;
			case AttributeType::Int2: glType = GL_INT;
				components = 2;
				break;
			case AttributeType::Int3: glType = GL_INT;
				components = 3;
				break;
			case AttributeType::Int4: glType = GL_INT;
				components = 4;
				break;
			case AttributeType::UByte4Norm: glType = GL_UNSIGNED_BYTE;
				components = 4;
				normalized = GL_TRUE;
				break;
			}

			glEnableVertexAttribArray(i);
			glVertexAttribPointer(
				i, components, glType, normalized, static_cast<GLsizei>(layout.Stride),
				const_cast<const void*>(reinterpret_cast<void*>(static_cast<size_t>(attrib.Offset)))
			);
		}
	}
}
