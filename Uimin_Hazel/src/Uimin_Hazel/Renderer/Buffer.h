#pragma once

#include "Uimin_Hazel/Core/Core.h"  // Ref(shared_ptr), Scope(unique_ptr) 타입 정의

namespace Hazel {

	// 셰이더에서 사용하는 변수 타입들을 엔진 레벨에서 이름붙여 관리
	// OpenGL 타입 코드 대신 이 이름을 쓰면 나중에 다른 그래픽 API로 바꿔도 코드가 그대로 동작
	enum class ShaderDataType
	{
		None = 0, Float, Float2, Float3, Float4, Mat3, Mat4, Int, Int2, Int3, Int4, Bool
	};

	// 타입별 바이트 크기를 반환하는 헬퍼 함수
	static uint32_t ShaderDataTypeSize(ShaderDataType type)
	{
		switch (type)
		{
		case ShaderDataType::Float:    return 4;
		case ShaderDataType::Float2:   return 4 * 2;
		case ShaderDataType::Float3:   return 4 * 3;   // 위치 x,y,z = 12바이트
		case ShaderDataType::Float4:   return 4 * 4;   // 색상 r,g,b,a = 16바이트
		case ShaderDataType::Mat3:     return 4 * 3 * 3;
		case ShaderDataType::Mat4:     return 4 * 4 * 4;
		case ShaderDataType::Int:      return 4;
		case ShaderDataType::Int2:     return 4 * 2;
		case ShaderDataType::Int3:     return 4 * 3;
		case ShaderDataType::Int4:     return 4 * 4;
		case ShaderDataType::Bool:     return 1;
		}

		HZ_CORE_ASSERT(false, "Unknown ShaderDataType!");
		return 0;
	}

	// 버퍼 안에 들어있는 데이터 항목 하나를 설명하는 구조체
	// 예: "a_Position은 Float3 타입이고 오프셋은 0이다"
	struct BufferElement
	{
		std::string    Name;        // 셰이더 변수명 (예: "a_Position")
		ShaderDataType Type;        // 데이터 타입 (예: Float3)
		uint32_t       Size;        // 바이트 크기 (예: 12)
		size_t         Offset;      // 정점 하나 안에서 이 항목이 시작되는 위치
		bool           Normalized;  // GPU에 올릴 때 0~1 범위로 정규화할지 여부

		BufferElement() = default;

		BufferElement(ShaderDataType type, const std::string& name, bool normalized = false)
			: Name(name), Type(type), Size(ShaderDataTypeSize(type)), Offset(0), Normalized(normalized)
		{
		}

		// 이 타입이 float 몇 개로 이루어져 있는지 반환 (셰이더 attribute 설정에 사용)
		uint32_t GetComponentCount() const
		{
			switch (Type)
			{
			case ShaderDataType::Float:   return 1;
			case ShaderDataType::Float2:  return 2;
			case ShaderDataType::Float3:  return 3;
			case ShaderDataType::Float4:  return 4;
			case ShaderDataType::Mat3:    return 3 * 3;
			case ShaderDataType::Mat4:    return 4 * 4;
			case ShaderDataType::Int:     return 1;
			case ShaderDataType::Int2:    return 2;
			case ShaderDataType::Int3:    return 3;
			case ShaderDataType::Int4:    return 4;
			case ShaderDataType::Bool:    return 1;
			}

			HZ_CORE_ASSERT(false, "Unknown ShaderDataType!");
			return 0;
		}
	};

	// 정점 하나를 구성하는 모든 항목의 묶음 (배치 순서와 크기를 기술)
	// 예: Float3(위치) + Float4(색상) + Float2(UV) → Stride = 36바이트
	class BufferLayout
	{
	public:
		BufferLayout() {}

		// 중괄호 리스트로 항목을 넘기면 자동으로 Offset과 Stride를 계산해줌
		BufferLayout(const std::initializer_list<BufferElement>& elements)
			: m_Elements(elements)
		{
			CalculateOffsetsAndStride();
		}

		inline uint32_t GetStride() const { return m_Stride; }
		inline const std::vector<BufferElement>& GetElements() const { return m_Elements; }

		// for (auto& e : layout) 문법을 지원하기 위한 반복자
		std::vector<BufferElement>::iterator begin() { return m_Elements.begin(); }
		std::vector<BufferElement>::iterator end()   { return m_Elements.end();   }
		std::vector<BufferElement>::const_iterator begin() const { return m_Elements.begin(); }
		std::vector<BufferElement>::const_iterator end()   const { return m_Elements.end();   }

	private:
		// 각 항목의 Offset과 전체 Stride(정점 하나의 총 바이트 크기)를 자동 계산
		void CalculateOffsetsAndStride()
		{
			size_t offset = 0;
			m_Stride = 0;
			for (auto& element : m_Elements)
			{
				element.Offset  = offset;
				offset         += element.Size;
				m_Stride       += element.Size;
			}
		}

	private:
		std::vector<BufferElement> m_Elements;
		uint32_t                   m_Stride = 0;
	};

	class VertexBuffer
	{
	public:
		virtual ~VertexBuffer() {}

		virtual void Bind()   const = 0;
		virtual void Unbind() const = 0;

		// 배치 렌더링용: GPU 버퍼에 CPU 데이터를 복사해 넣는 함수
		// Init에서 빈 버퍼를 만들어두고, EndScene마다 이 함수로 실제 데이터를 채워 넣는다
		virtual void SetData(const void* data, uint32_t size) = 0;

		virtual const BufferLayout& GetLayout() const = 0;
		virtual void SetLayout(const BufferLayout& layout) = 0;

		// 크기만 예약된 빈 버퍼 생성 (배치용: 나중에 SetData로 데이터를 채워 넣음)
		static Ref<VertexBuffer> Create(uint32_t size);

		// 데이터와 함께 버퍼 생성 (정적 데이터용: 생성 후 내용이 바뀌지 않는 경우)
		static Ref<VertexBuffer> Create(float* vertices, uint32_t size);
	};

	// 현재 Hazel은 32비트 인덱스만 지원
	class IndexBuffer
	{
	public:
		virtual ~IndexBuffer() {}

		virtual void Bind()   const = 0;
		virtual void Unbind() const = 0;

		virtual uint32_t GetCount() const = 0;

		// count: 인덱스 개수 (바이트 크기가 아님에 주의)
		static Ref<IndexBuffer> Create(uint32_t* indices, uint32_t count);
	};

}
