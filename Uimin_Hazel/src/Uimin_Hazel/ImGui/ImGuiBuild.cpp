#include "hzpch.h"

#define IMGUI_IMPL_OPENGL_LOADER_GLAD
// ★ 핵심: ImGui가 공식으로 제공하는 OpenGL3/GLFW 백엔드의 .cpp 소스를 직접 #include 한다.
// 이렇게 하면 별도의 빌드 설정 없이 우리 프로젝트 컴파일 시 함께 빌드된다.
// 이전에는 우리가 직접 만든 ImGuiOpenGLRenderer.cpp가 이 역할을 했지만,
// 이제 ImGui 공식 구현체로 대체한다.
#include "backends/imgui_impl_opengl3.cpp"  // OpenGL3 렌더링 백엔드
#include "backends/imgui_impl_glfw.cpp"     // GLFW 입력/윈도우 백엔드