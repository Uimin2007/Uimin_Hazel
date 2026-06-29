#pragma once

#include <glm/glm.hpp>
namespace Hazel {
	class OrthographicCamera
	{
	public:
		// 화면의 왼쪽, 오른쪽, 아래, 위 좌표 범위를 받아 직교 투영 시스템을 구성하는 생성자
		OrthographicCamera(float left, float right, float bottom, float top);

		// 줌-인/아웃이나 창 크기 변경 시 프로그램 실행 중에 투영 영역을 재설정할 수 있도록 하기 위함
		void SetProjection(float left, float right, float bottom, float top);

		// 카메라 위치 받아오기
		const glm::vec3& GetPosition() const { return m_Position; }
		// 카메라 위치를 변경하고, 변경된 위치를 바탕으로 뷰 행렬(View Matrix)을 재계산
		void SetPosition(const glm::vec3& position) { m_Position = position; RecalculateViewMatrix(); }


		// 카메라 회전 정보 받아오기
		float GetRotation() const { return m_Rotation; }
		// 카메라 회전(Z축 기준 각도)을 변경하고 뷰 행렬을 재계산합니다.
		void SetRotation(float rotation) { m_Rotation = rotation; RecalculateViewMatrix(); }


		const glm::mat4& GetProjectionMatrix() const { return m_ProjectionMatrix; } // 투영 메트릭스(시야각과 화면비율, 물체가 그려지는 한계거리 등을 담은 행렬) 받아오기
		const glm::mat4& GetViewMatrix() const { return m_ViewMatrix; } // 뷰 메트릭스(카메라가 3D 공간 어디에 있고 어디를 바라보는지에 대한 위치 및 회전(방향) 정보를 담은 행렬) 받아오기
		const glm::mat4& GetViewProjectionMatrix() const { return m_ViewProjectionMatrix; } // 앞의 두 행렬을 곱한 행렬을 미리 만들어둔걸 받아온다.
	private:
		// 위치나 회전이 바뀔 때 행렬 연산을 거쳐 최종 변환 행렬을 업데이트하는 내부 함수
		void RecalculateViewMatrix();
	private:
		glm::mat4 m_ProjectionMatrix;     // 화면 비율과 그릴 영역이 계산된 투영 행렬
		glm::mat4 m_ViewMatrix;           // 카메라의 월드상 위치와 회전 각도를 담은 뷰 행렬
		glm::mat4 m_ViewProjectionMatrix; // 투영 행렬과 뷰 행렬이 곱해져 셰이더로 최종 전달될 행렬
		glm::vec3 m_Position = { 0.0f, 0.0f, 0.0f }; // 카메라 시작 위치
		float m_Rotation = 0.0f;                     // 카메라 시작 회전 각도
	};
}

