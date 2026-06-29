#pragma once
// 미리 컴파일된 헤더 파일 (자주 사용하는 표준 헤더들을 미리 컴파일한다.)

#include <iostream>
#include <memory> // shared_ptr, unique_ptr 등의 스마트 포인터를 제공한다.
#include <utility> // pair, move, swap 등의 유틸리티 제공
#include <algorithm> // sort, find, min, max 등의 STL 알고리즘을 제공
#include <functional> // function, bind, lambda 등의 함수 객체 유틸리티
#include <ostream>

#include <string>
#include <sstream> // 문자열 스트림을 위한 입출력 기능도 제공하며 특히 여러 타입을 이어붙이는 데 유용한 stringstream을 제공한다.
#include <array> // 고정 크기 배열
#include <vector> 
#include <unordered_map> // unordered_map을 포함시킨다.
// unordered_map : 키-값 쌍으로 데이터를 저장하며, 조회 속도가 매우 빠른 해시 테이블 기반의 컨테이너 
// 예: unordered_map<string, int> score; -> "string 타입의 키에 int 타입의 값을 매핑하는 맵"
#include <unordered_set> // unordered_map과 유사하지만 값만 저장하는 집합 컨테이너. 중복 값을 허용하지 않아서 고유한 원소를 관리할 때 사용한다.

#include "Uimin_Hazel/Core/Log.h"

// 각 번역 단위의 .cpp 파일에서 별도로 include 없이 HZ_PROFILE_* 매크로를 사용할 수 있게 함
#include "Uimin_Hazel/Debug/Instrumentor.h"

// premake.lua에서 설정된 전처리기 정의에 의해 HZ_PLATFORM_WINDOWS 매크로가 정의되면 window.h를 포함한다.
// (HZ_PLATFORM_WINDOWS는 premake.lua에서 설정된 매크로이다.)
#ifdef HZ_PLATFORM_WINDOWS
	#include <Windows.h>
#endif
