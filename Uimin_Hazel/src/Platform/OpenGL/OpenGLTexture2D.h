#pragma once

#include "Uimin_Hazel/Renderer/Texture.h"
#include <glad/glad.h>

namespace Hazel {

	class OpenGLTexture2D : public Texture2D
	{
	public:
		OpenGLTexture2D(uint32_t width, uint32_t height); // 크기만으로 빈 텍스처 생성
		OpenGLTexture2D(const std::string& path);         // 파일에서 이미지 로딩
		virtual ~OpenGLTexture2D();

		virtual uint32_t GetWidth() const override { return m_Width; }
		virtual uint32_t GetHeight() const override { return m_Height; }

		// CPU 데이터를 GPU 텍스처에 업로드 (SetData 인터페이스 구현)
		virtual void SetData(void* data, uint32_t size) override;

		virtual void Bind(uint32_t slot = 0) const override;

		// GPU 오브젝트 ID(m_RendererID)가 같으면 동일한 텍스처로 판단
		// Renderer2D의 텍스처 슬롯 중복 검사에 사용됨
		virtual bool operator==(const Texture& other) const override
		{
			return m_RendererID == ((OpenGLTexture2D&)other).m_RendererID;
		}
	private:
		std::string m_Path;
		uint32_t m_Width, m_Height;
		GLuint m_RendererID;
		GLenum m_InternalFormat, m_DataFormat; // SetData()에서 재사용하기 위해 포맷 정보를 멤버로 저장
	};

}
