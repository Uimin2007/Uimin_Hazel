#include "hzpch.h"
#include "OpenGLContext.h"

#include <GLFW/glfw3.h>        // glfwMakeContextCurrent, glfwGetProcAddress, glfwSwapBuffers
#include <glad/glad.h>         // gladLoadGLLoader (OpenGL 함수 포인터 로딩)

namespace Hazel {
    // 생성자 구현 : 넘어온 GLFWwindow 포인터를 멤버 변수(핸들)에 저장
    OpenGLContext::OpenGLContext(GLFWwindow* windowHandle)
        : m_WindowHandle(windowHandle)  // 이니셜라이저 리스트로 멤버 초기화
    {
        HZ_CORE_ASSERT(windowHandle, "Window handle is null!")
    }

    // Init 구현 : OpenGL 초기화
    void OpenGLContext::Init()
    {
        glfwMakeContextCurrent(m_WindowHandle); // 이 창을 "현재 OpenGL이 그림을 그릴 대상"으로 지정
        int status = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress); // GPU 드라이버에서 OpenGL 함수들의 주소를 가져옴
        HZ_CORE_ASSERT(status, "Failed to initialize Glad!");

        // GPU 정보를 로그에 출력한다.
        HZ_CORE_INFO("OpenGL Info:");
        HZ_CORE_INFO("  Vendor: {0}", (const char*)glGetString(GL_VENDOR));     // GPU 제조사
        HZ_CORE_INFO("  Renderer: {0}", (const char*)glGetString(GL_RENDERER)); // GPU 모델명
        HZ_CORE_INFO("  Version: {0}", (const char*)glGetString(GL_VERSION));   // OpenGL 버전
    }

    // SwapBuffers 구현 : 더블 버퍼링 버퍼 교환
    void OpenGLContext::SwapBuffers()
    {
        glfwSwapBuffers(m_WindowHandle);
    }
}
