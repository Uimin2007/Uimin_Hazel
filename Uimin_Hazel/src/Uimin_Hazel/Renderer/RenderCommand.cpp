#include "hzpch.h"
#include "RenderCommand.h"
#include "Platform/OpenGL/OpenGLRendererAPI.h"


namespace Hazel {
	// 여기서 "이번 프로젝트의 렌더링 담당 일꾼은 OpenGLRendererAPI다!" 라고 생성해서 엔진에서 쓸 API 객체에 연결
	// 만약 나중에 다이렉트X를 쓰게 되면 이 줄 하나만 new DirectXRendererAPI 로 바꾸면 게임 전체 렌더링이 다이렉트X로 완전히 교체됨
	RendererAPI* RenderCommand::s_RendererAPI = new OpenGLRendererAPI;
}
