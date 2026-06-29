#include "hzpch.h"
#include "Buffer.h"

#include "Renderer.h"
#include "Platform/OpenGL/OpenGLBuffer.h"

namespace Hazel {

	// 배치 렌더링용: 크기만 예약된 빈 버퍼를 생성한다
	// Init()에서 최대 정점 수 × sizeof(QuadVertex) 크기로 한 번 만들어두고
	// 매 프레임 EndScene()에서 SetData()로 실제 데이터를 채워 넣는다
	Ref<VertexBuffer> VertexBuffer::Create(uint32_t size)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:   HZ_CORE_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
		case RendererAPI::API::OpenGL: return CreateRef<OpenGLVertexBuffer>(size);
		}

		HZ_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}

	// 정적 데이터용: float 배열과 크기를 받아 즉시 GPU에 올린다
	// 정점 데이터가 한 번 정해지면 바뀌지 않는 경우에 사용 (예: ExampleLayer의 삼각형)
	Ref<VertexBuffer> VertexBuffer::Create(float* vertices, uint32_t size)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:   HZ_CORE_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
		case RendererAPI::API::OpenGL: return CreateRef<OpenGLVertexBuffer>(vertices, size);
		}

		HZ_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}

	// count: 인덱스 개수 (바이트 크기가 아님)
	// 배치 렌더링에서는 Init()에서 최대치(60000개)를 미리 만들어 GPU에 올려두고
	// DrawIndexed(count)로 실제 사용한 개수만큼만 그린다
	Ref<IndexBuffer> IndexBuffer::Create(uint32_t* indices, uint32_t count)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:   HZ_CORE_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
		case RendererAPI::API::OpenGL: return CreateRef<OpenGLIndexBuffer>(indices, count);
		}

		HZ_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}
}