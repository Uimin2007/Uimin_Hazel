#pragma once

#include "Uimin_Hazel/Events/Event.h"


namespace Hazel {

    // 모든 키 이벤트 클래스의 부모 클래스
    class HAZEL_API KeyEvent : public Event {
    public:
      // 어떤 키가 눌렸는지 해당 키의 코드(KeyCode)를 반환한다.
      inline int GetKeyCode() const { return m_KeyCode; }
      // 참고: KeyCode 클래스가 따로 없으므로 단순히 int를 사용합니다!

      // 이 이벤트가 '키보드' 카테고리와 '입력' 카테고리에 동시에 속함을 알려주는
      // 매크로 (Event.h에 정의되어 있음)
      // (|는 비트 OR 연산자로, 여러 카테고리에 동시에 속한다는 의미이다.)
      EVENT_CLASS_CATEGORY(EventCategoryKeyboard | EventCategoryInput)
    protected:
      // 설명: 어떤 키가 눌렸는지 해당 키의 코드(KeyCode)를 받아서 저장한다.
      // 키 이벤트들만 이 자리를 물려받아 쓸 수 있게 하기 위해 protected로
      // 선언한다.
      KeyEvent(int keycode) : m_KeyCode(keycode) {}

      int m_KeyCode;
    };

    // 키 눌림(Press) 이벤트
    class HAZEL_API  KeyPressedEvent : public KeyEvent {
    public:
      // 키보드를 계속 누르면 키 입력이 반복해서 들어오는데, 그 반복 횟수를
      // 받기 위해 단순히 bool이 아닌 int로 받는다.
      KeyPressedEvent(int keycode, int repeatCount)
          : KeyEvent(keycode), m_RepeatCount(repeatCount) {}

      // 현재 이 키가 몇 번 반복해서 눌리고 있는지 반환한다(0이면 처음 눌림)
      inline int GetRepeatCount() const { return m_RepeatCount; }

      // 디버그 로그 창에 "KeyPressedEvent: 65 (1 repeats)" 처럼 출력하기 위한
      // 포맷 함수.
      std::string ToString() const override {
        std::stringstream ss;
        ss << "KeyPressedEvent: " << m_KeyCode << " (" << m_RepeatCount
           << " repeats)";
        return ss.str();
      }

      EVENT_CLASS_TYPE(KeyPressed)
    private:
      int m_RepeatCount;
    };

    // 키 떼기(Release) 이벤트
    class HAZEL_API KeyReleasedEvent : public KeyEvent {
    public:
      KeyReleasedEvent(int keycode) : KeyEvent(keycode) {}

      std::string ToString() const override {
        std::stringstream ss;
        ss << "KeyReleasedEvent: " << m_KeyCode;
        return ss.str();
      }

      EVENT_CLASS_TYPE(KeyReleased)
    };

    // [class HAZEL_API KeyTypedEvent] : 외부에서 '키 타이핑' 이벤트를 처리하는 클래스 (외부 모듈에서도 사용 가능)
// [public KeyEvent] : 기본 키 이벤트 속성(어떤 키가 눌렸는가?)을 상속받아 그대로 사용한다.
    class HAZEL_API KeyTypedEvent : public KeyEvent
    {
    public:

        // [설명] : 이벤트를 생성할 때 "어떤 키가 타이핑되었는가(keycode)"를 자료로 받음
        // [: KeyEvent(keycode)] : 상속받은 부모 클래스(KeyEvent)에 키 키코드를 그대로 넘겨서 초기화한다.
        KeyTypedEvent(int keycode)
            : KeyEvent(keycode) {
        }

        // [std::string ToString()] : 이 이벤트를 문자열(텍스트)로 자료를 바꿔서 반환하는 함수
        // [const override] : 내부 값을 바꾸지 않겠다는 표시(const)이며, 부모의 가상 함수를 재정의(override)한다.
        std::string ToString() const override
        {
            // 문자열을 조각조각 이어 붙여서 만들 수 있는 도구(stringstream)
            std::stringstream ss;

            // "KeyTypedEvent: 65" (65는 'A'키) 와 같은 형식으로 문자를 이어 붙여 담는다.
            // m_KeyCode는 부모(KeyEvent)에서 가지고 있는 키 코드 값을 그대로 쓴다.
            ss << "KeyTypedEvent: " << m_KeyCode;

            // 완성된 문자열을 반환함
            return ss.str();
        }

        // 매번 GetEventType이나 GetName 같은 코드를 일일이 타이핑하지 않아도
        // 단어 하나(KeyTyped)만으로 필요한 함수 코드를 자동 생성하는 편의 매크로.
        EVENT_CLASS_TYPE(KeyTyped)
    };



} 
