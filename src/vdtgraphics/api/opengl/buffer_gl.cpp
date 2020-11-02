#include <vdtgraphics/api/opengl/buffer_gl.h>

#include <vdtgraphics/api/opengl/opengl.h>

namespace graphics
{
	/*
	VertexBufferGL::VertexBufferGL(const std::size_t size)
		: VertexBuffer(size)
	{
		glGenBuffers(1, &m_id);
		bind();
		glBufferData(GL_ARRAY_BUFFER, size, nullptr, GL_DYNAMIC_DRAW);

		registerLayout();

		unbind();
	}

	VertexBufferGL::VertexBufferGL(const void* data, const unsigned int count, const std::size_t size, const BufferLayout& layout)
		: VertexBuffer(data, count, size, layout)
	{
		glGenBuffers(1, &m_id);
		bind();
		glBufferData(GL_ARRAY_BUFFER, size, data, GL_DYNAMIC_DRAW);

		registerLayout();

		unbind();
	}

	VertexBufferGL::~VertexBufferGL()
	{
		glDeleteBuffers(1, &m_id);
	}

	void VertexBufferGL::bind()
	{
		glBindBuffer(GL_ARRAY_BUFFER, m_id);
	}

	void VertexBufferGL::unbind()
	{
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	void VertexBufferGL::set(const void* data, const std::size_t size)
	{
		bind();
		glBufferSubData(GL_ARRAY_BUFFER, 0, size, data);
		unbind();
	}

	void VertexBufferGL::set(const BufferLayout& layout)
	{
		VertexBuffer::set(layout);

		bind();
		registerLayout();
		unbind();
	}

	void VertexBufferGL::registerLayout()
	{
		unsigned int elementIndex = 0;
		std::size_t elementSize = 0;
		for (const BufferElement& element : m_layout)
		{
			GLenum type = GL_FLOAT;
			switch (element.type)
			{
			case BufferElement::Type::Int: type = GL_INT; break;
			case BufferElement::Type::Bool: type = GL_BOOL; break;
			case BufferElement::Type::Float:
			default:
				type = GL_FLOAT; break;
			}

			glVertexAttribPointer(
				elementIndex,
				element.count,
				type,
				element.normalized ? GL_TRUE : GL_FALSE,
				static_cast<GLsizei>(m_layout.getStride()),
				(void*)(elementSize)
			);
			elementSize += element.size;
			glEnableVertexAttribArray(elementIndex++);
		}
	}
	*/
	
	IndexBufferGL::IndexBufferGL(const std::size_t size)
		: IndexBuffer(size)
		, m_id()
	{
		glGenBuffers(1, &m_id);

		bind();

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_id);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER
			, size * sizeof(unsigned int)
			, nullptr
			, GL_DYNAMIC_DRAW);

		unbind();
	}

	IndexBufferGL::IndexBufferGL(const unsigned int* indices, const std::size_t size, const BufferType type)
		: IndexBuffer(indices, size, type)
		, m_id()
	{
		glGenBuffers(1, &m_id);

		bind();

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_id);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER
			, size 
			, indices
			, GL_STATIC_DRAW);

		unbind();
	}

	IndexBufferGL::~IndexBufferGL()
	{
		glDeleteBuffers(1, &m_id);
	}

	void IndexBufferGL::bind()
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_id);
	}

	void IndexBufferGL::unbind()
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}
	
	void IndexBufferGL::set(const unsigned int* indices, const std::size_t size)
	{
		if (m_type == BufferType::Dynamic)
		{
			bind();
			glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, size * sizeof(unsigned int), indices);
			m_size = size;
			unbind();
		}
	}
	
	VertexBufferGL::VertexBufferGL(const std::size_t vertexSize, const unsigned int vertices)
		: VertexBuffer(vertexSize, vertices)
	{
		glGenBuffers(1, &m_id);
		bind();
		glBufferData(GL_ARRAY_BUFFER, vertexSize * vertices, nullptr, getUsageModeGL(m_usage));
		unbind();
	}

	VertexBufferGL::VertexBufferGL(const std::size_t vertexSize, const unsigned int vertices, const UsageMode usage, const PrimitiveType primitiveType)
		: VertexBuffer(vertexSize, vertices, usage, primitiveType)
	{
		glGenBuffers(1, &m_id);
		bind();
		glBufferData(GL_ARRAY_BUFFER, vertexSize * vertices, nullptr, getUsageModeGL(m_usage));
		unbind();
	}

	VertexBufferGL::~VertexBufferGL()
	{
		glDeleteBuffers(1, &m_id);
	}

	void VertexBufferGL::bind()
	{
		glBindBuffer(GL_ARRAY_BUFFER, m_id);

		activateLayout();
	}

	void VertexBufferGL::unbind()
	{
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	void VertexBufferGL::update(const void* data, const unsigned int vertices)
	{
		update(data, vertices, 0);
	}

	void VertexBufferGL::update(const void* data, const unsigned int vertices, const unsigned int offset)
	{
		bind();
		if (vertices >= m_vertices)
		{
			glBufferData(GL_ARRAY_BUFFER, m_vertexSize * vertices, nullptr, getUsageModeGL(m_usage));
			m_vertices = vertices;
		}
		glBufferSubData(GL_ARRAY_BUFFER, m_vertexSize * offset, m_vertexSize * vertices, data);
		unbind();
	}

	void VertexBufferGL::update(const BufferLayout& layout)
	{
		VertexBuffer::update(layout);		
	}

	void VertexBufferGL::activateLayout()
	{
		unsigned int elementIndex = 0;
		std::size_t elementSize = 0;
		for (const BufferElement& element : m_layout)
		{
			GLenum type = GL_FLOAT;
			switch (element.type)
			{
			case BufferElement::Type::Int: type = GL_INT; break;
			case BufferElement::Type::Bool: type = GL_BOOL; break;
			case BufferElement::Type::Float:
			default:
				type = GL_FLOAT; break;
			}

			glVertexAttribPointer(
				elementIndex,
				element.count,
				type,
				element.normalized ? GL_TRUE : GL_FALSE,
				static_cast<GLsizei>(m_layout.getStride()),
				(void*)(elementSize)
			);
			elementSize += element.size;
			glEnableVertexAttribArray(elementIndex++);
		}
	}

	GLenum VertexBufferGL::getUsageModeGL(const VertexBuffer::UsageMode usage)
	{
		switch (usage)
		{
		case UsageMode::Dynamic: return GL_DYNAMIC_DRAW;
		case UsageMode::Static: return GL_STATIC_DRAW;
		case UsageMode::Stream:
		default: return GL_STREAM_DRAW;
		}
	}
}