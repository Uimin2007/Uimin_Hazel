#include "hzpch.h"
#include "OpenGLVertexArray.h"
#include <glad/glad.h>

namespace Hazel {

	// 만들어진 C++ 데이터 타입을 OpenGL 코드로 변환해주는 역할
	static GLenum ShaderDataTypeToOpenGLBaseType(ShaderDataType type) { 
		switch (type)
		{
		case Hazel::ShaderDataType::Float:    return GL_FLOAT;
		case Hazel::ShaderDataType::Float2:   return GL_FLOAT;
		case Hazel::ShaderDataType::Float3:   return GL_FLOAT;
		case Hazel::ShaderDataType::Float4:   return GL_FLOAT;
		case Hazel::ShaderDataType::Mat3:     return GL_FLOAT;
		case Hazel::ShaderDataType::Mat4:     return GL_FLOAT;
		case Hazel::ShaderDataType::Int:      return GL_INT;
		case Hazel::ShaderDataType::Int2:     return GL_INT;
		case Hazel::ShaderDataType::Int3:     return GL_INT;
		case Hazel::ShaderDataType::Int4:     return GL_INT;
		case Hazel::ShaderDataType::Bool:     return GL_BOOL;
		}
		HZ_CORE_ASSERT(false, "Unknown ShaderDataType!");
		return 0;
	}

	OpenGLVertexArray::OpenGLVertexArray() {
		// 객체 생성 시 즉시 GPU에 VAO 공간을 하나 만듭니다.
		glCreateVertexArrays(1, &m_RendererID);
	}
	OpenGLVertexArray::~OpenGLVertexArray() {
		glDeleteVertexArrays(1, &m_RendererID);
	}
	void OpenGLVertexArray::Bind() const {
		glBindVertexArray(m_RendererID);
	}
	void OpenGLVertexArray::Unbind() const {
		glBindVertexArray(0);
	}
	void OpenGLVertexArray::AddVertexBuffer(const std::shared_ptr<VertexBuffer>& vertexBuffer) {
		// [왜 이 방식인가?]: VBO를 추가할 때 자동으로 설명서(Layout)를 읽어서 세팅까지 끝내버리기 위함입니다.
		HZ_CORE_ASSERT(vertexBuffer->GetLayout().GetElements().size(), "Vertex Buffer has no layout!");
		glBindVertexArray(m_RendererID); // 1. 나(VAO)를 장착하고
		vertexBuffer->Bind();            // 2. 추가할 VBO를 장착함
		// 3. VBO에 들어있는 설명서를 읽어서 렌더링 통로(location)를 자동으로 만듦
		uint32_t index = 0;
		const auto& layout = vertexBuffer->GetLayout(); // 설명서(레이아웃) 가져오기
		for (const auto& element : layout) // 레이아웃의 엘리먼트들을 순서대로 하나씩 꺼내서 아래 작업을 반복
		{
			glEnableVertexAttribArray(index); // index 칸의 스위치를 켜라
			glVertexAttribPointer(index,
				element.GetComponentCount(), // 데이터 몇개를 한 묶음으로 읽을지 정보를 가져옴.예: 숫자 3개(X, Y, Z)를 연속으로 묶어서 읽어라
				ShaderDataTypeToOpenGLBaseType(element.Type), // 숫자 데이터들이 무슨 타입의 데이터인지 예: 소수점(GL_FLOAT) 
				element.Normalized ? GL_TRUE : GL_FALSE,
				layout.GetStride(), // 버텍스 데이터 하나를 읽으면, 그 다음 버텍스 데이터를 읽기 전에 몇 데이터를 건너뛰어서 읽어야 하는지
				(const void*)element.Offset); // 배열의 첫 시작이 어딘지
			index++;
		}

		m_VertexBuffers.push_back(vertexBuffer); // 보관함에 저장
	}

	void OpenGLVertexArray::SetIndexBuffer(const std::shared_ptr<IndexBuffer>& indexBuffer) {
		// [왜 이 방식인가?]: VAO에 IBO를 꽂아두면, 나중에 VAO만 Bind해도 IBO까지 자동으로 따라오기 때문입니다.
		glBindVertexArray(m_RendererID);
		indexBuffer->Bind();
		m_IndexBuffer = indexBuffer;
	}
}