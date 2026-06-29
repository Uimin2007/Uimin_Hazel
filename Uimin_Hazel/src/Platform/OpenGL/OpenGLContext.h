#pragma once

// GraphicsContext를 상속 받아야 하기 때문에 가져옴
#include "Uimin_Hazel/Renderer/GraphicsContext.h"

// 전방선언
struct GLFWwindow;

namespace Hazel {

	// GraphicsContext의 OpenGL 전용 구현체, GLFW 윈도우에 대해 OpenGL 컨텍스트를 생성하고,
    //		GLAD로 OpenGL 함수 포인터를 로딩하고, 매 프레임 버퍼를 교환한다.
	class OpenGLContext : public GraphicsContext {
	public:
		// 생성자에서 GLFW가 만든 윈도우 핸들을 받아 저장한다.
		OpenGLContext(GLFWwindow* windowHandle);

		// GraphicsContext.h에서 만든 순수가상 함수들 구현
		virtual void Init() override;
		virtual void SwapBuffers() override;

	private:
		GLFWwindow* m_WindowHandle; // 윈도우의 접근권한을 주는 티켓 같은 것
	};

}

