#pragma once

// 모든 플랫폼의 VertexArray들의 뼈대가 됨
// VertexArray : VBO(버텍스버퍼), IBO(인덱스버퍼), 레이아웃(설명서)를 하나로 묶어서 관리하는 클래스

#include <memory>
#include <vector>
#include "Uimin_Hazel/Renderer/Buffer.h"

namespace Hazel {

	class VertexArray {
	public:
		virtual ~VertexArray() {}

		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;

		virtual void AddVertexBuffer(const std::shared_ptr<VertexBuffer>& vertexBuffer) = 0;

		virtual void SetIndexBuffer(const std::shared_ptr<IndexBuffer>& indexbuffer) = 0;

		// 보관중인 버퍼들을 나중에 꺼내 쓰기 위한 Get 함수들
		virtual const std::vector<std::shared_ptr<VertexBuffer>>& GetVertexBuffers() const = 0;
		virtual const std::shared_ptr<IndexBuffer>& GetIndexBuffer() const = 0;
		// 버퍼(Buffer::Create)와 똑같은 방식으로 런타임에 플랫폼에 맞는 객체를 찍어낸다.
		static Ref<VertexArray> Create();
	};
}

