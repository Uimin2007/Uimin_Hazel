#include "hzpch.h"
#include "WindowsInput.h"
#include "Uimin_Hazel/Core/Application.h"
#include <GLFW/glfw3.h>

namespace Hazel {
	Input* Input::s_Instance = new WindowsInput(); // 싱글톤 인스턴스

    // 현재 창에서 GLFW에게 "이 키가 눌렸는가?" 를 물어봄
    bool WindowsInput::IsKeyPressedImpl(int keycode) {
        auto window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
        // Application::Get()               현재 실행 중인 Application 인스턴스를 반환
        // .GetWindow()                     현재 실행 중인 Window 인스턴스를 반환
        // .GetNativeWindow()               내부의 실제 GLFW 창 포인터를 반환(void*)
        // static_cast<GLFWwindow*>(...)    void* 를 GLFWwindow* 로 캐스팅

        auto state = glfwGetKey(window, keycode); // glfw(플랫폼)에게 해당 키가 눌렸는지 상태를 state에 저장
        return state == GLFW_PRESS || state == GLFW_REPEAT; // 키가 눌렸거나 반복 입력인 경우 반환
    }
    // 마우스 버튼 입력 상태를 확인하는 함수로, 키 입력 방식과 동일하게 동작한다.
    bool WindowsInput::IsMouseButtonPressedImpl(int button) {
        auto window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
        auto state = glfwGetMouseButton(window, button);
        return state == GLFW_PRESS;
    }
    std::pair<float, float> WindowsInput::GetMousePositionImpl() {
        auto window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        return { (float)xpos, (float)ypos };
    }
    float WindowsInput::GetMouseXImpl() {
        auto [x, y] = GetMousePositionImpl();  // C++17 구조체 바인딩
        return x;
    }
    float WindowsInput::GetMouseYImpl() {
        auto [x, y] = GetMousePositionImpl();
        return y;
    }
}
