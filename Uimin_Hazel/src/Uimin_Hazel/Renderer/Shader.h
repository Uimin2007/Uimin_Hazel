#pragma once

#include <string>
#include <glm/glm.hpp>  // glm 수학 라이브러리

namespace Hazel {

	class Shader {
	public:
		virtual ~Shader() = default;

		// 셰이더 프로그램을 활성화 할건지 비활성화 할 건지
		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;

		// Uniform 설정 함수 (추상 인터페이스) — OpenGLShader 캐스팅 없이 호출 가능
		virtual void SetInt(const std::string& name, int value) = 0;
		// int 배열을 셰이더에 전달. u_Textures[32] 같은 sampler 배열 초기화에 사용
		virtual void SetIntArray(const std::string& name, int* values, uint32_t count) = 0;
		// float 하나를 셰이더에 전달. u_TilingFactor 같은 단일 실수값에 사용
		virtual void SetFloat(const std::string& name, float value) = 0;
		virtual void SetFloat3(const std::string& name, const glm::vec3& value) = 0;
		virtual void SetFloat4(const std::string& name, const glm::vec4& value) = 0;
		virtual void SetMat4(const std::string& name, const glm::mat4& value) = 0;

		// 셰이더의 이름 반환
		virtual const std::string& GetName() const = 0;

		// 파일 경로 하나만 넘겨주면 파일을 읽어서 셰이더를 생성해주는 함수
		static Ref<Shader> Create(const std::string& filepath);

		// 버텍스, 프레그먼트 소스코드(glfw용)을 그대로 glfw에 전달해서 복붙하는 함수
		static Ref<Shader> Create(const std::string& name, const std::string& vertexSrc, const std::string& fragmentSrc);
	};

	class ShaderLibrary
	{
	public:
		void Add(const std::string& name, const Ref<Shader>& shader);
		void Add(const Ref<Shader>& shader);
		Ref<Shader> Load(const std::string& filepath);
		Ref<Shader> Load(const std::string& name, const std::string& filepath);

		Ref<Shader> Get(const std::string& name);

		bool Exists(const std::string& name) const;
	private:
		std::unordered_map<std::string, Ref<Shader>> m_Shaders;
	};
}

