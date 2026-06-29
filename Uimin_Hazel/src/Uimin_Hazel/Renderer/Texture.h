#pragma once

#include <string>
#include <memory>

namespace Hazel {

	class Texture
	{
	public:
		virtual ~Texture() = default;

		virtual uint32_t GetWidth() const = 0;
		virtual uint32_t GetHeight() const = 0;

		// CPU 메모리에 있는 픽셀 데이터를 GPU 텍스처에 직접 업로드
		virtual void SetData(void* data, uint32_t size) = 0;

		virtual void Bind(uint32_t slot = 0) const = 0;

		// 두 텍스처가 같은 GPU 리소스를 가리키는지 비교
		// Renderer2D에서 텍스처 슬롯 중복 등록을 막기 위해 사용
		virtual bool operator==(const Texture& other) const = 0;
	};

	class Texture2D : public Texture
	{
	public:
		// 파일 없이 크기만으로 빈 텍스처를 생성 (코드에서 픽셀 데이터를 직접 넣을 때 사용)
		static std::shared_ptr<Texture2D> Create(uint32_t width, uint32_t height);
		static std::shared_ptr<Texture2D> Create(const std::string& path);
	};

}
