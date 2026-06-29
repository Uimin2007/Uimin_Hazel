#include "hzpch.h"
#include "OpenGLBuffer.h"

#include <glad/glad.h>

namespace Hazel {

	// ── VertexBuffer ──────────────────────────────────────────

	// 배치 렌더링용 생성자: 크기만 받아 빈 버퍼를 GPU에 예약한다
	// 데이터는 매 프레임 EndScene()에서 SetData()로 채워 넣는다
	OpenGLVertexBuffer::OpenGLVertexBuffer(uint32_t size)
	{
		HZ_PROFILE_FUNCTION();

		glCreateBuffers(1, &m_RendererID);
		glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);

		// nullptr: 지금은 데이터를 넣지 않고 공간만 예약
		// GL_DYNAMIC_DRAW: 드라이버에게 "이 버퍼는 자주 바뀔 것"이라고 알려줌 → 최적화
		glBufferData(GL_ARRAY_BUFFER, size, nullptr, GL_DYNAMIC_DRAW);
	}

	// 정적 데이터용 생성자: 배열 데이터를 즉시 GPU에 올린다
	OpenGLVertexBuffer::OpenGLVertexBuffer(float* vertices, uint32_t size)
	{
		HZ_PROFILE_FUNCTION();

		glCreateBuffers(1, &m_RendererID);
		glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);

		// GL_STATIC_DRAW: 드라이버에게 "이 버퍼는 한 번 올리면 안 바뀜"이라고 알려줌
		glBufferData(GL_ARRAY_BUFFER, size, vertices, GL_STATIC_DRAW);
	}

	OpenGLVertexBuffer::~OpenGLVertexBuffer()
	{
		HZ_PROFILE_FUNCTION();
		glDeleteBuffers(1, &m_RendererID);
	}

	void OpenGLVertexBuffer::Bind() const
	{
		HZ_PROFILE_FUNCTION();
		glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
	}

	void OpenGLVertexBuffer::Unbind() const
	{
		HZ_PROFILE_FUNCTION();
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	// CPU에 모인 정점 데이터를 GPU 버퍼에 복사해 넣는다
	// glBufferData와 달리 버퍼를 재할당하지 않고 기존 공간에 덮어씀 → 매 프레임 호출해도 비용이 낮다
	void OpenGLVertexBuffer::SetData(const void* data, uint32_t size)
	{
		glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);

		// 오프셋 0번부터 size 바이트만큼 data를 GPU 버퍼에 복사
		// Init()에서 예약한 버퍼의 앞부분을 이번 프레임 데이터로 덮어씀
		glBufferSubData(GL_ARRAY_BUFFER, 0, size, data);
	}


	// ── IndexBuffer ───────────────────────────────────────────

	// count: 인덱스 개수 (sizeof로 계산한 바이트 크기가 아님)
	OpenGLIndexBuffer::OpenGLIndexBuffer(uint32_t* indices, uint32_t count)
		: m_Count(count)
	{
		HZ_PROFILE_FUNCTION();

		glCreateBuffers(1, &m_RendererID);

		// GL_ELEMENT_ARRAY_BUFFER: 인덱스 데이터 전용 슬롯
		// (위치 데이터용 GL_ARRAY_BUFFER와 별도로 관리됨)
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RendererID);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(uint32_t), indices, GL_STATIC_DRAW);
		// 인덱스 패턴(0,1,2,2,3,0 반복)은 매 프레임 바뀌지 않으므로 GL_STATIC_DRAW 사용
	}

	OpenGLIndexBuffer::~OpenGLIndexBuffer()
	{
		HZ_PROFILE_FUNCTION();
		glDeleteBuffers(1, &m_RendererID);
	}

	void OpenGLIndexBuffer::Bind() const
	{
		HZ_PROFILE_FUNCTION();
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RendererID);
	}

	void OpenGLIndexBuffer::Unbind() const
	{
		HZ_PROFILE_FUNCTION();
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}

}