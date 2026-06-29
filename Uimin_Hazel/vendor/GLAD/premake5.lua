project "Glad"
    kind "StaticLib"
    language "C"
    staticruntime "on"
    
    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

    files
    {
        "include/glad/glad.h",
        "include/KHR/khrplatform.h",
        "src/glad.c"
    }

	includedirs
	{
		"include"
	}
    
    filter "system:windows"
        systemversion "latest"
            -- 디버그/릴리즈별 설정 추가
    filter "configurations:Debug"
        runtime "Debug"        -- 디버그용 런타임 사용
        symbols "on"           -- 디버그 심볼 생성

    filter "configurations:Release"
        runtime "Release"      -- 릴리즈용 런타임 사용
        optimize "on"          -- 최적화 켜기