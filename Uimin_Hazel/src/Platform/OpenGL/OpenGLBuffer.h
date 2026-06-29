#pragma once

#include "Uimin_Hazel/Renderer/Buffer.h"

namespace Hazel {

	class OpenGLVertexBuffer : public VertexBuffer
	{
	public:
		// 배치 렌더링용 생성자: 크기만 받아 빈 버퍼를 GPU에 예약한다
		// GL_DYNAMIC_DRAW: 매 프레임 데이터가 바뀔 것임을 드라이버에 알려 최적화 힌트를 줌
		OpenGLVertexBuffer(uint32_t size);

		// 정적 데이터용 생성자: float 배열을 받아 즉시 GPU에 올린다
		// GL_STATIC_DRAW: 데이터가 한 번 올라가면 바뀌지 않는다는 힌트
		OpenGLVertexBuffer(float* vertices, uint32_t size);

		virtual ~OpenGLVertexBuffer();

		virtual void Bind()   const override;
		virtual void Unbind() const override;

		// 배치 렌더링의 핵심: CPU에 모인 정점 데이터를 GPU 버퍼에 복사해 넣는다
		// glBufferSubData를 사용해 버퍼 전체를 재할당하지 않고 앞부분만 덮어씀
		virtual void SetData(const void* data, uint32_t size) override;

		virtual const BufferLayout& GetLayout() const override { return m_Layout; }
		virtual void SetLayout(const BufferLayout& layout) override { m_Layout = layout; }

	private:
		uint32_t     m_RendererID; // OpenGL이 부여하는 버퍼 고유 ID
		BufferLayout m_Layout;     // 정점 데이터 레이아웃 설명서
	};

	class OpenGLIndexBuffer : public IndexBuffer
	{
	public:
		// count: 인덱스 개수 (바이트 크기가 아님)
		OpenGLIndexBuffer(uint32_t* indices, uint32_t count);
		virtual ~OpenGLIndexBuffer();

		virtual void Bind()   const override;
		virtual void Unbind() const override;

		virtual uint32_t GetCount() const { return m_Count; }

	private:
		uint32_t m_RendererID;
		uint32_t m_Count;      // 전체 인덱스 개수 (DrawIndexed에서 사용)
	};

}