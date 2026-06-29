#pragma once

#include "RenderCommand.h"
#include "OrthographicCamera.h" 
#include "Shader.h"         

// 모든 함수가 static이다. 인스턴스를 만들지 않고 Renderer2D::DrawQuad(...) 형태로 바로 호출한다. 기존 Renderer와 동일한 설계 방식이다.
// DrawQuad가 두 가지 버전으로 선언된 이유 : 2D 작업에서는 Z좌표가 필요 없는 경우가 많으므로 vec2 버전도 제공한다.내부적으로는 vec2 버전이 vec3 버전을 호출한다.

namespace Hazel {

	class Renderer
	{
	public:
		static void Init();

		// BeginScene: 프레임 렌더링을 시작하기 전에 카메라 위치, 시야각 등 '전체 기본 세팅'을 하는 함수
		static void BeginScene(OrthographicCamera& camera);
		// EndScene: 모든 객체를 다 그렸으면 한 프레임 렌더링을 마무리 짓는 함수이다.
		static void EndScene();

		// 윈도우 크기 변경 시 호출될 고수준 렌더러 함수
		static void OnWindowResize(uint32_t width, uint32_t height);

		// 랜더링 대기열에 VAO를 전달하는 함수(셰이더도 추가)
		static void Submit(const std::shared_ptr<Shader>& shader, const std::shared_ptr<VertexArray>& vertexArray, const glm::mat4& transform = glm::mat4(1.0f));

		inline static RendererAPI::API GetAPI() { return RendererAPI::GetAPI(); }

	private:
		// 이번 프레임(Scene)을 렌더링 하는 동안 사용될 데이터들을 묶어둔 구조체
		struct SceneData
		{
			glm::mat4 ViewProjectionMatrix; // 카메라에서 받아온 뷰-투영 행렬이 여기에 담긴다
		};
		static SceneData* m_SceneData; // 씬 데이터 구조체 인스턴스를 저장하는 정적 포인터
	};


}

