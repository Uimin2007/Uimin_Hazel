#pragma once
#include "Uimin_Hazel/Renderer/RendererAPI.h"

namespace Hazel {
	class OpenGLRendererAPI : public RendererAPI
	{
	public:
		virtual void Init() override;
		virtual void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) override;
		virtual void SetClearColor(const glm::vec4& color) override;
		virtual void Clear() override;
		// count: 실제로 그릴 인덱스 개수 (0이면 IndexBuffer 전체 사용)
		virtual void DrawIndexed(const Ref<VertexArray>& vertexArray, uint32_t count = 0) override;
	};
}


