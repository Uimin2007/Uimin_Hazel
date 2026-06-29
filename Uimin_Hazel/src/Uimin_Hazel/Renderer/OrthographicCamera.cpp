#include "hzpch.h"
#include "OrthographicCamera.h"

#include <glm/gtc/matrix_transform.hpp> // translate, rotate 등을 위한 헤더 포함


namespace Hazel {
	OrthographicCamera::OrthographicCamera(float left, float right, float bottom, float top)
		// glm::ortho로 직교 투영 행렬을 만들고(Z축 깊이는 -1부터 1까지), 뷰 행렬은 초기값(1.0f 단위 행렬)으로 설정
		: m_ProjectionMatrix(glm::ortho(left, right, bottom, top, -1.0f, 1.0f)), m_ViewMatrix(1.0f)
	{
		// 투영(Projection) * 카메라 시점(View) 행렬 곱셈으로 뷰-투영 행렬을 미리 만들어둡니다. 
		m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
	}
	void OrthographicCamera::RecalculateViewMatrix()
	{
		// 1. 카메라 위치(m_Position)만큼 단위 행렬을 이동시킴(translate)
		// 2. Z축 기준 시계 반대 방향 회전 행렬을 생성(rotate)
		// 이 둘을 곱해 카메라 객체의 이동 변환행렬(Transform)을 구합니다.
		glm::mat4 transform = glm::translate(glm::mat4(1.0f), m_Position) *
			glm::rotate(glm::mat4(1.0f), glm::radians(m_Rotation), glm::vec3(0, 0, 1));
		// 뷰 행렬은 카메라 움직임의 반대 이동(역행렬)이어야 합니다. 
		// (카메라가 우측으로 1칸 이동하는 것은, 온 세상이 좌측으로 1칸 이동하는 것과 같이 연산해야 하기 때문)
		m_ViewMatrix = glm::inverse(transform);

		// 갱신된 카메라 뷰 행렬을 적용하여 최종 행렬 변수를 업데이트 합니다.
		m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
	}

	// 입력받은 화면 경계(상하좌우)를 바탕으로 직교 투영 행렬을 다시 생성하고, 뷰 행렬과 곱하여 최종 행렬을 업데이트함
	void OrthographicCamera::SetProjection(float left, float right, float bottom, float top)
	{
		m_ProjectionMatrix = glm::ortho(left, right, bottom, top, -1.0f, 1.0f);
		m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
	}
}