#pragma once
#include <glm/glm.hpp>
#include "VertexArray.h"

// 

namespace Hazel {
	class RendererAPI
	{
	public:
		enum class API
		{
			None = 0, OpenGL = 1
		};
	public:
		// 무조건 구현해야 하는 함수들
		virtual void Init() = 0;
		virtual void SetClearColor(const glm::vec4& color) = 0; // 화면을 덮을 바탕색 결정
		virtual void Clear() = 0; // 그 색으로 화면을 덮어서 지워버리는 함수
		// 화면에 그릴 영역(Viewport)을 설정하는 순수 가상 함수
		virtual void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) = 0;
		// count: 실제로 그릴 인덱스 개수. 0이면 IndexBuffer 전체를 사용.
		// 배치 렌더링에서는 이번 프레임에 쌓인 인덱스 수만 그려야 하므로 count를 명시적으로 전달함
		virtual void DrawIndexed(const Ref<VertexArray>& vertexArray, uint32_t count = 0) = 0;
		inline static API GetAPI() { return s_API; }
	private:
		static API s_API;
	};
}

