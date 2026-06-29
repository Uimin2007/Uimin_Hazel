// Basic Texture Shader

#type vertex
#version 330 core

// 정점 데이터 입력 (배치 렌더링: 위치/색상/UV/텍스처인덱스/타일링을 정점마다 직접 CPU에서 기록해서 전달)
layout(location = 0) in vec3 a_Position;      // 월드 공간 위치
layout(location = 1) in vec4 a_Color;         // 정점별 색상 (기존 u_Color uniform 대체)
layout(location = 2) in vec2 a_TexCoord;      // UV 좌표
layout(location = 3) in float a_TexIndex;     // 어떤 텍스처 슬롯을 쓸지 (0~31)
layout(location = 4) in float a_TilingFactor; // 텍스처 반복 배수

// 카메라 행렬만 uniform으로 받음
// u_Transform은 제거됨: 배치에서는 DrawQuad가 각 꼭짓점 위치를 직접 계산해 넣으므로 필요 없음
uniform mat4 u_ViewProjection;

out vec4 v_Color;         // 프래그먼트 셰이더로 색상 전달
out vec2 v_TexCoord;      // 프래그먼트 셰이더로 UV 전달
out float v_TexIndex;     // 프래그먼트 셰이더로 텍스처 슬롯 번호 전달
out float v_TilingFactor; // 프래그먼트 셰이더로 타일링 배율 전달

void main()
{
	v_Color        = a_Color;
	v_TexCoord     = a_TexCoord;
	v_TexIndex     = a_TexIndex;
	v_TilingFactor = a_TilingFactor;

	// u_Transform 없이 바로 ViewProjection만 곱함
	// 위치가 이미 월드 공간 좌표로 들어오기 때문
	gl_Position = u_ViewProjection * vec4(a_Position, 1.0);
}

#type fragment
#version 330 core

layout(location = 0) out vec4 color;

in vec4 v_Color;          // 버텍스 셰이더에서 받은 정점 색상
in vec2 v_TexCoord;       // 버텍스 셰이더에서 받은 UV 좌표
in float v_TexIndex;      // 버텍스 셰이더에서 받은 텍스처 슬롯 번호
in float v_TilingFactor;  // 버텍스 셰이더에서 받은 타일링 배율

// 텍스처 배열: 최대 32개의 텍스처를 한 번의 드로우 콜에서 동시에 사용 가능
// Init()에서 SetIntArray로 [0,1,...,31]을 전달해 각 원소가 해당 번호의 GPU 유닛을 가리키게 함
uniform sampler2D u_Textures[32];

void main()
{
	// v_TexIndex로 텍스처 배열에서 올바른 텍스처를 선택해 샘플링
	// - 단색 쿼드: v_TexIndex=0 → u_Textures[0] = 흰색 텍스처 → 흰색(1,1,1,1) × v_Color = v_Color 그대로
	// - 텍스처 쿼드: v_TexIndex=N → u_Textures[N] = 실제 텍스처
	color = texture(u_Textures[int(v_TexIndex)], v_TexCoord * v_TilingFactor) * v_Color;
}