#pragma once

#include "Uimin_Hazel/Core/Layer.h"

#include "Uimin_Hazel/Events/ApplicationEvent.h"
#include "Uimin_Hazel/Events/KeyEvent.h"
#include "Uimin_Hazel/Events/MouseEvent.h"

namespace Hazel {

	class HAZEL_API ImGuiLayer : public Layer
	{
	public:
		ImGuiLayer();
		~ImGuiLayer();

		// override : Layer 기반 클래스에서 상속받은 가상 함수를 재정의하는 선언
		virtual void OnAttach() override;   // ImGui 초기화
		virtual void OnDetach() override;   // ImGui 정리
		virtual void OnImGuiRender() override; // 매 프레임 ImGui UI를 그린다.
		void Begin(); // 매 프레임마다 ImGui 렌더링 시작 (NewFrame 호출)
		void End();   // 매 프레임마다 ImGui 렌더링 마무리(Render + Viewport 처리)

	private:
		float m_Time = 0.0f; // UI 업데이트 간격을 계산하기 위한 시간 추적 변수	
};
}
