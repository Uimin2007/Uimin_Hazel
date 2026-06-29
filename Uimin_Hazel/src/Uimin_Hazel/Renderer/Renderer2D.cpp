#include "hzpch.h"
#include "Renderer2D.h"

#include "VertexArray.h"
#include "Shader.h"
#include "RenderCommand.h"

#include <glm/gtc/matrix_transform.hpp>

namespace Hazel {

	// ── 데이터 구조체 ───────────────────────────────────────────

	// 정점 하나가 가지는 데이터 묶음
	// DrawQuad 호출 시 CPU 배열에 이 구조체 4개(쿼드 꼭짓점 4개)가 기록됨
	struct QuadVertex
	{
		glm::vec3 Position; // 이 정점의 3D 위치
		glm::vec4 Color;    // 이 정점의 색상 (uniform 대신 정점마다 직접 저장)
		glm::vec2 TexCoord; // 이 정점의 UV 좌표 (텍스처 어느 부분과 매핑되는지)
		float TexIndex;         // 이 정점이 어떤 텍스처 슬롯을 사용할지 (0 = 흰색 더미 텍스처)
		float TilingFactor;     // 텍스처 UV를 몇 배로 반복할지
	};

	// Renderer2D 전체에서 사용하는 데이터를 한 곳에 모아둔 구조체
	struct Renderer2DData
	{
		// 한 번의 드로우콜(배치)에 담을 수 있는 최대 쿼드 수
		const uint32_t MaxQuads    = 10000;
		const uint32_t MaxVertices = MaxQuads * 4; // 쿼드당 꼭짓점 4개 → 40000개
		const uint32_t MaxIndices  = MaxQuads * 6; // 쿼드당 인덱스 6개(삼각형 2개) → 60000개
		static const uint32_t MaxTextureSlots = 32; // GPU 텍스처 슬롯 최대 개수

		Ref<VertexArray>  QuadVertexArray;  // VAO: 버퍼 레이아웃 정보를 GPU에 등록한 오브젝트
		Ref<VertexBuffer> QuadVertexBuffer; // VBO: 매 프레임 정점 데이터를 올릴 동적 버퍼
		Ref<Shader>       TextureShader;    // 단색/텍스처 모두 처리하는 셰이더
		Ref<Texture2D>    WhiteTexture;     // 단색 쿼드용 1×1 흰색 더미 텍스처

		uint32_t    QuadIndexCount = 0;           // 이번 프레임에 기록된 인덱스 총 개수
		QuadVertex* QuadVertexBufferBase = nullptr; // CPU 배열의 시작 주소 (고정, 안 움직임)
		QuadVertex* QuadVertexBufferPtr  = nullptr; // 현재 기록 위치 (DrawQuad 호출마다 전진)

		std::array<Ref<Texture2D>, MaxTextureSlots> TextureSlots; // 슬롯 0~31에 바인딩된 텍스처 목록
		uint32_t TextureSlotIndex = 1;  // 다음에 새 텍스처를 넣을 슬롯 번호. 0번은 흰색 텍스처로 고정
	};

	// 포인터 대신 값 타입으로 선언 → new/delete 없이 프로그램 종료 시 자동 소멸
	static Renderer2DData s_Data;


	// ── 초기화 / 종료 ───────────────────────────────────────────

	// 랜더러를 사용하기 전에 필요한 버퍼나 텍스쳐 등을 미리 생성하거나 등록해주는 함수이다. 프로그램이 시작할 때 한번만 실행된다.
	void Renderer2D::Init() 
	{
		HZ_PROFILE_FUNCTION();

		// 버텍스 배열 (VAO) 생성: 이후 추가되는 VBO의 레이아웃 정보를 GPU에 기억시킴
		s_Data.QuadVertexArray = VertexArray::Create();

		// 배치용 동적 VBO 생성: 데이터 없이 크기만 예약
		// 매 프레임 EndScene()에서 SetData()로 이 버퍼를 채워 넣음
		s_Data.QuadVertexBuffer = VertexBuffer::Create(s_Data.MaxVertices * sizeof(QuadVertex));
		s_Data.QuadVertexBuffer->SetLayout({
			{ ShaderDataType::Float3, "a_Position" }, // 위치 (12바이트)
			{ ShaderDataType::Float4, "a_Color"    }, // 색상 (16바이트) ← 이번 커밋의 핵심 추가
			{ ShaderDataType::Float2, "a_TexCoord" },  // UV  (8바이트)
			{ ShaderDataType::Float,  "a_TexIndex" }, // 텍스처 슬롯 번호
			{ ShaderDataType::Float,  "a_TilingFactor"} // 타일링 배율
		});
		s_Data.QuadVertexArray->AddVertexBuffer(s_Data.QuadVertexBuffer);

		// CPU 정점 배열 할당: DrawQuad가 이곳에 데이터를 기록함
		// EndScene()에서 GPU로 전송한 후에도 메모리는 유지되어 다음 프레임에 재사용됨
		s_Data.QuadVertexBufferBase = new QuadVertex[s_Data.MaxVertices];

		// 인덱스 버퍼 생성: 쿼드를 삼각형 2개로 그리는 패턴(0,1,2, 2,3,0)을 10000개분 미리 만들어둠
		// 이 패턴은 매 프레임 동일하므로 Init에서 한 번만 생성해 GPU에 올려두고 재사용
		uint32_t* quadIndices = new uint32_t[s_Data.MaxIndices];
		uint32_t offset = 0;
		for (uint32_t i = 0; i < s_Data.MaxIndices; i += 6)
		{
			// 쿼드 하나를 삼각형 2개로 구성하는 인덱스 패턴
			// 꼭짓점 배치: 3(좌상)─2(우상)
			//              │  /   │
			//              0(좌하)─1(우하)
			// 삼각형1: 0→1→2 / 삼각형2: 2→3→0
			quadIndices[i + 0] = offset + 0;
			quadIndices[i + 1] = offset + 1;
			quadIndices[i + 2] = offset + 2;

			quadIndices[i + 3] = offset + 2;
			quadIndices[i + 4] = offset + 3;
			quadIndices[i + 5] = offset + 0;

			// 다음 쿼드의 꼭짓점은 4칸 뒤에서 시작 (쿼드당 꼭짓점 4개)
			offset += 4;
		}
		Ref<IndexBuffer> quadIB = IndexBuffer::Create(quadIndices, s_Data.MaxIndices);
		s_Data.QuadVertexArray->SetIndexBuffer(quadIB);
		delete[] quadIndices; // GPU에 올렸으니 CPU 임시 배열 해제

		// 1×1 흰색 텍스처: 단색 쿼드를 그릴 때 텍스처 슬롯에 꽂는 더미
		// 셰이더에서 "텍스처 픽셀 × 색상"으로 계산하므로 흰색(1,1,1,1)을 곱하면 색상이 그대로 나옴
		s_Data.WhiteTexture = Texture2D::Create(1, 1);
		uint32_t whiteTextureData = 0xffffffff; // RGBA 각 채널이 모두 255(흰색)
		s_Data.WhiteTexture->SetData(&whiteTextureData, sizeof(uint32_t));

		int32_t samplers[s_Data.MaxTextureSlots];
		for (uint32_t i = 0; i < s_Data.MaxTextureSlots; i++) {
			samplers[i] = i;
		}

		// 셰이더 로드 및 텍스처 슬롯 고정
		s_Data.TextureShader = Shader::Create("assets/shaders/Texture.glsl");
		s_Data.TextureShader->Bind();
		s_Data.TextureShader->SetIntArray("u_Textures", samplers, s_Data.MaxTextureSlots); // 32개의 GPU 텍스쳐 슬롯 전부 설정

		s_Data.TextureSlots[0] = s_Data.WhiteTexture; // 슬롯 0번을 흰색 더미 텍스쳐로 설정
	}

	void Renderer2D::Shutdown()
	{
		HZ_PROFILE_FUNCTION();

		// Init()에서 new[]로 할당한 CPU 정점 배열 해제
		delete[] s_Data.QuadVertexBufferBase;
	}


	// ── 씬 시작 / 종료 ─────────────────────────────────────────

	void Renderer2D::BeginScene(const OrthographicCamera& camera)
	{
		HZ_PROFILE_FUNCTION();

		// 카메라 행렬을 셰이더에 전달: 이번 씬의 모든 드로우콜에 공통 적용됨
		s_Data.TextureShader->Bind();
		s_Data.TextureShader->SetMat4("u_ViewProjection", camera.GetViewProjectionMatrix());

		// 기본 텍스처로 흰색 더미 텍스처를 슬롯 0에 바인딩
		// 색상 쿼드는 별도로 텍스처를 바인딩하지 않으므로,
		// 여기서 흰색을 올려두면 "흰색 × 지정 색 = 지정 색"으로 올바른 색이 나옴
		s_Data.WhiteTexture->Bind();

		// 배치 상태 초기화: 기록 위치 포인터를 배열 맨 앞으로 되돌림
		// 이전 프레임 데이터를 지우는 것이 아니라, 덮어쓸 시작 위치를 리셋하는 것
		s_Data.QuadIndexCount = 0;
		s_Data.QuadVertexBufferPtr = s_Data.QuadVertexBufferBase;

		s_Data.TextureSlotIndex = 1; // 새 씬을 시작할 때 전 씬에서 등록된 텍스쳐가 그대로 랜더링 될 수 있으니 슬롯 카운터를 1로 초기화 (0번은 흰색으로 고정)
	}

	void Renderer2D::EndScene()
	{
		HZ_PROFILE_FUNCTION();

		// 포인터 차이로 이번 프레임에 실제로 기록된 바이트 수를 계산
		// uint8_t*로 캐스팅하는 이유: 바이트 단위로 계산하기 위해 (QuadVertex* 차이는 개수가 나옴)
		uint32_t dataSize = (uint32_t)((uint8_t*)s_Data.QuadVertexBufferPtr - (uint8_t*)s_Data.QuadVertexBufferBase);

		// CPU 배열 → GPU VBO 복사: 기록된 크기만큼만 전송
		s_Data.QuadVertexBuffer->SetData(s_Data.QuadVertexBufferBase, dataSize);

		Flush(); // 실제 드로우콜 실행
	}

	void Renderer2D::Flush() // CPU에 쌓아둔 정점 데이터를 GPU로 보내고 드로우 콜을 실행하는 함수
	{
		// 사용된 텍스펴들을 GPU 슬롯에 바인딩
		for (uint32_t i = 0; i < s_Data.TextureSlotIndex; i++) {
			s_Data.TextureSlots[i]->Bind(i); // 슬롯 i번 텍스처를 GPU 텍스처 유닛 i번에 바인딩
		}

		// 이번 프레임에 기록된 인덱스 수만큼만 그림
		// QuadIndexCount가 0이면 아무것도 그리지 않음
		RenderCommand::DrawIndexed(s_Data.QuadVertexArray, s_Data.QuadIndexCount);
	}


	// ── DrawQuad (색상) ─────────────────────────────────────────

	// vec2 버전: Z=0으로 고정해서 vec3 버전에 위임
	void Renderer2D::DrawQuad(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color)
	{
		DrawQuad({ position.x, position.y, 0.0f }, size, color);
	}

	// 실제 구현: 4개의 꼭짓점 데이터를 CPU 배열에 기록하고 포인터를 전진시킴
	// GPU에는 아무것도 보내지 않음 → 모든 DrwaQuad가 끝난 후 EndScene()에서 한 번에 전송
	void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color)
	{
		HZ_PROFILE_FUNCTION();

		const float texIndex = 0.0f; // 단색은 슬롯 0번(흰색 더미 텍스처)
		const float tilingFactor = 1.0f; // 단색은 타일링 없음

		// 좌하단 꼭짓점 (UV: 0,0)
		s_Data.QuadVertexBufferPtr->Position = position;
		s_Data.QuadVertexBufferPtr->Color    = color;
		s_Data.QuadVertexBufferPtr->TexCoord = { 0.0f, 0.0f };
		s_Data.QuadVertexBufferPtr->TexIndex = texIndex;      
		s_Data.QuadVertexBufferPtr->TilingFactor = tilingFactor;
		s_Data.QuadVertexBufferPtr++; // 포인터를 다음 QuadVertex 칸으로 이동

		// 우하단 꼭짓점 (UV: 1,0)
		s_Data.QuadVertexBufferPtr->Position = { position.x + size.x, position.y, 0.0f };
		s_Data.QuadVertexBufferPtr->Color    = color;
		s_Data.QuadVertexBufferPtr->TexCoord = { 1.0f, 0.0f };
		s_Data.QuadVertexBufferPtr->TexIndex = texIndex;
		s_Data.QuadVertexBufferPtr->TilingFactor = tilingFactor;
		s_Data.QuadVertexBufferPtr++;

		// 우상단 꼭짓점 (UV: 1,1)
		s_Data.QuadVertexBufferPtr->Position = { position.x + size.x, position.y + size.y, 0.0f };
		s_Data.QuadVertexBufferPtr->Color    = color;
		s_Data.QuadVertexBufferPtr->TexCoord = { 1.0f, 1.0f };
		s_Data.QuadVertexBufferPtr->TexIndex = texIndex;
		s_Data.QuadVertexBufferPtr->TilingFactor = tilingFactor;
		s_Data.QuadVertexBufferPtr++;

		// 좌상단 꼭짓점 (UV: 0,1)
		s_Data.QuadVertexBufferPtr->Position = { position.x, position.y + size.y, 0.0f };
		s_Data.QuadVertexBufferPtr->Color    = color;
		s_Data.QuadVertexBufferPtr->TexCoord = { 0.0f, 1.0f };
		s_Data.QuadVertexBufferPtr->TexIndex = texIndex;
		s_Data.QuadVertexBufferPtr->TilingFactor = tilingFactor;
		s_Data.QuadVertexBufferPtr++;

		// 이 쿼드를 그리는 데 필요한 인덱스 6개(삼각형 2개)를 카운트에 추가
		s_Data.QuadIndexCount += 6;
	}


	// ── DrawQuad (텍스처) ───────────────────────────────────────
	// 주의: 텍스처 버전은 아직 배치로 전환되지 않음 (커밋 #42에서 처리 예정)
	// 지금은 기존 방식(호출마다 드로우콜 1번)으로 동작함

	void Renderer2D::DrawQuad(const glm::vec2& position, const glm::vec2& size, const Ref<Texture2D>& texture, float tilingFactor, const glm::vec4& tintColor)
	{
		DrawQuad({ position.x, position.y, 0.0f }, size, texture, tilingFactor, tintColor);
	}

	void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec2& size, const Ref<Texture2D>& texture, float tilingFactor, const glm::vec4& tintColor)
	{
		HZ_PROFILE_FUNCTION();

		// 틴트 컬러: 셰이더에서 "텍스처 픽셀 × v_Color"로 계산됨
		// 흰색(1,1,1,1)이면 텍스처 원본 색상 그대로, 다른 색이면 텍스처가 그 색으로 물듦
		constexpr glm::vec4 color = { 1.0f, 1.0f, 1.0f, 1.0f };

		// 이 텍스처가 이미 이번 배치에 등록된 슬롯이 있는지 찾음
		// 같은 텍스처를 여러 쿼드에서 쓸 때 슬롯을 중복 등록하지 않기 위해
		float textureIndex = 0.0f;
		for (uint32_t i = 1; i < s_Data.TextureSlotIndex; i++)
		{
			if (*s_Data.TextureSlots[i].get() == *texture.get())
			{
				textureIndex = (float)i; // 이미 등록돼 있으면 해당 슬롯 번호 사용
				break;
			}
		}

		// 아직 등록되지 않은 새 텍스처라면 다음 빈 슬롯에 등록
		if (textureIndex == 0.0f)
		{
			textureIndex = (float)s_Data.TextureSlotIndex;
			s_Data.TextureSlots[s_Data.TextureSlotIndex] = texture;
			s_Data.TextureSlotIndex++;
		}

		// 좌하단
		s_Data.QuadVertexBufferPtr->Position    = position;
		s_Data.QuadVertexBufferPtr->Color       = color;
		s_Data.QuadVertexBufferPtr->TexCoord    = { 0.0f, 0.0f };
		s_Data.QuadVertexBufferPtr->TexIndex    = textureIndex;
		s_Data.QuadVertexBufferPtr->TilingFactor = tilingFactor;
		s_Data.QuadVertexBufferPtr++;

		// 우하단
		s_Data.QuadVertexBufferPtr->Position    = { position.x + size.x, position.y, 0.0f };
		s_Data.QuadVertexBufferPtr->Color       = color;
		s_Data.QuadVertexBufferPtr->TexCoord    = { 1.0f, 0.0f };
		s_Data.QuadVertexBufferPtr->TexIndex    = textureIndex;
		s_Data.QuadVertexBufferPtr->TilingFactor = tilingFactor;
		s_Data.QuadVertexBufferPtr++;

		// 우상단
		s_Data.QuadVertexBufferPtr->Position    = { position.x + size.x, position.y + size.y, 0.0f };
		s_Data.QuadVertexBufferPtr->Color       = color;
		s_Data.QuadVertexBufferPtr->TexCoord    = { 1.0f, 1.0f };
		s_Data.QuadVertexBufferPtr->TexIndex    = textureIndex;
		s_Data.QuadVertexBufferPtr->TilingFactor = tilingFactor;
		s_Data.QuadVertexBufferPtr++;

		// 좌상단
		s_Data.QuadVertexBufferPtr->Position    = { position.x, position.y + size.y, 0.0f };
		s_Data.QuadVertexBufferPtr->Color       = color;
		s_Data.QuadVertexBufferPtr->TexCoord    = { 0.0f, 1.0f };
		s_Data.QuadVertexBufferPtr->TexIndex    = textureIndex;
		s_Data.QuadVertexBufferPtr->TilingFactor = tilingFactor;
		s_Data.QuadVertexBufferPtr++;

		s_Data.QuadIndexCount += 6;
	}


	// ── DrawRotatedQuad ─────────────────────────────────────────
	// 회전 버전도 아직 배치로 전환되지 않음 (커밋 #42 이후 처리 예정)

	void Renderer2D::DrawRotatedQuad(const glm::vec2& position, const glm::vec2& size, float rotation, const glm::vec4& color)
	{
		DrawRotatedQuad({ position.x, position.y, 0.0f }, size, rotation, color);
	}

	void Renderer2D::DrawRotatedQuad(const glm::vec3& position, const glm::vec2& size, float rotation, const glm::vec4& color)
	{
		HZ_PROFILE_FUNCTION();

		const float texIndex    = 0.0f; // 단색은 슬롯 0번(흰색 더미 텍스처) 고정
		const float tilingFactor = 1.0f; // 단색은 타일링 없음

		// 회전 행렬을 만들어 각 꼭짓점 위치를 직접 계산
		// DrawQuad(color)와 달리 꼭짓점이 정렬된 사각형이 아니므로 행렬로 변환 필요
		glm::mat4 transform = glm::translate(glm::mat4(1.0f), position)
			* glm::rotate(glm::mat4(1.0f), rotation, { 0.0f, 0.0f, 1.0f })
			* glm::scale(glm::mat4(1.0f), { size.x, size.y, 1.0f });

		// 로컬 공간 꼭짓점 4개에 transform을 곱해 월드 공간 위치로 변환 후 버퍼에 기록
		s_Data.QuadVertexBufferPtr->Position    = transform * glm::vec4(-0.5f, -0.5f, 0.0f, 1.0f); // 좌하단
		s_Data.QuadVertexBufferPtr->Color       = color;
		s_Data.QuadVertexBufferPtr->TexCoord    = { 0.0f, 0.0f };
		s_Data.QuadVertexBufferPtr->TexIndex    = texIndex;
		s_Data.QuadVertexBufferPtr->TilingFactor = tilingFactor;
		s_Data.QuadVertexBufferPtr++;

		s_Data.QuadVertexBufferPtr->Position    = transform * glm::vec4(0.5f, -0.5f, 0.0f, 1.0f); // 우하단
		s_Data.QuadVertexBufferPtr->Color       = color;
		s_Data.QuadVertexBufferPtr->TexCoord    = { 1.0f, 0.0f };
		s_Data.QuadVertexBufferPtr->TexIndex    = texIndex;
		s_Data.QuadVertexBufferPtr->TilingFactor = tilingFactor;
		s_Data.QuadVertexBufferPtr++;

		s_Data.QuadVertexBufferPtr->Position    = transform * glm::vec4(0.5f, 0.5f, 0.0f, 1.0f); // 우상단
		s_Data.QuadVertexBufferPtr->Color       = color;
		s_Data.QuadVertexBufferPtr->TexCoord    = { 1.0f, 1.0f };
		s_Data.QuadVertexBufferPtr->TexIndex    = texIndex;
		s_Data.QuadVertexBufferPtr->TilingFactor = tilingFactor;
		s_Data.QuadVertexBufferPtr++;

		s_Data.QuadVertexBufferPtr->Position    = transform * glm::vec4(-0.5f, 0.5f, 0.0f, 1.0f); // 좌상단
		s_Data.QuadVertexBufferPtr->Color       = color;
		s_Data.QuadVertexBufferPtr->TexCoord    = { 0.0f, 1.0f };
		s_Data.QuadVertexBufferPtr->TexIndex    = texIndex;
		s_Data.QuadVertexBufferPtr->TilingFactor = tilingFactor;
		s_Data.QuadVertexBufferPtr++;

		s_Data.QuadIndexCount += 6;
	}

	void Renderer2D::DrawRotatedQuad(const glm::vec2& position, const glm::vec2& size, float rotation, const Ref<Texture2D>& texture, float tilingFactor, const glm::vec4& tintColor)
	{
		DrawRotatedQuad({ position.x, position.y, 0.0f }, size, rotation, texture, tilingFactor, tintColor);
	}

	void Renderer2D::DrawRotatedQuad(const glm::vec3& position, const glm::vec2& size, float rotation, const Ref<Texture2D>& texture, float tilingFactor, const glm::vec4& tintColor)
	{
		HZ_PROFILE_FUNCTION();

		constexpr glm::vec4 color = { 1.0f, 1.0f, 1.0f, 1.0f };

		// 이 텍스처가 이미 이번 배치에 등록된 슬롯이 있는지 찾음
		float textureIndex = 0.0f;
		for (uint32_t i = 1; i < s_Data.TextureSlotIndex; i++)
		{
			if (*s_Data.TextureSlots[i].get() == *texture.get())
			{
				textureIndex = (float)i;
				break;
			}
		}

		if (textureIndex == 0.0f)
		{
			textureIndex = (float)s_Data.TextureSlotIndex;
			s_Data.TextureSlots[s_Data.TextureSlotIndex] = texture;
			s_Data.TextureSlotIndex++;
		}

		glm::mat4 transform = glm::translate(glm::mat4(1.0f), position)
			* glm::rotate(glm::mat4(1.0f), rotation, { 0.0f, 0.0f, 1.0f })
			* glm::scale(glm::mat4(1.0f), { size.x, size.y, 1.0f });

		s_Data.QuadVertexBufferPtr->Position    = transform * glm::vec4(-0.5f, -0.5f, 0.0f, 1.0f);
		s_Data.QuadVertexBufferPtr->Color       = color;
		s_Data.QuadVertexBufferPtr->TexCoord    = { 0.0f, 0.0f };
		s_Data.QuadVertexBufferPtr->TexIndex    = textureIndex;
		s_Data.QuadVertexBufferPtr->TilingFactor = tilingFactor;
		s_Data.QuadVertexBufferPtr++;

		s_Data.QuadVertexBufferPtr->Position    = transform * glm::vec4(0.5f, -0.5f, 0.0f, 1.0f);
		s_Data.QuadVertexBufferPtr->Color       = color;
		s_Data.QuadVertexBufferPtr->TexCoord    = { 1.0f, 0.0f };
		s_Data.QuadVertexBufferPtr->TexIndex    = textureIndex;
		s_Data.QuadVertexBufferPtr->TilingFactor = tilingFactor;
		s_Data.QuadVertexBufferPtr++;

		s_Data.QuadVertexBufferPtr->Position    = transform * glm::vec4(0.5f, 0.5f, 0.0f, 1.0f);
		s_Data.QuadVertexBufferPtr->Color       = color;
		s_Data.QuadVertexBufferPtr->TexCoord    = { 1.0f, 1.0f };
		s_Data.QuadVertexBufferPtr->TexIndex    = textureIndex;
		s_Data.QuadVertexBufferPtr->TilingFactor = tilingFactor;
		s_Data.QuadVertexBufferPtr++;

		s_Data.QuadVertexBufferPtr->Position    = transform * glm::vec4(-0.5f, 0.5f, 0.0f, 1.0f);
		s_Data.QuadVertexBufferPtr->Color       = color;
		s_Data.QuadVertexBufferPtr->TexCoord    = { 0.0f, 1.0f };
		s_Data.QuadVertexBufferPtr->TexIndex    = textureIndex;
		s_Data.QuadVertexBufferPtr->TilingFactor = tilingFactor;
		s_Data.QuadVertexBufferPtr++;

		s_Data.QuadIndexCount += 6;
	}

}
