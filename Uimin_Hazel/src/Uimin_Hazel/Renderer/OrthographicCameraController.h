#pragma once

#include "Uimin_Hazel/Renderer/OrthographicCamera.h"
#include "Uimin_Hazel/Core/Timestep.h"

#include "Uimin_Hazel/Events/ApplicationEvent.h"
#include "Uimin_Hazel/Events/MouseEvent.h"

namespace Hazel {

	// 기존 Sandbox에 하드코딩된 카메라 조작 코드를 엔진 단으로 분리하여 재사용성을 높이기 위해 컨트롤러 클래스 도입
	class OrthographicCameraController
	{
	public:
		OrthographicCameraController(float aspectRatio, bool rotation = false);

		void OnUpdate(Timestep ts);
		void OnEvent(Event& e);

		// 렌더링 시 실제 화면을 그릴 때 필요한 진짜 카메라 객체를 반환해주는 Getter
		OrthographicCamera& GetCamera() { return m_Camera; }
		const OrthographicCamera& GetCamera() const { return m_Camera; }

		// 카메라 컨트롤러의 줌 레벨을 확인하고 변경할 수 있는 접근자(Getter/Setter) 함수 추가
		float GetZoomLevel() const { return m_ZoomLevel; }
		void SetZoomLevel(float level) { m_ZoomLevel = level; }
	private:
		bool OnMouseScrolled(MouseScrolledEvent& e);
		bool OnWindowResized(WindowResizeEvent& e);
	private:
		// 카메라의 줌 상태와 화면 비율을 유지하기 위한 멤버 변수들
		float m_AspectRatio;
		float m_ZoomLevel = 1.0f; // 초기 줌 수치
		OrthographicCamera m_Camera;

		bool m_Rotation; // 회전 기능 활성화 여부

		// 카메라 위치, 회전값 및 이동/회전 속도를 관리하는 변수들
		glm::vec3 m_CameraPosition = { 0.0f, 0.0f, 0.0f };
		float m_CameraRotation = 0.0f;
		float m_CameraTranslationSpeed = 5.0f, m_CameraRotationSpeed = 180.0f;
	};

}

