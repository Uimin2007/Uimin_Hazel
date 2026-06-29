#pragma once

#include "hzpch.h" // 미리 컴파일된 헤더 포함 (자주 쓰는 헤더들)

#include "Uimin_Hazel/Core/Core.h"
#include "Uimin_Hazel/Events/Event.h" // 이벤트 처리를 위한 헤더 포함

namespace Hazel {

	// 창의 초기 속성값을 담는 기본 데이터 구조체
	struct WindowProps
	{
		std::string Title; // 창의 제목
		unsigned int Width; // 창의 너비(가로)
		unsigned int Height; // 창의 높이(세로)

		// 기본 생성자: 인자가 없으면 기본값(1280x720)으로 창이 생성됨 
		WindowProps(const std::string& title = "Hazel Engine",
			unsigned int width = 1280,
			unsigned int height = 720)
			: Title(title), Width(width), Height(height)
		{
		}
	};

	// 플랫폼에 독립적인 창 인터페이스 클래스. 각 플랫폼에서 이 클래스를 상속받아 실제 창을 구현한다.
	class HAZEL_API Window
	{
	public:
		// std::function으로 표현된, 이벤트가 발생했을 때 호출될 콜백 함수 타입 정의. (외부에서 등록하는 함수)
		using EventCallbackFn = std::function<void(Event&)>;

		// 소멸자: 파생 클래스에서 메모리를 올바르게 해제하기 위한 가상 소멸자 
		virtual ~Window() {}

		// 매 프레임마다 창의 이벤트 처리(입력, 창 갱신, 그리기 등)와 버퍼 스왑을 수행하는 함수 
		virtual void OnUpdate() = 0;

		// 창의 너비(가로 해상도)를 반환하는 순수 가상 함수 
		virtual unsigned int GetWidth() const = 0;
		// 창의 높이(세로 해상도)를 반환하는 순수 가상 함수 
		virtual unsigned int GetHeight() const = 0;

		// 창에서 이벤트가 발생했을 때 호출될 이벤트 콜백 함수를 등록하는 순수 가상 함수 
		virtual void SetEventCallback(const EventCallbackFn& callback) = 0;

		// 수직 동기화(VSync)를 활성화 또는 비활성화하는 순수 가상 함수 
		virtual void SetVSync(bool enabled) = 0;
		// 수직 동기화(VSync) 활성화 여부를 반환하는 순수 가상 함수 
		virtual bool IsVSync() const = 0;

		// WindowsInput에서 GLFW 창 포인터(GLFWwindow*)를 얻기 위해 사용. Window는 GLFW에 의존하지 않으므로
		// 따라서 void* 로 "어떤 타입의 포인터든 넘길 수 있는 범용 포인터"를 반환하도록 설계한다.
		virtual void* GetNativeWindow() const = 0;

		// 중요한 설계 포인트: 실제 창 객체(예: WindowsWindow)를 생성하여 반환한다. 
		// 실제 구현은 .cpp 파일(WindowsWindow.cpp)에 있음
		static Window* Create(const WindowProps& props = WindowProps());
	};

}
