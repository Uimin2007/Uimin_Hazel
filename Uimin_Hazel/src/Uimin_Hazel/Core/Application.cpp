#include "hzpch.h"
#include "Application.h"

#include "Uimin_Hazel/Core/Log.h"

#include "Uimin_Hazel/Core/Input.h"
#include "Uimin_Hazel/Renderer/Renderer.h"

#include <glfw/glfw3.h>

namespace Hazel {

// BIND_EVENT_FN(~)은 std::bind(~)로 치환된다.
// bind() : #include <functional>에서 제공하는 함수 라이브러리 기능. Application 내에서 this 포인터를 고정한 채 인자 하나를 받는 함수 객체를 생성함
#define BIND_EVENT_FN(x) std::bind(&Application::x, this, std::placeholders::_1) 
//  예: x가 OnEvent일 때 : "OnEvent 함수를 'this(현재 인스턴스)'에 묶고, 첫 번째 인자로 Event& 를 받는다"

	Application* Application::s_Instance = nullptr; // 처음에 초기화는 없다.



	Application::Application() {

		s_Instance = this; // 생성자를 통해 싱글톤 주소를 인스턴스에 저장한다.

		m_Window = std::unique_ptr<Window>(Window::Create());
		// 이벤트 흐름 설명:
		// WindowsWindow의 m_Data.EventCallback에 "OnEvent 함수"를 등록하면
		// 이후 WindowsWindow.cpp 내에서 data.EventCallback(event)가 호출될 때
		// Application::OnEvent(event)가 실행된다.	
			m_Window->SetEventCallback(BIND_EVENT_FN(OnEvent));

		Renderer::Init();

		// ImGuiLayer를 생성하고 오버레이로 추가한다.
		m_ImGuiLayer = new ImGuiLayer();
		PushOverlay(m_ImGuiLayer);
	}

	Application::~Application() {

	}
	 
	// 레이어를 m_LayerStack에 있는 레이어 목록에 추가한다. 참고 : m_LayerStack은 private이므로 직접 접근이 불가능하여 이 함수를 사용함
	void Application::PushLayer(Layer* layer) {
		m_LayerStack.PushLayer(layer);


		layer->OnAttach();
	}
	void Application::PushOverlay(Layer* layer) {
		m_LayerStack.PushOverlay(layer);
		layer->OnAttach();
	}


	void Application::OnEvent(Event& e) {
		EventDispatcher dispatcher(e);
		// 이벤트 디스패처를 통해 WindowCloseEvent가 발생하면 BIND_EVENT_FN으로 연결된 OnWindowClose를 호출한다.
		dispatcher.Dispatch<WindowCloseEvent>(BIND_EVENT_FN(OnWindowClose));
		dispatcher.Dispatch<WindowResizeEvent>(BIND_EVENT_FN(OnWindowResize));

		// 이벤트를 레이어 스택의 맨 위에서부터 아래로 전파하며 처리한다.
		for (auto it = m_LayerStack.end(); it != m_LayerStack.begin();) {
			(*--it)->OnEvent(e); // 반복자를 먼저 감소(--it), 역참조하여 레이어 획득(*it), 이벤트를 전달하여 처리(->`OnEvent(e))
			if (e.Handled) { // 이벤트가 처리되었으면 더 이상 전파하지 않음
				break;
			}
		}
	}

	void Application::Run() {

		while (m_Running) {

			float time = (float)glfwGetTime();

			// 현재 시간 - 이전 프레임 시간 = 1프레임에 그리는데 걸린 시간
			Timestep timestep = time - m_LastFrameTime;

			m_LastFrameTime = time; // 현재 시간을 다음 프레임을 위해 저장한다.
			// (창이 최소화된 상태가 아닐 때) 레이어 스택의 각 레이어에서 OnUpdate()를 호출하여 업데이트한다.
			if (!m_Minimized)
			{
				for (Layer* layer : m_LayerStack)
					layer->OnUpdate(timestep);
			}

			// ImGui 렌더링 구간		
			m_ImGuiLayer->Begin(); // Begin()으로 새 프레임을 시작함
			for (Layer* layer : m_LayerStack) { // 각 레이어에서 오버라이드한 OnImGuiRender(ImGui)를 호출한다.
				layer->OnImGuiRender(); 
			}
			m_ImGuiLayer->End(); 

			m_Window->OnUpdate();
		}
	}

	// 창 닫기 이벤트를 처리한다.
	bool Application::OnWindowClose(WindowCloseEvent& e)
	{
		m_Running = false;
		return true;
	}

	// 창 크기 변경 이벤트를 처리한다. 
	bool Application::OnWindowResize(WindowResizeEvent& e)
	{
		// 창의 너비나 높이가 0이면 창이 최소화된 상태이다.
		if (e.GetWidth() == 0 || e.GetHeight() == 0)
		{
			m_Minimized = true; // 최소화 상태로 표시		
				return false;
		}
		m_Minimized = false; // 최소화 해제
		// 창 크기에 맞게 렌더러의 뷰포트(Viewport)를 갱신한다.
		Renderer::OnWindowResize(e.GetWidth(), e.GetHeight());
		return false;
	}
}
