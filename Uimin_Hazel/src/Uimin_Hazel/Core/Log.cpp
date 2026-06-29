#include "hzpch.h"
#include "Log.h"
#include "spdlog/sinks/stdout_color_sinks.h" // 로그에 색을 넣기 위해 필요한 헤더

namespace Hazel {

	std::shared_ptr<spdlog::logger> Log::s_CoreLogger;
	std::shared_ptr<spdlog::logger> Log::s_ClientLogger;

	void Log::Init() { // 로거 초기화 함수(어떤 색, 순서인지 정하는 함수)
		spdlog::set_pattern("%^[%T] %n: %v%$"); // 로그가 찍히는 형식을 지정. 순서대로 현재 시간, 로거의 이름(HAZEL 혹은 APP), 코드에 적은 텍스트이다.

		s_CoreLogger = spdlog::stdout_color_mt("HAZEL"); // 실질적으로 로그를 찍으라는 코드. mt는 멀티 스레드를 쓴다는 뜻이다.
		s_CoreLogger->set_level(spdlog::level::trace); // 로그의 등급중에 가장 낮은 trace로 설정. 이렇게 하면 사소한 에러까지 전부 띄우라는 뜻이다.

		s_ClientLogger = spdlog::stdout_color_mt("APP");
		s_ClientLogger->set_level(spdlog::level::trace);
	}
}
