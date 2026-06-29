#include "hzpch.h"
#include "OpenGLRendererAPI.h"
#include <glad/glad.h>


namespace Hazel {
	// RendererAPI에서 만든 순수가상 함수들을 OpenGL버전으로 재정의함
	void OpenGLRendererAPI::Init()
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glEnable(GL_DEPTH_TEST);
		// z값이 작을수록(카메라에서 멀수록) 뒤에 그려지도록 깊이 테스트 활성화
	}

	void OpenGLRendererAPI::SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
	{
		glViewport(x, y, width, height);
	}

	void OpenGLRendererAPI::SetClearColor(const glm::vec4& color) {
		glClearColor(color.r, color.g, color.b, color.a);
		// glm::vec4(r,g,b,a) 로 받은 C++ 색상을 OpenGL 전용 함수인 glClearColor에 쪼개서 넣어줌
	}
	void OpenGLRendererAPI::Clear() {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		// 색상 버퍼(화면에 보이는 색)와 깊이 버퍼(앞뒤 레이어 순서를 가리는 정보)를 동시에 지워서 화면을 완벽한 백지로 만듦
	}
	void OpenGLRendererAPI::DrawIndexed(const Ref<VertexArray>& vertexArray, uint32_t count)
	{
		// count가 0이면 IndexBuffer에 저장된 전체 인덱스 개수를 사용
		// 배치 렌더링에서는 이번 프레임에 실제로 기록된 인덱스 수(QuadIndexCount)를 넘겨받아 그 만큼만 그림
		uint32_t indexCount = count ? count : vertexArray->GetIndexBuffer()->GetCount();
		glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, nullptr);

		// 드로우콜 후 텍스처 바인딩 해제
		// 단색/텍스처 쿼드가 슬롯 0을 공유하므로 이전 텍스처가 다음 드로우콜에 남아있지 않게 정리
		glBindTexture(GL_TEXTURE_2D, 0);
	}

}

