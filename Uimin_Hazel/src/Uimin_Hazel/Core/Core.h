#pragma once

#include <memory>

#ifdef HZ_PLATFORM_WINDOWS // 윈도우일 때만 컴파일
	#if HZ_DYNAMIC_LINK  // DLL 방식일 때 (premake에서 HZ_DYNAMIC_LINK를 정의해야 활성화됨)
		#ifdef HZ_BUILD_DLL
			#define HAZEL_API __declspec(dllexport) // 엔진 빌드 중 → 함수를 수출
		#else
			#define HAZEL_API __declspec(dllimport) // 외부에서 사용 중 → 함수를 수입
		#endif
	#else //  정적 라이브러리 방식 (기본값)
		#define HAZEL_API // 아무것도 안 붙임 — 평범한 함수처럼 사용
	#endif
#else
	#error Hazel only support Windows!
#endif



// Assert 매크로: 조건이 false이면 에러 로그를 출력하고 프로그램을 중단시킨다.
#ifdef HZ_ENABLE_ASSERTS
	#define HZ_ASSERT(x, ...) { if(!(x)) { HZ_ERROR("Assertion Failed: {0}", __VA_ARGS__); __debugbreak(); } }
	#define HZ_CORE_ASSERT(x, ...) { if(!(x)) { HZ_CORE_ERROR("Assertion Failed: {0}", __VA_ARGS__); __debugbreak(); } }
#else
	#define HZ_ASSERT(x, ...)
	#define HZ_CORE_ASSERT(x, ...)
#endif

#define BIT(x) (1 << x) // 1을 왼쪽으로 x칸씩 밀어서 1, 2, 4, 8...으로 만들어

#define HZ_BIND_EVENT_FN(fn) std::bind(&fn, this, std::placeholders::_1)

namespace Hazel {

	template<typename T>
	using Scope = std::unique_ptr<T>;

	template<typename T>
	using Ref = std::shared_ptr<T>;

	// std::make_shared의 엔진 전용 이름
	// Ref<T>와 짝을 이루는 생성 함수: new 없이 shared_ptr을 안전하게 만든다
	template<typename T, typename ... Args>
	constexpr Ref<T> CreateRef(Args&& ... args)
	{
		return std::make_shared<T>(std::forward<Args>(args)...);
	}

}