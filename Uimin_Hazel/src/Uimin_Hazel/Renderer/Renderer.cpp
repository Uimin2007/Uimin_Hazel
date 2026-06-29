#include "hzpch.h"
#include "Renderer.h"

#include "Platform/OpenGL/OpenGLShader.h"
#include "Renderer2D.h"   // Renderer.cpp에서 Renderer2D::Init()을 호출해야 하므로 필요

namespace Hazel {

	// 랜더러의 정적 멤버인 m_SceneData에 메모리를 할당
	Renderer::SceneData* Renderer::m_SceneData = new Renderer::SceneData;

	void Renderer::Init()
	{
		RenderCommand::Init();
		Renderer2D::Init();   // 2D 렌더러의 GPU 리소스(버텍스 배열, 셰이더) 사전 준비
	}

	void Renderer::OnWindowResize(uint32_t width, uint32_t height)
	{
		RenderCommand::SetViewport(0, 0, width, height);
	}

	void Renderer::BeginScene(OrthographicCamera& camera)
	{
		// 사용될 카메라의 ViewProjection 행렬 값을 SceneData 전역 변수에 저장해둔다
		m_SceneData->ViewProjectionMatrix = camera.GetViewProjectionMatrix();
	}


	void Renderer::EndScene()
	{
	}

	// 3d 오브젝트 랜더링을 위해 있는 드로우콜 함수
	void Renderer::Submit(const std::shared_ptr<Shader>& shader, const std::shared_ptr<VertexArray>& vertexArray, const glm::mat4& transform)
	{
		// 셰이더 바인딩
		shader->Bind();

		// OpenGLShader로 강제 형변환하여 Uniform 업로드 호출 (추후 Material 시스템 전까지 임시 사용)
		std::dynamic_pointer_cast<OpenGLShader>(shader)->UploadUniformMat4("u_ViewProjection", m_SceneData->ViewProjectionMatrix); 
		// 위와 동일
		std::dynamic_pointer_cast<OpenGLShader>(shader)->UploadUniformMat4("u_Transform", transform);  

		// 그래픽 카드에게 그릴 대상의 버텍스 정보들을 연결시킨다.
		vertexArray->Bind();

		// 랜더커맨드에 방금 연결된 버텍스 정보(버택스 배열)을 실제로 랜더링 하라고 한다.
		RenderCommand::DrawIndexed(vertexArray);
	}
}