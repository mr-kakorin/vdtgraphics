#include <vdtgraphics/renderer2d.h>

#include <vdtgraphics/index_buffer.h>
#include <vdtgraphics/renderable.h>
#include <vdtgraphics/shader.h>
#include <vdtgraphics/shader_program.h>
#include <vdtgraphics/texture.h>
#include <vdtgraphics/vertex_buffer.h>

#include <initializer_list>
#include <map>

#include <glad/glad.h>

namespace graphics
{
	Renderer2D::Renderer2D(const int width, const int height, const Settings& settings)
		: m_width(width)
		, m_height(height)
		, m_initialized(false)
		, m_clearColor(Color::Black)
		, m_shaderLibrary()
		, m_polygonStyle(PolygonStyle::fill)
		, m_fillPolygonBatch(settings.polygonBatchSize)
		, m_spriteBatch(settings.spriteBatchSize)
		, m_strokePolygonBatch(settings.polygonBatchSize)
		, m_projectionMatrix(math::mat4::identity)
		, m_viewMatrix(math::mat4::identity)
		, m_viewProjectionMatrix(math::mat4::identity)
		, m_polygonRenderable(nullptr)
		, m_spriteBatchRenderable(nullptr)
		, m_colorProgram(nullptr)
		, m_polygonProgram(nullptr)
		, m_spritebatchProgram(nullptr)
	{
	}

	void Renderer2D::init()
	{
		if (m_initialized) return;

		if (!gladLoadGL())
		{
			return;
		}

		// color
		{
			// shaders
			m_colorProgram = createProgram(ShaderLibrary::names::ColorShader);
		}
		// polygonbatch
		{
			// shaders
			m_polygonProgram = createProgram(ShaderLibrary::names::PolygonBatchShader);

			// render data
			m_polygonRenderable = std::make_unique< Renderable>();
			VertexBuffer& vb = *m_polygonRenderable->addVertexBuffer(Renderable::names::MainBuffer, 7 * 2000 * sizeof(float), BufferUsageMode::Static);
			VertexBufferLayout& layout = vb.layout;
			layout.push(VertexBufferElement("position", VertexBufferElement::Type::Float, 3));
			layout.push(VertexBufferElement("color", VertexBufferElement::Type::Float, 4));
		}
		// spritebatch
		{
			// shaders
			m_spritebatchProgram = createProgram(ShaderLibrary::names::SpriteBatchShader);

			float vertices[] = {
				/*
				 First pixel from memory is bottom-left for OpenGL.
				 1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
				 1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
				-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
				-1.0f,  1.0f, 0.0f, 0.0f, 1.0f
				*/
				 1.0f, -1.0f, 0.0f, 1.0f, 1.0f,
				 1.0f,  1.0f, 0.0f, 1.0f, 0.0f,
				-1.0f,  1.0f, 0.0f, 0.0f, 0.0f,
				-1.0f, -1.0f, 0.0f, 0.0f, 1.0f
			};

			unsigned int indices[] = {
				0, 1, 3, 1, 2, 3
			};

			m_spriteBatchRenderable = std::make_unique< Renderable>();
			VertexBuffer& vb = *m_spriteBatchRenderable->addVertexBuffer(Renderable::names::MainBuffer, sizeof(vertices), BufferUsageMode::Static);
			vb.fillData(vertices, sizeof(vertices));
			VertexBufferLayout& layout = vb.layout;
			layout.push(VertexBufferElement("position", VertexBufferElement::Type::Float, 3));
			layout.push(VertexBufferElement("coords", VertexBufferElement::Type::Float, 2));
			IndexBuffer& ib = *m_spriteBatchRenderable->addIndexBuffer(Renderable::names::MainBuffer, sizeof(indices), BufferUsageMode::Static);
			ib.fillData(indices, sizeof(indices));

			VertexBuffer& cropBuffer = *m_spriteBatchRenderable->addVertexBuffer("cropsBuffer", 4 * 2000 * sizeof(float), BufferUsageMode::Stream);
			cropBuffer.layout.push(VertexBufferElement("crop", VertexBufferElement::Type::Float, 4, true, true));
			cropBuffer.layout.startingIndex = 2;

			VertexBuffer& colorBuffer = *m_spriteBatchRenderable->addVertexBuffer("colorsBuffer", 4 * 2000 * sizeof(float), BufferUsageMode::Stream);
			colorBuffer.layout.push(VertexBufferElement("color", VertexBufferElement::Type::Float, 4, true, true));
			colorBuffer.layout.startingIndex = 3;

			VertexBuffer& transformBuffer = *m_spriteBatchRenderable->addVertexBuffer("transformsBuffer", 16 * 2000 * sizeof(float), BufferUsageMode::Stream);
			transformBuffer.layout.push(VertexBufferElement("transform", VertexBufferElement::Type::Float, 4, true, true));
			transformBuffer.layout.push(VertexBufferElement("transform", VertexBufferElement::Type::Float, 4, true, true));
			transformBuffer.layout.push(VertexBufferElement("transform", VertexBufferElement::Type::Float, 4, true, true));
			transformBuffer.layout.push(VertexBufferElement("transform", VertexBufferElement::Type::Float, 4, true, true));
			transformBuffer.layout.startingIndex = 4;

			m_spriteBatchRenderable->bind();
		}

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_DEPTH_TEST);

		m_initialized = true;
	}

	void Renderer2D::begin()
	{
		init();

		if (!m_initialized) return;

		glViewport(0, 0, m_width, m_height);
	}

	int Renderer2D::flush()
	{
		if (!m_initialized) return 0;

		int drawCalls = 0;

		glClearColor(m_clearColor.red, m_clearColor.green, m_clearColor.blue, m_clearColor.alpha);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		m_spriteBatch.flush([this, &drawCalls](Texture* const texture, const std::vector<float>& transforms, const std::vector<float>& rects, const std::vector<float>& colors) -> void
			{
				const size_t instances = transforms.size() / 16;
				if (instances != rects.size() / 4) return;

				m_spriteBatchRenderable->bind();

				VertexBuffer& cropBuffer = *m_spriteBatchRenderable->findVertexBuffer("cropsBuffer");
				cropBuffer.bind();
				cropBuffer.fillData((void*)&rects[0], rects.size() * sizeof(float));

				VertexBuffer& colorBuffer = *m_spriteBatchRenderable->findVertexBuffer("colorsBuffer");
				colorBuffer.bind();
				colorBuffer.fillData((void*)&colors[0], colors.size() * sizeof(float));

				VertexBuffer& transformBuffer = *m_spriteBatchRenderable->findVertexBuffer("transformsBuffer");
				transformBuffer.bind();
				transformBuffer.fillData((void*)&transforms[0], transforms.size() * sizeof(float));

				m_spritebatchProgram->bind();
				texture->bind(0);
				m_spritebatchProgram->set("u_texture", 0);
				m_spritebatchProgram->set("u_matrix", m_viewProjectionMatrix);

				const int primitiveType = GL_TRIANGLES;
				const int offset = 0;
				const int count = 6;
				const int numInstances = static_cast<int>(instances);
				const int indexType = GL_UNSIGNED_INT;

				++drawCalls;
				glDrawElementsInstanced(primitiveType, count, indexType, offset, numInstances);
			}
		);

		m_fillPolygonBatch.flush([this, &drawCalls](const std::vector<float>& data) -> void
			{
				m_polygonRenderable->bind();

				VertexBuffer* vertexBuffer = m_polygonRenderable->findVertexBuffer(Renderable::names::MainBuffer);
				vertexBuffer->bind();
				vertexBuffer->fillData((void*)&data[0], data.size() * sizeof(float));

				m_polygonProgram->bind();
				m_polygonProgram->set("u_matrix", m_viewProjectionMatrix);

				const int primitiveType = GL_TRIANGLES;
				const int offset = 0;
				const int count = static_cast<int>(data.size() / 7);

				++drawCalls;
				glDrawArrays(primitiveType, offset, count);
			}
		);

		m_strokePolygonBatch.flush([this, &drawCalls](const std::vector<float>& data) -> void
			{
				m_polygonRenderable->bind();

				VertexBuffer* vertexBuffer = m_polygonRenderable->findVertexBuffer(Renderable::names::MainBuffer);
				vertexBuffer->bind();
				vertexBuffer->fillData((void*)&data[0], data.size() * sizeof(float));

				m_polygonProgram->bind();
				m_polygonProgram->set("u_matrix", m_viewProjectionMatrix);

				const int primitiveType = GL_LINES;
				const int offset = 0;
				const int count = static_cast<int>(data.size() / 7);

				++drawCalls;
				glDrawArrays(primitiveType, offset, count);
			}
		);

		return drawCalls;
	}

	void Renderer2D::setClearColor(const Color& color)
	{
		m_clearColor = color;
	}

	void Renderer2D::setViewport(const int width, const int height)
	{
		m_width = width;
		m_height = height;
	}

	void Renderer2D::setProjectionMatrix(const math::matrix4& m)
	{
		m_projectionMatrix = m;
		m_viewProjectionMatrix = m_projectionMatrix * m_viewMatrix;
	}

	void Renderer2D::setViewMatrix(const math::matrix4& m)
	{
		m_viewMatrix = m;
		m_viewProjectionMatrix = m_projectionMatrix * m_viewMatrix;
	}

	void Renderer2D::setPolygonStyle(PolygonStyle style)
	{
		m_polygonStyle = style;
	}

	void Renderer2D::drawCircle(const math::vec3& position, float radius, const Color& color)
	{
		static const unsigned int s_triangles = 20; // number of triangles
		static const float s_twoPi = 2.0f * math::pi;
		static const float s_delta = s_twoPi / s_triangles;

		for (int i = 0; i < s_triangles; ++i)
		{
			const float angle = i * s_delta;
			const float nextAngle = (i + 1) * s_delta;

			if (m_polygonStyle == PolygonStyle::fill)
			{
				m_fillPolygonBatch.reserve(3);
				m_fillPolygonBatch.batch(position, color);
				m_fillPolygonBatch.batch(position + math::vec3(radius * std::sin(angle), radius * std::cos(angle), 0.f), color);
				m_fillPolygonBatch.batch(position + math::vec3(radius * std::sin(nextAngle), radius * std::cos(nextAngle), 0.f), color);
			}
			else
			{
				m_strokePolygonBatch.reserve(2);
				m_strokePolygonBatch.batch(position + math::vec3(radius * std::sin(angle), radius * std::cos(angle), 0.f), color);
				m_strokePolygonBatch.batch(position + math::vec3(radius * std::sin(nextAngle), radius * std::cos(nextAngle), 0.f), color);
			}
		}
	}

	void Renderer2D::drawLine(const math::vec3& point1, const math::vec3& point2, const Color& color)
	{
		m_strokePolygonBatch.reserve(2);
		m_strokePolygonBatch.batch(point1, color);
		m_strokePolygonBatch.batch(point2, color);
	}

	void Renderer2D::drawPolygon(const std::vector<std::pair<math::vec3, Color>>& points)
	{
		PolygonBatch& batch = m_polygonStyle == PolygonStyle::fill
			? m_fillPolygonBatch
			: m_strokePolygonBatch;

		for (const std::pair<math::vec3, Color>& pair : points)
		{
			batch.batch(pair.first, pair.second);
		}
	}

	void Renderer2D::drawRect(const math::vec3& position, float width, float height, const Color& color)
	{
		const float w = width / 2;
		const float h = height / 2;

		if (m_polygonStyle == PolygonStyle::fill)
		{
			m_fillPolygonBatch.reserve(6);
			m_fillPolygonBatch.batch(position + math::vec3(w, h, 0.f), color);
			m_fillPolygonBatch.batch(position + math::vec3(-w, h, 0.f), color);
			m_fillPolygonBatch.batch(position + math::vec3(-w, -h, 0.f), color);
			m_fillPolygonBatch.batch(position + math::vec3(-w, -h, 0.f), color);
			m_fillPolygonBatch.batch(position + math::vec3(w, -h, 0.f), color);
			m_fillPolygonBatch.batch(position + math::vec3(w, h, 0.f), color);
		}
		else
		{
			m_strokePolygonBatch.reserve(8);
			m_strokePolygonBatch.batch(position + math::vec3(w, h, 0.f), color);
			m_strokePolygonBatch.batch(position + math::vec3(-w, h, 0.f), color);
			m_strokePolygonBatch.batch(position + math::vec3(-w, h, 0.f), color);
			m_strokePolygonBatch.batch(position + math::vec3(-w, -h, 0.f), color);
			m_strokePolygonBatch.batch(position + math::vec3(-w, -h, 0.f), color);
			m_strokePolygonBatch.batch(position + math::vec3(w, -h, 0.f), color);
			m_strokePolygonBatch.batch(position + math::vec3(w, -h, 0.f), color);
			m_strokePolygonBatch.batch(position + math::vec3(w, h, 0.f), color);
		}
	}

	void Renderer2D::drawTexture(Texture* const texture, const math::mat4& matrix, const TextureRect& rect, const Color& color)
	{
		m_spriteBatch.batch(texture, matrix, rect, color);
	}

	void Renderer2D::drawTexture(Texture* const texture, const math::vec3& position, const TextureRect& rect, const Color& color)
	{
		m_spriteBatch.batch(texture, math::matrix4::translate(position), rect, color);
	}

	void Renderer2D::drawTexture(Texture* const texture, const math::vec3& position, const float rotation, const TextureRect& rect, const Color& color)
	{
		m_spriteBatch.batch(texture, math::matrix4::translate(position) * math::matrix4::rotate_z(rotation), rect, color);
	}

	void Renderer2D::drawTexture(Texture* const texture, const math::vec3& position, const math::vec3& scale, const TextureRect& rect, const Color& color)
	{
		m_spriteBatch.batch(texture, math::matrix4::scale(scale) * math::matrix4::translate(position), rect, color);
	}

	void Renderer2D::drawTexture(Texture* const texture, const math::vec3& position, const float rotation, const math::vec3& scale, const TextureRect& rect, const Color& color)
	{
		m_spriteBatch.batch(texture, math::matrix4::scale(scale) * math::matrix4::rotate_z(rotation) * math::matrix4::translate(position), rect, color);
	}

	std::unique_ptr<ShaderProgram> Renderer2D::createProgram(const std::string& name)
	{
		std::map<Shader::Type, std::string> sources;
		auto it = m_shaderLibrary.getShaders().find(name);
		if (it != m_shaderLibrary.getShaders().end()
			&& Shader::Reader::parse(it->second, sources))
		{
			Shader vs(Shader::Type::Vertex, sources.find(Shader::Type::Vertex)->second);
			Shader fs(Shader::Type::Fragment, sources.find(Shader::Type::Fragment)->second);
			return std::make_unique<ShaderProgram>(std::initializer_list<Shader*>{ &vs, & fs });
		}
		return nullptr;
	}
}