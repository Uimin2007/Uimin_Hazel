#include "hzpch.h"
#include "WindowsWindow.h"

#include "Uimin_Hazel/Events/ApplicationEvent.h"
#include "Uimin_Hazel/Events/MouseEvent.h"
#include "Uimin_Hazel/Events/KeyEvent.h"

#include "Platform/OpenGL/OpenGLContext.h"


namespace Hazel {

	// GLFW가 한 번만 초기화되도록 상태를 기억하는 전역(static) 변수입니다.
	static bool s_GLFWInitialized = false;


	static void GLFWErrorCallback(int error, const char* description)
	{
		HZ_CORE_ERROR("GLFW Error ({0}): {1}", error, description);
	}


	// Window.h에 선언해두었던 팩토리 함수(Create)를 여기서 구현합니다! 
	// 만약 맥(Mac)이었다면 MacWindow를 리턴하겠지만, 지금은 윈도우 환경이므로 WindowsWindow를 리턴합니다.
	Window* Window::Create(const WindowProps& props)
	{
		return new WindowsWindow(props);
	}

	// 생성자: Init 함수를 불러서 창을 초기화합니다.
	WindowsWindow::WindowsWindow(const WindowProps& props)
	{
		Init(props);
	}

	// 소멸자: Shutdown 함수를 불러서 창을 안전하게 닫습니다.
	WindowsWindow::~WindowsWindow()
	{
		Shutdown();
	}

	// 실제로 창을 띄우는 핵심 함수, 여기서 다른 콜백 같은 것들을 미리 등록해 둔다.
	void WindowsWindow::Init(const WindowProps& props)
	{
		// 받아온 설정값을 내부 구조체에 저장합니다.
		m_Data.Title = props.Title;
		m_Data.Width = props.Width;
		m_Data.Height = props.Height;

		HZ_CORE_INFO("Creating window {0} ({1}, {2})", props.Title, props.Width, props.Height);


		// 만약 GLFW가 아직 한 번도 초기화된 적이 없다면 [5]
		if (!s_GLFWInitialized)
		{
			// GLFW를 초기화합니다. 성공하면 내부에서 필요한 준비를 마칩니다.
			int success = glfwInit();
			HZ_CORE_ASSERT(success, "Could not intialize GLFW!");
			glfwSetErrorCallback(GLFWErrorCallback);
			// 초기화 성공 여부를 저장합니다.
			s_GLFWInitialized = true;
		}

		// GLFW 함수를 써서 운영체제에 진짜 창을 만들어 달라고 요청합니다. [6]
		m_Window = glfwCreateWindow((int)props.Width, (int)props.Height, m_Data.Title.c_str(), nullptr, nullptr);

		m_Context = new OpenGLContext(m_Window); // 새 OpenGL 컨텍스트 객체 생성
		m_Context->Init(); // OpenGL 초기화 수행

		// 이 윈도우 객체 안에 우리가 만든 m_Data 구조체의 주소를 쏙 숨겨둡니다. 
		// (나중에 GLFW가 마우스/키보드 이벤트를 줄 때 이 데이터를 꺼내 쓸 수 있습니다.) [6]
		glfwSetWindowUserPointer(m_Window, &m_Data);

		// 기본적으로 수직 동기화(VSync)를 켜줍니다. [6]
		SetVSync(true);  


		// 창 크기 변경 시 발생하는 이벤트
		glfwSetWindowSizeCallback(m_Window, [](GLFWwindow* window, int width, int height)
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
			data.Width = width;
			data.Height = height;

			WindowResizeEvent event(width, height);
			data.EventCallback(event);
		});
		// 창 닫기 버튼 클릭 시 발생하는 이벤트
		glfwSetWindowCloseCallback(m_Window, [](GLFWwindow* window)
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
			WindowCloseEvent event;
			data.EventCallback(event);
		});
		// 키보드 입력 시 발생 이벤트
		glfwSetKeyCallback(m_Window, [](GLFWwindow* window, int key, int scancode, int action, int mods)
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

			switch (action)
			{
				case GLFW_PRESS:
				{
					KeyPressedEvent event(key, 0);
					data.EventCallback(event);
					break;
				}
				case GLFW_RELEASE:
				{
					KeyReleasedEvent event(key);
					data.EventCallback(event);
					break;
				}
				case GLFW_REPEAT:
				{
					KeyPressedEvent event(key, 1);
					data.EventCallback(event);
					break;
				}
			}
		});
		// 마우스 버튼 클릭 시 발생 이벤트(이벤트 발생 시 람다 함수가 실행됨)
		glfwSetMouseButtonCallback(m_Window, [](GLFWwindow* window, int button, int action, int mods)
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

			switch (action)
			{
				case GLFW_PRESS:
				{
					MouseButtonPressedEvent event(button);
					data.EventCallback(event);
					break;
				}
				case GLFW_RELEASE:
				{
					MouseButtonReleasedEvent event(button);
					data.EventCallback(event);
					break;
				}
			}
		});
		// 마우스 휠 스크롤 시 발생 이벤트
		glfwSetScrollCallback(m_Window, [](GLFWwindow* window, double xOffset, double yOffset)
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

			MouseScrolledEvent event((float)xOffset, (float)yOffset);
			data.EventCallback(event);
		});
		// 마우스 이동 시 발생 이벤트
		glfwSetCursorPosCallback(m_Window, [](GLFWwindow* window, double xPos, double yPos)
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

			MouseMovedEvent event((float)xPos, (float)yPos);
			data.EventCallback(event);
		});
	}

	// 창을 닫을 때 메모리를 해제하는 함수입니다. [5]
	void WindowsWindow::Shutdown()
	{
		// GLFW에게 창을 파괴하라고 명령합니다.
		glfwDestroyWindow(m_Window);
	}

	// 매 프레임(보통 1초에 60번)마다 호출되는 함수입니다. [6]
	void WindowsWindow::OnUpdate()
	{
		// 1. 운영체제로부터 마우스 이동, 키보드 입력, 창 닫기 등의 이벤트(메시지)를 수집해옵니다. [6]
		glfwPollEvents();

		// 2. 백버퍼에 미리 그려둔 그림을 앞버퍼(화면)와 교체(Swap)하여 우리 눈에 보여줍니다. [6]
		m_Context->SwapBuffers();
	}

	// 수직 동기화를 켜고 끄는 기능입니다. [6]
	void WindowsWindow::SetVSync(bool enabled)
	{
		if (enabled)
			glfwSwapInterval(1); // 1 = 수직 동기화 켬 (모니터 주사율에 맞춤)
		else
			glfwSwapInterval(0); // 0 = 수직 동기화 끔 (최대 프레임으로 렌더링)

		m_Data.VSync = enabled;
	}

	// 현재 수직 동기화 상태를 반환합니다. [6]
	bool WindowsWindow::IsVSync() const
	{
		return m_Data.VSync;
	}

}