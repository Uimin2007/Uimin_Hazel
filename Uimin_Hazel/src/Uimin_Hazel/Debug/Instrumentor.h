#pragma once

#include <string>
#include <chrono>
#include <algorithm>
#include <fstream>
#include <thread>

namespace Hazel {

	// 측정 결과 하나를 담는 구조체
	// InstrumentationTimer가 측정을 마치면 이 구조체를 채워서 Instrumentor에 넘긴다
	struct ProfileResult
	{
		std::string Name;     // 측정 구간 이름 (함수명 등)
		long long   Start;    // 측정 시작 시각 (마이크로초)
		long long   End;      // 측정 종료 시각 (마이크로초)
		uint32_t    ThreadID; // 어느 스레드에서 실행됐는지
	};

	// 현재 진행 중인 프로파일링 세션 정보
	// 지금은 이름만 가지고 있지만, 나중에 세션별 추가 정보를 담을 수 있도록 구조체로 분리
	struct InstrumentationSession
	{
		std::string Name;
	};

	// 측정 결과를 JSON 파일에 기록하는 중앙 관리자 클래스 (싱글톤)
	// 프로그램 전체에서 Instrumentor::Get()으로 접근한다
	class Instrumentor
	{
	private:
		InstrumentationSession* m_CurrentSession; // 현재 세션 포인터 (세션이 없으면 nullptr)
		std::ofstream           m_OutputStream;   // JSON 파일에 쓰기 위한 출력 스트림
		int                     m_ProfileCount;   // 기록된 항목 수 (JSON 배열 콤마 처리에 사용)
	public:
		Instrumentor()
			: m_CurrentSession(nullptr), m_ProfileCount(0)
		{
		}

		// 세션 시작: JSON 파일을 열고 헤더를 쓴다
		void BeginSession(const std::string& name, const std::string& filepath = "results.json")
		{
			m_OutputStream.open(filepath);
			WriteHeader();
			m_CurrentSession = new InstrumentationSession{ name };
		}

		// 세션 종료: JSON 파일을 닫고 메모리를 정리한다
		void EndSession()
		{
			WriteFooter();
			m_OutputStream.close();
			delete m_CurrentSession;
			m_CurrentSession = nullptr;
			m_ProfileCount = 0; // 다음 세션을 위해 카운터 초기화
		}

		// 측정 결과 하나를 Chrome Tracing 형식의 JSON으로 파일에 기록
		void WriteProfile(const ProfileResult& result)
		{
			if (m_ProfileCount++ > 0)
				m_OutputStream << ","; // 두 번째 항목부터 콤마를 붙여 유효한 JSON 배열 유지

			std::string name = result.Name;
			std::replace(name.begin(), name.end(), '"', '\''); // 따옴표가 JSON 구조를 깨지 않도록 치환

			m_OutputStream << "{";
			m_OutputStream << "\"cat\":\"function\",";
			m_OutputStream << "\"dur\":" << (result.End - result.Start) << ','; // 실행 시간(마이크로초)
			m_OutputStream << "\"name\":\"" << name << "\",";
			m_OutputStream << "\"ph\":\"X\",";                                  // X = 시작+종료가 합쳐진 Complete Event 타입
			m_OutputStream << "\"pid\":0,";                                     // 단일 프로세스이므로 0 고정
			m_OutputStream << "\"tid\":" << result.ThreadID << ",";             // 스레드 ID
			m_OutputStream << "\"ts\":" << result.Start;                        // 시작 시각
			m_OutputStream << "}";

			m_OutputStream.flush(); // 즉시 디스크에 기록 (크래시 시 데이터 유실 방지)
		}

		// Chrome Tracing JSON 포맷 시작 부분
		void WriteHeader()
		{
			m_OutputStream << "{\"otherData\": {},\"traceEvents\":[";
			m_OutputStream.flush();
		}

		// Chrome Tracing JSON 포맷 끝 부분
		void WriteFooter()
		{
			m_OutputStream << "]}";
			m_OutputStream.flush();
		}

		// 싱글톤 접근: 처음 호출 시 인스턴스를 생성하고 이후 동일한 인스턴스를 반환
		static Instrumentor& Get()
		{
			static Instrumentor instance;
			return instance;
		}
	};

	// 스코프 기반 자동 시간 측정 클래스 (RAII 패턴)
	// 객체가 생성될 때 타이머 시작, 스코프를 벗어나 소멸될 때 자동으로 기록
	class InstrumentationTimer
	{
	public:
		// 생성자: 타이머 시작 — 객체가 만들어지는 순간 시간을 기록
		InstrumentationTimer(const char* name)
			: m_Name(name), m_Stopped(false)
		{
			m_StartTimepoint = std::chrono::high_resolution_clock::now();
		}

		// 소멸자: 스코프를 벗어나면 자동으로 Stop() 호출 → 결과가 JSON 파일에 기록됨
		~InstrumentationTimer()
		{
			if (!m_Stopped)
				Stop();
		}

		void Stop()
		{
			auto endTimepoint = std::chrono::high_resolution_clock::now();

			// 시작/끝 시각을 마이크로초 단위 정수로 변환
			long long start = std::chrono::time_point_cast<std::chrono::microseconds>(m_StartTimepoint).time_since_epoch().count();
			long long end   = std::chrono::time_point_cast<std::chrono::microseconds>(endTimepoint).time_since_epoch().count();

			// std::hash로 스레드 ID를 숫자로 변환
			uint32_t threadID = std::hash<std::thread::id>{}(std::this_thread::get_id());

			// 측정 결과를 Instrumentor에 전달 → JSON 파일에 기록
			Instrumentor::Get().WriteProfile({ m_Name, start, end, threadID });

			m_Stopped = true;
		}
	private:
		const char* m_Name;
		std::chrono::time_point<std::chrono::high_resolution_clock> m_StartTimepoint;
		bool m_Stopped;
	};

}

// HZ_PROFILE 1: 프로파일링 활성화 / 0: 모든 관련 코드가 컴파일에서 완전히 제거됨
// 0: 프로파일링 완전 비활성화 → 모든 HZ_PROFILE_* 매크로가 빈 문자열로 치환되어 성능 오버헤드 없음
// 1: 프로파일링 활성화 → 함수 실행 시간이 JSON 파일에 기록됨
// 배치 렌더링 구현 후 실제 성능을 확인하기 위해 일단 0으로 설정
#define HZ_PROFILE 0
#if HZ_PROFILE
	// 세션 시작: 파일 열기 + JSON 헤더 작성
	#define HZ_PROFILE_BEGIN_SESSION(name, filepath) ::Hazel::Instrumentor::Get().BeginSession(name, filepath)
	// 세션 종료: JSON 푸터 작성 + 파일 닫기
	#define HZ_PROFILE_END_SESSION()                 ::Hazel::Instrumentor::Get().EndSession()
	// 이름 지정 구간 측정: 해당 스코프가 끝날 때까지의 시간을 기록
	// timer##__LINE__ 은 줄 번호를 변수명에 붙여 같은 함수 내 변수명 충돌을 방지
	#define HZ_PROFILE_SCOPE(name)                   ::Hazel::InstrumentationTimer timer##__LINE__(name);
	// 현재 함수 전체 측정: __FUNCSIG__는 컴파일러가 함수 전체 시그니처를 문자열로 제공
	#define HZ_PROFILE_FUNCTION()                    HZ_PROFILE_SCOPE(__FUNCSIG__)
#else
	// 프로파일링 비활성화 시 모든 매크로를 빈 문자열로 치환
	#define HZ_PROFILE_BEGIN_SESSION(name, filepath)
	#define HZ_PROFILE_END_SESSION()
	#define HZ_PROFILE_SCOPE(name)
	#define HZ_PROFILE_FUNCTION()
#endif
