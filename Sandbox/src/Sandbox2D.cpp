#include "Sandbox2D.h"

#include <imgui.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

Sandbox2D::Sandbox2D()
	: Layer("Sandbox2D"), m_CameraController(1280.0f / 720.0f)
{
}

void Sandbox2D::OnAttach()
{
	HZ_PROFILE_FUNCTION();

	m_CheckerboardTexture = Hazel::Texture2D::Create("assets/textures/Checkerboard.png");
	// 레이어 추가 시 한 번만 로드. 매 프레임 로드하면 성능 낭비이므로 여기서 처리.
}

void Sandbox2D::OnDetach()
{
	HZ_PROFILE_FUNCTION();
}

void Sandbox2D::OnUpdate(Hazel::Timestep ts)
{
	HZ_PROFILE_FUNCTION();

	// Update
	m_CameraController.OnUpdate(ts);
	// Renderer2D의 각 함수 내부에 HZ_PROFILE_FUNCTION()이 있으므로 여기서 구간 블록 불필요

	// Render
	Hazel::RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1 });
	Hazel::RenderCommand::Clear();

	Hazel::Renderer2D::BeginScene(m_CameraController.GetCamera());

	Hazel::Renderer2D::DrawQuad({ -1.0f, 0.0f },      { 0.8f, 0.8f },  { 0.8f, 0.2f, 0.3f, 1.0f });

	// m_SquareColor: ImGui의 ColorEdit4와 연결된 멤버변수. 하드코딩 대신 사용하면 실시간 색상 변경이 가능해짐
	Hazel::Renderer2D::DrawQuad({  0.5f, -0.5f },     { 0.5f, 0.75f }, m_SquareColor);

	// tilingFactor 10.0f: 10x10 크기 사각형에 텍스처를 10번 반복시켜 타일 패턴을 만듦
	// 1.0f로 그리면 텍스처 1장이 10x10 전체를 늘어나 흐리게 보임
	Hazel::Renderer2D::DrawQuad({  0.0f, 0.0f, -0.1f }, { 10.0f, 10.0f }, m_CheckerboardTexture, 10.0f);
	// z=-0.1이므로 색상 사각형(z=0) 뒤에 배치됨 (깊이 테스트). 크기 10x10으로 배경 전체를 덮음.

	// 같은 텍스처를 두 번 사용 → 슬롯 중복 등록 없이 기존 슬롯(1번)을 재사용함
	// tilingFactor 20.0f: 1x1 크기에 20번 반복 → 훨씬 조밀한 패턴
	Hazel::Renderer2D::DrawQuad({ -0.5f, -0.5f, 0.0f }, {  1.0f,  1.0f }, m_CheckerboardTexture, 20.0f);

	Hazel::Renderer2D::EndScene();
}

void Sandbox2D::OnImGuiRender()
{
	HZ_PROFILE_FUNCTION(); // OnImGuiRender 함수 자체의 실행 시간도 기록

	ImGui::Begin("Settings");
	ImGui::ColorEdit4("Square Color", glm::value_ptr(m_SquareColor));
	ImGui::End();
}

void Sandbox2D::OnEvent(Hazel::Event& e)
{
	m_CameraController.OnEvent(e);
}
