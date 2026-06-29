#pragma once
namespace Hazel {

    // GraphicsContext: 그래픽 API의 "컨텍스트"를 추상화한 인터페이스
    //
    // 역할 : 어떤 그래픽 API(OpenGL, Vulkan, DirectX 등)를 쓰든
    //         "초기화(Init)"와 "버퍼 교환(SwapBuffers)" 기능은 반드시
    //         필요하므로, 이 두 함수를 순수 가상 함수로 강제한다.
    //
    // WindowsWindow가 특정 그래픽 API에 직접 의존하지 않고
    // 이 인터페이스 포인터만 들고 있으면, 나중에 OpenGL → Vulkan
    // 교체 시 WindowsWindow 코드를 전혀 수정하지 않아도 됨.
    // (= 전략 패턴 / 의존성 역전 원칙)

    class GraphicsContext {
    public:
        // init: 그래픽 API를 초기화
        virtual void Init() = 0;

        // swapbuffer
        virtual void SwapBuffers() = 0;
    };
}