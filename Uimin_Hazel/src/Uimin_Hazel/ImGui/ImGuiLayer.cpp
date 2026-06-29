#include "hzpch.h"
#include "ImGuiLayer.h"

#include "imgui.h"

// ImGui 플랫폼 및 렌더러 구현체
#include "backends/imgui_impl_glfw.h"     // GLFW 입력 처리(ImGui에 입력을 연결하는 백엔드)
#include "backends/imgui_impl_opengl3.h"  // OpenGL3로 ImGui를 렌더링하는 백엔드

#include "Uimin_Hazel/Core/Application.h"

#include <GLFW/glfw3.h>
#include <glad/glad.h>

namespace Hazel {

    ImGuiLayer::ImGuiLayer()
        : Layer("ImGuiLayer")
    {
    }

    ImGuiLayer::~ImGuiLayer()
    {
    }

    // OnAttach : 레이어가 스택에 추가될 때 호출 (ImGui 전체 초기화)
    void ImGuiLayer::OnAttach()
    {
        // ImGui 버전 체크 (헤더와 라이브러리가 맞는지 검사)
        IMGUI_CHECKVERSION();

        // ImGui 컨텍스트(Context)를 생성한다.
        ImGui::CreateContext();

        // IO 설정 (ImGui가 입력을 어떻게 처리할지 설정)
        ImGuiIO& io = ImGui::GetIO(); (void)io; // (void)io는 "이상없음을 알리는 아무 동작도 없는 코드"

        // 키보드로 ImGui 메뉴를 탐색할 수 있도록 설정 (Tab, 방향키 등)
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

        // Docking 기능 활성화
        // ImGui 창을 고정된 위치에 붙이거나(tab), 화면 가장자리에 도킹할 수 있게 하는 기능
        // 이 기능이 활성화되면 레이아웃을 자유롭게 배치할 수 있다.
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

        // Multi-Viewport 기능 활성화
        // ImGui 창을 메인 창 밖으로 떼어내서 별도의 OS 창으로 만들 수 있다.
        // 즉 ImGui 패널이 독립적으로 플로팅된다.
        io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

        // 어두운 컬러 테마 설정
        ImGui::StyleColorsDark();

        // Viewport 스타일 커스터마이징
        // OS 창으로 분리됐을 때 ImGui 창의 스타일을 맞추는 설정
        ImGuiStyle& style = ImGui::GetStyle();
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            // 창 모서리를 0으로 설정하여 OS 창 모서리와 스타일을 맞춤
            style.WindowRounding = 0.0f;
            // 배경 알파값을 1.0(완전 불투명)으로 설정하여 분리된 창의 배경이 투명해지지 않도록 함
            style.Colors[ImGuiCol_WindowBg].w = 1.0f;
        }

        // 현재 애플리케이션에서 GLFW 창 포인터를 가져온다.
        Application& app = Application::Get();
        GLFWwindow* window = static_cast<GLFWwindow*>(app.GetWindow().GetNativeWindow());

        // GLFW 백엔드 초기화
        // 두 번째 인자 true = "GLFW 콜백을 ImGui에 자동으로 설치하겠다."
        // 이 경우 OnEvent()에서 이벤트를 별도로 전달할 필요가 없다!
        ImGui_ImplGlfw_InitForOpenGL(window, true);

        // OpenGL 렌더링 백엔드 초기화
        // GLSL 4.1 버전을 사용한다는 지시어
        ImGui_ImplOpenGL3_Init("#version 410");
    }

    // OnDetach : 레이어가 스택에서 제거될 때 호출 (리소스 해제)
    void ImGuiLayer::OnDetach()
    {
        // 백엔드 종료 및 리소스 해제 후 Context를 제거한다.
        // 해제 순서가 중요: 초기화의 역순으로 해제해야 안전하다.
        ImGui_ImplOpenGL3_Shutdown(); // OpenGL 백엔드를 종료한다.
        ImGui_ImplGlfw_Shutdown();    // GLFW 콜백을 해제한다.
        ImGui::DestroyContext();      // ImGui 컨텍스트를 제거한다.
    }

    // Begin : 매 프레임마다 ImGui 렌더링 "시작"을 알린다.
    // Application::Run()에서 레이어별 OnImGuiRender() 호출 전에 실행된다.
    void ImGuiLayer::Begin()
    {
        ImGui_ImplOpenGL3_NewFrame(); // OpenGL 백엔드 초기화
        ImGui_ImplGlfw_NewFrame();    // 마우스 위치, 입력 등 GLFW 백엔드 갱신
        ImGui::NewFrame();            // ImGui 내부 상태를 초기화하여 새로운 UI 그리기 준비
    }

    // End : 매 프레임마다 ImGui 렌더링 "마무리"를 담당한다.
    // 모든 레이어의 OnImGuiRender()가 끝난 후에 호출된다.
    void ImGuiLayer::End()
    {
        ImGuiIO& io = ImGui::GetIO();
        Application& app = Application::Get();

        // 디스플레이 크기를 ImGui에 전달(주로 OnUpdate()에서 한 번 처리)
        // (float) 캐스팅은 GetWidth()가 unsigned int를 반환하기 때문이며,
        // ImVec2는 float 두 개를 받으므로 명시적인 변환이 필요하다.
        io.DisplaySize = ImVec2((float)app.GetWindow().GetWidth(), (float)app.GetWindow().GetHeight());

        // 렌더링 실행
        ImGui::Render(); // 마지막 프레임 UI 데이터를 정점(vertex) 버퍼로 변환
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData()); // 변환된 데이터를 OpenGL로 실제로 그린다.

        // Multi-Viewport 처리
        // Viewport가 활성화된 경우에만 분리된 ImGui 창들을 업데이트한다.
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            // 현재 OpenGL 컨텍스트를 (다른 창으로 전환했다가 돌아오기 위해 저장)
            GLFWwindow* backup_current_context = glfwGetCurrentContext();

            // 분리된 OS 창들의 위치/크기 업데이트
            ImGui::UpdatePlatformWindows();
            // 분리된 OS 창을 렌더링한다.
            ImGui::RenderPlatformWindowsDefault();

            // 원래 메인 창의 OpenGL 컨텍스트로 복구
            glfwMakeContextCurrent(backup_current_context);
        }
    }

    // OnImGuiRender : 프레임마다 그릴 ImGui UI를 정의하는 함수
    // 기본적으로 OnUpdate() 이후 실행되는 코드가 이 안에 들어온다.
    // 서브클래스에서 오버라이드하여 자신만의 UI를 그릴 수 있다.
    void ImGuiLayer::OnImGuiRender()
    {
        static bool show = true;       // 데모 윈도우 표시 여부
        ImGui::ShowDemoWindow(&show);  // ImGui 공식 데모 윈도우를 표시하여 테스트에 활용한다.
    }

}
