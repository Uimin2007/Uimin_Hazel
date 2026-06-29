#pragma once

#include "Core.h"

#include "Uimin_Hazel/Core/LayerStack.h"
#include "Uimin_Hazel/Events/Event.h"
#include "Uimin_Hazel/Events/ApplicationEvent.h"

#include "Uimin_Hazel/ImGui/ImGuiLayer.h"
#include "Uimin_Hazel/Renderer/Shader.h"
#include "Uimin_Hazel/Renderer/Buffer.h"
#include "Uimin_Hazel/Renderer/VertexArray.h"

#include "Uimin_Hazel/Renderer/OrthographicCamera.h"

#include "Window.h"

namespace Hazel {

	class HAZEL_API Application
	{
	public:
		Application();
		virtual ~Application();

		void Run();

		void OnEvent(Event& e); // WindowsWindow.cpp에서 이벤트 콜백으로 등록되어, 이벤트 발생 시 호출된다.

		// 레이어를 레이어 스택에 추가하는 함수
		void PushLayer(Layer* layer);
		void PushOverlay(Layer* layer);

		// Get 함수로 현재 인스턴스의 주소를 반환한다.
		inline static Application& Get() { return *s_Instance; }

		// 현재 인스턴스의 주소(s_Instance)를 통해 창 객체를 반환한다.
		inline Window& GetWindow() { return *m_Window; }
	private:
		bool OnWindowClose(WindowCloseEvent& e); // WindowCloseEvent를 수신하면 처리하는 함수 (정의는 Application.cpp에)
		bool OnWindowResize(WindowResizeEvent& e); // 창 크기 변경 이벤트를 처리하는 함수

		std::unique_ptr<Window> m_Window; // 창 객체 포인터
		ImGuiLayer* m_ImGuiLayer;
		bool m_Running = true;			// 애플리케이션 실행 여부 플래그
		bool m_Minimized = false;		// 현재 창이 최소화 상태인지 나타내는 플래그
		
		LayerStack m_LayerStack; // 레이어들의 스택 컨테이너
		float m_LastFrameTime = 0.0f; // 이전 프레임의 시간을 저장하는 변수

	private:
		// 이 Application 객체(인스턴스)의 주소를 저장한다.(이 엔진에서는 Application 인스턴스는 하나만 존재한다.)
		static Application* s_Instance;
	};

	// 사용자 측(SandboxApp.cpp)에서 정의하도록 선언
	Application* CreateApplication();
}
