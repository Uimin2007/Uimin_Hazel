#pragma once

#include "hzpch.h"
#include "Uimin_Hazel/Core/Core.h"


namespace Hazel {

    // 이벤트는 메시지처럼 처리된다. 어떤 상황이 발생하면 이벤트 객체를 생성하여 전달하고,
    // 처리하는 쪽에서 소비하도록 한다. 이 방식은 동기적이어서
    // 적절한 메시지 버퍼 방식보다 전파 처리가 약간 취약할 수 있으나,
    // 현재는 충분하다.

    enum class EventType {
      None = 0, // 아무 이벤트도 발생하지 않은 기본 상태

      // 창 관련 이벤트
      WindowClose,     // 창 닫기
      WindowResize,    // 창 크기 변경
      WindowLostFocus, // 현재 창이 포커스를 잃었을 때(비활성화 됐을 때)
      WindowMoved,     // 창이 이동했을 때

      // 애플리케이션 단계 관련 이벤트
      AppTick,   // 앱 틱이 발생했을 때
      AppUpdate, // 프레임마다 발생하는 업데이트 이벤트
      AppRender, // 렌더링 단계의 렌더 이벤트

      // 키보드 관련 이벤트
      KeyPressed,
      KeyReleased,
      KeyTyped, // 문자가 입력됐을 때, 특정 플랫폼의 반복 입력도 처리함

      // 마우스 관련 이벤트
      MouseButtonPressed,
      MouseButtonReleased,
      MouseMoved, // 마우스가 이동했을 때 발생
      MouseScrolled
    };

    enum EventCategory { // 이벤트는 여러 카테고리에 동시에 속할 수 있으므로 비트플래그로 처리한다.
                         // 비트 마스크 그룹핑
      // 비트플래그 카테고리를 사용하는 이유 :

      None = 0,
      EventCategoryApplication = BIT(0), // 애플리케이션 그룹
      EventCategoryInput =
          BIT(1), // 입력 그룹(키보드와 마우스 이벤트를 포함)
      EventCategoryKeyboard = BIT(2),   // 키보드 카테고리
      EventCategoryMouse = BIT(3),      // 마우스 카테고리
      EventCategoryMouseButton = BIT(4) // 마우스 버튼 그룹
    };

    // 각 이벤트 클래스마다 반복적으로 정의해야 하는 함수들을 자동으로 생성하는 매크로
    // GetStaticType(): 인스턴스 없이 메모리에 올라있을 때 타입을 빠르게 확인하기 위한 정적 함수
    // 이 함수를 통해 타입 비교가 가능하다. 
    // GetEventType(): "다형적인 Event& 형태로 받은 실제 메모리상의 객체가 어떤 타입인지 (예: 마우스 이벤트?
    // Event&로 받기 때문에 정적으로는 알 수 없으므로 가상 함수를 통해 실제 타입을 확인한다.) 
    // GetName(): 현재 이벤트 클래스의 이름을 문자열로(enum에서 문자열로 변환하여 반환
    // 디버그용 출력 (예:로그나 창 제목에 표시하기 위해 사용)
    #define EVENT_CLASS_TYPE(type)                                                 \
      static EventType GetStaticType() { return EventType::type; }                 \
      virtual EventType GetEventType() const override { return GetStaticType(); }  \
      virtual const char *GetName() const override { return #type; }
    // #type : 매크로 인자인 enum(EventType)을 문자열("")로 변환해 반환
    // 문자열화 연산자 (ex: EVENT_CLASS_TYPE(MouseMoved) --> "MouseMoved")

    // 이벤트가 어떤 범주에 속하는지를 비트 마스크로 표현하고, 카테고리
    // 비교에 사용하는 EventCategory를 반환한다.
    #define EVENT_CLASS_CATEGORY(category)                                         \
      virtual int GetCategoryFlags() const override { return category; }

    // 이벤트 기반 클래스
    class HAZEL_API Event {
    public:
      virtual ~Event() = default; // 가상 소멸자

      bool Handled = false; // 이벤트가 처리되었는지 나타내는 플래그
      // 예: UI가 이벤트를 가로채서 처리했으면 Handled를 true로 바꾼다.
      // 그러면 그 아래의 레이어에서는 이벤트를 처리하지 않고 전파를 멈춘다.

      // 순수 가상 인터페이스 함수들 :
      virtual EventType GetEventType() const = 0; // 이벤트의 타입을 반환한다.
      virtual const char *GetName() const = 0;    // 이벤트의 이름을 반환한다.
      virtual int GetCategoryFlags() const = 0; // 이벤트의 카테고리를 반환한다.
      virtual std::string
      ToString() const { // 이벤트를 문자열로 변환하여 반환한다.
        return GetName();
      }

      bool IsInCategory(EventCategory category) { // 이벤트가 해당 카테고리에
                                                  // 속하는지 확인한다.
       return GetCategoryFlags() &
               category; // 예: IsInCategory(EventCategoryMouse) 라면 마우스 이벤트일 때
                         // true를 반환한다.
      }
    };

    // 발생한 이벤트를 받아서 이벤트 타입에 따라 적절한 함수로 전달해주는 디스패처 클래스이다.
    class EventDispatcher {
        template<typename T>
        using EventFn = std::function<bool(T&)>;
    public:
      // 처리할 이벤트를 참조로 받아서 저장한다.
      EventDispatcher(Event &event) : m_Event(event) {}

      // 중요!: 이벤트 타입과 일치하는 이벤트(T)를 처리하는 함수(F)를 받아서
      // Dispatch(이벤트 처리:콜백 함수) 형식으로 호출된다.
      template <typename T> 
      bool Dispatch(EventFn<T> func) {
        // 현재 저장된 이벤트의 타입(m_Event.GetEventType())이 주어진 이벤트 타입(T)과
        // 일치하면 true를 반환하고, 그렇지 않으면 false를 반환한다.
        if (m_Event.GetEventType() == T::GetStaticType()) {
          // 이벤트를 실제로 처리한 결과를 저장하거나, 처리 함수에 이벤트를
          // 처리 함수(func)에 전달한다.
          m_Event.Handled |= func(static_cast<T &>(m_Event));
          return true;
        }
        return false;
      }

    private:
      Event &m_Event;
    };

    // << 연산자를 오버로드하여, Event 객체를 std::ostream에 (로그로) 출력할 수 있게 하는 
    // 전역 함수
    inline std::ostream& operator<<(std::ostream& os, const Event& e) {
      return os << e.ToString();
    }

    // 최신 spdlog(fmt v9 이상)와 Event 클래스 출력을 호환하기 위한 포매터
    template <typename T>
    struct fmt::formatter<T, std::enable_if_t<std::is_base_of<Hazel::Event, T>::value, char>> {
        // 1. 포맷 지정자를 파싱(기본적으로 무시)
        constexpr auto parse(fmt::format_parse_context& ctx) { return ctx.begin(); }
        // 2. 실제 포맷 출력을 수행하는 함수
        template <typename FormatContext>
        auto format(const Hazel::Event& e, FormatContext& ctx) const {
            return fmt::format_to(ctx.out(), "{}", e.ToString()); // 실제 출력은 ToString()에 위임한다.
        }
    };

}


