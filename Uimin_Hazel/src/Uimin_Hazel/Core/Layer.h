#pragma once

#include "Uimin_Hazel/Core/Core.h"
#include "Uimin_Hazel/Events/Event.h"
#include "Uimin_Hazel/Core/Timestep.h" // 참고: Timestep 헤더에서 포함

// 레이어는 업데이트 주기 관련 시간 값을 받는다.

namespace Hazel {

	class HAZEL_API Layer { // HAZEL_API는 다른 프로젝트의 레이어가 dll로 내보낼 때 사용(예: Sandbox)에서 export한다.
	public:
		Layer(const std::string& name = "Layer");

		virtual ~Layer();

		virtual void OnAttach() {} // 레이어가 스택에 추가될 때 호출됨
		virtual void OnDetach() {}	// 레이어가 스택에서 제거될 때 호출됨
		virtual void OnUpdate(Timestep ts) {}  // 매 프레임마다 호출됨
		// ImGui UI를 그리기 위한 함수.
		// 기본적으로 ImGuiLayer::OnUpdate() 함수에서 창을 실제 그린다.
		// 사용자가 이 함수를 override하여 자신만의 ImGui UI를 그릴 수 있다.
		// Application에서 적절히 Begin()과 End() 사이에 호출된다.
		virtual void OnImGuiRender() {}
		virtual void OnEvent(Event& event) {} // 이벤트가 발생했을 때 처리함

		inline const std::string& GetName() const { // 레이어 이름을 반환하는 함수(디버그용)
			return m_DebugName;
		}
	protected:
		std::string m_DebugName; // 디버그용 이름
	};
}

