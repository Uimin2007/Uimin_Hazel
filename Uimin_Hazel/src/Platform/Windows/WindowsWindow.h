#pragma once

#include "Uimin_Hazel/Core/Window.h" // 추상 기반 창 클래스 포함
#include "Uimin_Hazel/Renderer/GraphicsContext.h"  
#include <GLFW/glfw3.h> // 실제 창 생성 및 입력 처리를 위한 GLFW 라이브러리 포함


namespace Hazel {

	// Window 추상 클래스를 상속받아 Windows 플랫폼에서 실제 창을 구현하는 클래스. 
	class WindowsWindow : public Window
	{
	public:
		// 생성자: 창 속성(props)을 받아서 창을 초기화한다.
		WindowsWindow(const WindowProps& props);
		// 소멸자: 창을 정리하고 해제한다.
		virtual ~WindowsWindow();

		// 부모(Window)로부터 상속받은 가상 함수를 오버라이드(override) 하는 선언
		void OnUpdate() override;

		inline unsigned int GetWidth() const override { return m_Data.Width; }
		inline unsigned int GetHeight() const override { return m_Data.Height; }

		// 이벤트 발생 시 호출될 콜백 함수를 등록한다.
		inline void SetEventCallback(const EventCallbackFn& callback) override { m_Data.EventCallback = callback; }

		void SetVSync(bool enabled) override;
		bool IsVSync() const override;

		// Window.h의 window 포인터를 반환하는 순수 가상 함수를 구현한다.
		inline virtual void* GetNativeWindow() const { return m_Window; }

	private:
		// 창을 초기화하고 설정하는 내부 함수. [4]
		virtual void Init(const WindowProps& props);
		virtual void Shutdown();

	private:
		// GLFW가 관리하는 실제 창의 힙 메모리 주소를 저장하는 포인터 [3]
		GLFWwindow* m_Window;

		GraphicsContext* m_Context; // 그래픽 API 컨텍스트(렌더링 상태)

		// GLFW는 C 라이브러리이므로 C++ 객체를 직접 전달할 수 없다. 
		// 이벤트 콜백 함수와 창의 속성값 등을 묶어서 GLFW에 전달하기 위한 구조체 [4]
		struct WindowData
		{
			std::string Title;
			unsigned int Width, Height;
			bool VSync;

			EventCallbackFn EventCallback; // 이벤트 발생 시 호출할 콜백 함수
		};

		WindowData m_Data; // 구조체 형태로 창 데이터를 묶어서 관리
	};

}
