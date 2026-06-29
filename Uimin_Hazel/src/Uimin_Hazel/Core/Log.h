#pragma once


#include "Core.h"
#include "spdlog/spdlog.h"
#include "spdlog/fmt/ostr.h"

namespace Hazel {

	class HAZEL_API Log
	{
	public:
		static void Init();

		inline static std::shared_ptr<spdlog::logger>& GetCoreLogger() { return s_CoreLogger; }
		inline static std::shared_ptr<spdlog::logger>& GetClientLogger() { return s_ClientLogger; }
	private:
		static std::shared_ptr<spdlog::logger> s_CoreLogger;
		static std::shared_ptr<spdlog::logger> s_ClientLogger;
	};
}
// 심각도 순으로 정리한 로그 메크로(단축어)들
#define HZ_CORE_TRACE(...)  ::Hazel::Log::GetCoreLogger()->trace(__VA_ARGS__) // 
#define HZ_CORE_INFO(...)   ::Hazel::Log::GetCoreLogger()->info(__VA_ARGS__) // 정보 메세지
#define HZ_CORE_WARN(...)   ::Hazel::Log::GetCoreLogger()->warn(__VA_ARGS__) // 이건 경고 메세지
#define HZ_CORE_ERROR(...)  ::Hazel::Log::GetCoreLogger()->error(__VA_ARGS__) // 문제가 발생했을때 HZ_CORE_ERROR(...)라고 치면 에러메세지를 띄우게 함
#define HZ_CORE_FATAL(...)  ::Hazel::Log::GetCoreLogger()->fatal(__VA_ARGS__)

// 이건 클라이언트 로그 메크로들
#define HZ_TRACE(...)		::Hazel::Log::GetClientLogger()->trace(__VA_ARGS__) 
#define HZ_INFO(...)		::Hazel::Log::GetClientLogger()->info(__VA_ARGS__) 
#define HZ_WARN(...)		::Hazel::Log::GetClientLogger()->warn(__VA_ARGS__) 
#define HZ_ERROR(...)		::Hazel::Log::GetClientLogger()->error(__VA_ARGS__) 
#define HZ_FATAL(...)		::Hazel::Log::GetClientLogger()->fatal(__VA_ARGS__)
