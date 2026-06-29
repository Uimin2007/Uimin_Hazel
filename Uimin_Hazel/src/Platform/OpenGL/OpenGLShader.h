#pragma once

#include "Uimin_Hazel/Renderer/Shader.h"
#include <glm/glm.hpp>

typedef unsigned int GLenum; // 나중에 삭제예정

namespace Hazel {

	// OpenGL 그래픽스 API 전용으로 구현된 셰이더 클래스
	// 부모인 Shader 인터페이스의 기능(Bind, Unbind 등)을 구체적으로 어떻게 할지(OpenGL 방식으로) 정의한다.
	class OpenGLShader : public Shader
	{
	public:
		// 생성자로 Texture.glsl에서 
		OpenGLShader(const std::string& filepath);
		// 생성자: 버텍스 셰이더 소스와 프래그먼트 셰이더 소스를 문자열로 받아서,
		// 내부적으로 OpenGL을 통해 셰이더 프로그램을 컴파일하고 링크(결합)한다.
		OpenGLShader(const std::string& name, const std::string& vertexSrc, const std::string& fragmentSrc);

		// 소멸자: 클래스가 파괴될 때 그래픽 카드 메모리에 올라간 셰이더 프로그램도 같이 삭제
		virtual ~OpenGLShader();

		virtual void Bind() const override;
		virtual void Unbind() const override;

		// Shader.h의 추상 메서드 구현 — 내부에서 UploadUniformXxx() 호출
		virtual void SetInt(const std::string& name, int value) override;
		// int 배열을 셰이더 uniform에 전달 (예: u_Textures[32] sampler 배열 초기화)
		virtual void SetIntArray(const std::string& name, int* values, uint32_t count) override;
		// float 단일 값을 셰이더 uniform에 전달 (예: u_TilingFactor)
		virtual void SetFloat(const std::string& name, float value) override;
		virtual void SetFloat3(const std::string& name, const glm::vec3& value) override;
		virtual void SetFloat4(const std::string& name, const glm::vec4& value) override;
		virtual void SetMat4(const std::string& name, const glm::mat4& value) override;

		virtual const std::string& GetName() const override { return m_Name; }

		// --- Uniform 업로드 함수들 ---
		// 셰이더 코드(.glsl 등) 내부에 있는 uniform 변수들에게 C++ 코드의 데이터를 전달해주는 역할
		// name: 셰이더에 선언된 변수 이름 (예: "u_Color", "u_Transform")
		// value/matrix: 넘겨줄 실제 데이터 값
		
		void UploadUniformInt(const std::string& name, int value); // 정수형 데이터 전달
		// int 배열을 셰이더 uniform에 전달. glUniform1iv를 감싼 함수 (sampler 배열 초기화용)
		void UploadUniformIntArray(const std::string& name, int* values, uint32_t count);
		void UploadUniformFloat(const std::string& name, float value); // 실수형 데이터 전달
		void UploadUniformFloat2(const std::string& name, const glm::vec2& value); // 2차원 벡터(x, y) 전달
		void UploadUniformFloat3(const std::string& name, const glm::vec3& value); // 3차원 벡터(x, y, z) 전달 (주로 RGB 색상이나 3D 위치)
		void UploadUniformFloat4(const std::string& name, const glm::vec4& value); // 4차원 벡터(x, y, z, w) 전달 (주로 RGBA 투명도 포함 색상)

		void UploadUniformMat3(const std::string& name, const glm::mat3& matrix); // 3x3 행렬 전달
		void UploadUniformMat4(const std::string& name, const glm::mat4& matrix); // 4x4 행렬 전달 (주로 이동/회전/크기 변환이나 카메라 시점 변환에 사용)
	private:
		// 1. 하드디스크에서 텍스트 파일을 읽어와 하나의 긴 문자열로 반환
		std::string ReadFile(const std::string & filepath);
		// 2. 읽어온 긴 문자열을 '#type' 기준으로 잘라내어 (타입 -> 소스코드) 형태의 맵(사전)으로 만든다.
		std::unordered_map<GLenum, std::string> PreProcess(const std::string& source);
		// 3. 분리된 소스코드들을 실제로 OpenGL에 전달하여 컴파일하고 하나로 링크
		void Compile(const std::unordered_map<GLenum, std::string>&shaderSources);
	private:
		// 그래픽 카드(OpenGL) 측에서 이 셰이더 프로그램을 식별하기 위해 발급해준 고유 ID 값이다.
		uint32_t m_RendererID;
		std::string m_Name;
	};

}
