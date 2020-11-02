#include <vdtgraphics/renderable.h>

#include <vector>
#include <vdtgraphics/api.h>

namespace graphics
{
	Renderable::Renderable(VertexBuffer* const vertexBuffer, IndexBuffer* const indexBuffer)
		: m_vertexBuffer(vertexBuffer)
		, m_indexBuffer(indexBuffer)
	{

	}

	Renderable::Renderable(API* const api, const Mesh& mesh)
		: m_vertexBuffer()
		, m_indexBuffer()
	{
		const std::vector<float>& vertices = mesh.getData();
		m_vertexBuffer = api->createVertexBuffer(sizeof(float) * 10, static_cast<unsigned int>(mesh.vertices.size()));
		m_vertexBuffer->update(Mesh::VertexData::BufferLayout);
		m_vertexBuffer->update(&mesh.vertices.front(), static_cast<unsigned int>(mesh.vertices.size()));
		m_indexBuffer = api->createIndexBuffer(static_cast<unsigned int>(mesh.indices.size()));
		m_indexBuffer->update(&mesh.indices.front(), static_cast<unsigned int>(mesh.indices.size()));
	}

	Renderable::~Renderable()
	{
		delete m_vertexBuffer;
		delete m_indexBuffer;
	}
}