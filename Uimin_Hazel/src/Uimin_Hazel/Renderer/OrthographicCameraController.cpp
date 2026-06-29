#include "hzpch.h"
#include "OrthographicCameraController.h"

#include "Uimin_Hazel/Core/Input.h"
#include "Uimin_Hazel/Core/KeyCodes.h"

namespace Hazel {

	// 초기 비율을 인수로 받아, 그에 맞게 투영 카메라를 설정한다(가로세로 비율에 맞게 절두체를 설정함)
	OrthographicCameraController::OrthographicCameraController(float aspectRatio, bool rotation)
		: m_AspectRatio(aspectRatio), m_Camera(-m_AspectRatio * m_ZoomLevel, m_AspectRatio* m_ZoomLevel, -m_ZoomLevel, m_ZoomLevel), m_Rotation(rotation)
	{
	}

	void OrthographicCameraController::OnUpdate(Timestep ts)
	{
		// W/A/S/D 키 입력에 따라 카메라 위치(X, Y)를 이동한다.
		if (Input::IsKeyPressed(HZ_KEY_A))
			m_CameraPosition.x -= m_CameraTranslationSpeed * ts;
		else if (Input::IsKeyPressed(HZ_KEY_D))
			m_CameraPosition.x += m_CameraTranslationSpeed * ts;

		if (Input::IsKeyPressed(HZ_KEY_W))
			m_CameraPosition.y += m_CameraTranslationSpeed * ts;
		else if (Input::IsKeyPressed(HZ_KEY_S))
			m_CameraPosition.y -= m_CameraTranslationSpeed * ts;

		// 회전 기능이 활성화된 경우 Q/E 키 입력에 따라 카메라를 회전한다.
		if (m_Rotation)
		{
			if (Input::IsKeyPressed(HZ_KEY_Q))
				m_CameraRotation += m_CameraRotationSpeed * ts;
			if (Input::IsKeyPressed(HZ_KEY_E))
				m_CameraRotation -= m_CameraRotationSpeed * ts;

			m_Camera.SetRotation(m_CameraRotation);
		}

		m_Camera.SetPosition(m_CameraPosition);

		// 줌 레벨에 따라 이동 속도를 조정하여 줌 상태에서도 일관된 이동감을 유지하기 위함
		m_CameraTranslationSpeed = m_ZoomLevel;
	}

	void OrthographicCameraController::OnEvent(Event& e)
	{
		// [이벤트 디스패치] 이벤트 타입에 따라 적절한 처리 함수로 이벤트를 전달하여 분기 처리한다.
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<MouseScrolledEvent>(HZ_BIND_EVENT_FN(OrthographicCameraController::OnMouseScrolled));
		dispatcher.Dispatch<WindowResizeEvent>(HZ_BIND_EVENT_FN(OrthographicCameraController::OnWindowResized));
	}

	bool OrthographicCameraController::OnMouseScrolled(MouseScrolledEvent& e)
	{
		// 마우스 스크롤 양만큼 줌 레벨을 조정한다. 최소 줌 레벨은 0.25f로 제한된다.
		m_ZoomLevel -= e.GetYOffset() * 0.25f;
		m_ZoomLevel = std::max(m_ZoomLevel, 0.25f);
		// 줌 레벨에 맞게 카메라의 투영 행렬(절두체 범위)를 갱신해야 화면에 반영된다.
		m_Camera.SetProjection(-m_AspectRatio * m_ZoomLevel, m_AspectRatio * m_ZoomLevel, -m_ZoomLevel, m_ZoomLevel);
		return false;
	}

	bool OrthographicCameraController::OnWindowResized(WindowResizeEvent& e)
	{
		// 창 크기가 바뀌면 전체 화면 비율을 다시 계산하고, 변경된 비율에 맞게 카메라 투영 행렬을 갱신한다.
		m_AspectRatio = (float)e.GetWidth() / (float)e.GetHeight();
		m_Camera.SetProjection(-m_AspectRatio * m_ZoomLevel, m_AspectRatio * m_ZoomLevel, -m_ZoomLevel, m_ZoomLevel);
		return false;
	}

}

