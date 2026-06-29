#include "hzpch.h"
#include "RendererAPI.h"

namespace Hazel{
	// 현재 엔진의 기본 그래픽 API를 OpenGL로 설정해둔다.
	RendererAPI::API RendererAPI::s_API = RendererAPI::API::OpenGL;
}