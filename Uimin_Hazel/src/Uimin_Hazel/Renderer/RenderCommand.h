#pragma once
#include "RendererAPI.h"


namespace Hazel {
	// 이 클래스는 모든 함수가 static(정적)으로 되어 있어서, 객체를 생성할 필요 없이 RenderCommand::Clear() 처럼 편하게 갖다 쓴다.
	class RenderCommand
	{
	public:
		inline static void Init() {
			s_RendererAPI->Init();
		}
		inline static void SetClearColor(const glm::vec4& color) {
			s_RendererAPI->SetClearColor(color); // OpenGL클리어 색상 적용 함수를 엔진 용으로 감싸는 과정
		}
		inline static void Clear() {
			s_RendererAPI->Clear(); // 마찬가지로 클리어 함수도 함수로 한번 더 감싼다.
		}
		// count: 실제로 그릴 인덱스 개수. 기본값 0이면 IndexBuffer 전체를 사용.
		// 배치 렌더링에서는 EndScene()이 이번 프레임에 기록된 QuadIndexCount를 넘겨줌
		inline static void DrawIndexed(const Ref<VertexArray>& vertexArray, uint32_t count = 0)
		{
			s_RendererAPI->DrawIndexed(vertexArray, count);
		}
		// RendererAPI의 SetViewport를 간편하게 호출하기 위한 정적(static) 명령어 함수 추가
		inline static void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
		{
			s_RendererAPI->SetViewport(x, y, width, height);
		}
	private:
		// OpenGL 랜더러 객체를 가리키는 객체 생성(지금 RendererAPI 옵션에 OpenGL밖에 없어서)
		static RendererAPI* s_RendererAPI;
	};
}
