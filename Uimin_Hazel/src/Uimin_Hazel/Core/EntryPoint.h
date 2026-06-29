#pragma once

// 게임을 시작하는 스타트 버튼(sandbox의 main())을 이 EntryPoint.h에 정의해서 엔진이 더 먼저 실행하게 한다.

#ifdef HZ_PLATFORM_WINDOWS

extern Hazel::Application* Hazel::CreateApplication(); // extern키워드로 미리 게임 생성 함수 선언

int main(int argc, char** argv) { 

	Hazel::Log::Init();

	// 앱 생성 구간 측정: CreateApplication() 내부에서 걸리는 시간을 Startup.json에 기록
	HZ_PROFILE_BEGIN_SESSION("Startup", "HazelProfile-Startup.json");
	auto app = Hazel::CreateApplication();
	HZ_PROFILE_END_SESSION();

	// 앱 실행 구간 측정: app->Run() 전체 실행 시간을 Runtime.json에 기록
	HZ_PROFILE_BEGIN_SESSION("Runtime", "HazelProfile-Runtime.json");
	app->Run();
	HZ_PROFILE_END_SESSION();

	// 앱 종료 구간 측정: delete(소멸자) 실행 시간을 Shutdown.json에 기록
	HZ_PROFILE_BEGIN_SESSION("Shutdown", "HazelProfile-Shutdown.json");
	delete app;
	HZ_PROFILE_END_SESSION();
}

#endif
