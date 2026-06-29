#include "hzpch.h"
#include "OpenGLShader.h"

#include <fstream>
#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp> // glm::value_ptr()를 사용하기 위해 포함 (클래스의 데이터를 포인터 배열로 변환해줌)

namespace Hazel {

	// 헬퍼 함수: 글자("vertex")를 OpenGL 고유 번호(GL_VERTEX_SHADER)로 변환
	static GLenum ShaderTypeFromString(const std::string & type) {
		if (type == "vertex") return GL_VERTEX_SHADER;
		if (type == "fragment" || type == "pixel") return GL_FRAGMENT_SHADER;
		HZ_CORE_ASSERT(false, "Unknown shader type!");
		return 0;
	}

	OpenGLShader::OpenGLShader(const std::string& filepath) {
		std::string source = ReadFile(filepath);         // 1. 파일 전체 텍스트 읽기
		auto shaderSources = PreProcess(source);         // 2. '#type' 태그 기준으로 분리하기
		Compile(shaderSources);                          // 3. 분리된 셰이더들을 컴파일 및 링크하기

		// [추가] 파일 경로에서 셰이더 이름 추출 (ex: assets/shaders/Texture.glsl -> Texture)
		auto lastSlash = filepath.find_last_of("/\\");
		lastSlash = lastSlash == std::string::npos ? 0 : lastSlash + 1;
		auto lastDot = filepath.rfind('.');
		auto count = lastDot == std::string::npos ? filepath.size() - lastSlash : lastDot - lastSlash;
		m_Name = filepath.substr(lastSlash, count);
	}

	OpenGLShader::OpenGLShader(const std::string& name, const std::string& vertexSrc, const std::string& fragmentSrc)
		: m_Name(name)
	{
		std::unordered_map<GLenum, std::string> sources;
		sources[GL_VERTEX_SHADER] = vertexSrc;
		sources[GL_FRAGMENT_SHADER] = fragmentSrc;
		Compile(sources); // 문자열을 Map에 담아서 Compile 함수 하나만 부르면 끝
	}

	OpenGLShader::~OpenGLShader()
	{
		// 클래스 소멸 시 그래픽 카드에 저장된 셰이더 프로그램도 같이 지워 메모리를 반환한다.
		glDeleteProgram(m_RendererID);
	}

	std::string OpenGLShader::ReadFile(const std::string& filepath)
	{
		std::string result;
		std::ifstream in(filepath, std::ios::in | std::ios::binary); // 파일 읽기 모드 오류 수정 (OR 연산자)
		if (in)
		{
			in.seekg(0, std::ios::end);
			result.resize(in.tellg());
			in.seekg(0, std::ios::beg);
			in.read(&result[0], result.size());
			in.close();
			;
		}
		else
		{
			HZ_CORE_ERROR("Could not open file '{0}'", filepath);
		}

		return result;
	}

	std::unordered_map<GLenum, std::string> OpenGLShader::PreProcess(const std::string& source)
	{
		std::unordered_map<GLenum, std::string> shaderSources;

		const char* typeToken = "#type";
		size_t typeTokenLength = strlen(typeToken);
		size_t pos = source.find(typeToken, 0);
		while (pos != std::string::npos)
		{
			size_t eol = source.find_first_of("\r\n", pos);
			HZ_CORE_ASSERT(eol != std::string::npos, "Syntax error");
			size_t begin = pos + typeTokenLength + 1;
			std::string type = source.substr(begin, eol - begin);
			HZ_CORE_ASSERT(ShaderTypeFromString(type), "Invalid shader type specified");

			size_t nextLinePos = source.find_first_not_of("\r\n", eol);
			pos = source.find(typeToken, nextLinePos);
			shaderSources[ShaderTypeFromString(type)] = source.substr(nextLinePos, pos - (nextLinePos == std::string::npos ? source.size() - 1 : nextLinePos));
		}

		return shaderSources;
	}

	void OpenGLShader::Compile(const std::unordered_map<GLenum, std::string>& shaderSources)
	{
		GLuint program = glCreateProgram();
		HZ_CORE_ASSERT(shaderSources.size() <= 2, "We only support 2 shaders for now");
		std::array<GLenum, 2> glShaderIDs; // 동적 배열(vector) 대신 고정 크기 배열(array)로 최적화
		int glShaderIDIndex = 0;
		for (auto& kv : shaderSources)
		{
			GLenum type = kv.first;
			const std::string& source = kv.second;

			GLuint shader = glCreateShader(type);

			const GLchar* sourceCStr = source.c_str();
			glShaderSource(shader, 1, &sourceCStr, 0);

			glCompileShader(shader);

			GLint isCompiled = 0;
			glGetShaderiv(shader, GL_COMPILE_STATUS, &isCompiled);
			if (isCompiled == GL_FALSE)
			{
				GLint maxLength = 0;
				glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);

				std::vector<GLchar> infoLog(maxLength);
				glGetShaderInfoLog(shader, maxLength, &maxLength, &infoLog[0]);

				glDeleteShader(shader);

				HZ_CORE_ERROR("{0}", infoLog.data());
				HZ_CORE_ASSERT(false, "Shader compilation failure!");
				break;
			}

			glAttachShader(program, shader);
			glShaderIDs[glShaderIDIndex++] = shader;
		}

		m_RendererID = program;

		// Link our program
		glLinkProgram(program);

		// Note the different functions here: glGetProgram* instead of glGetShader*.
		GLint isLinked = 0;
		glGetProgramiv(program, GL_LINK_STATUS, (int*)&isLinked);
		if (isLinked == GL_FALSE)
		{
			GLint maxLength = 0;
			glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);

			// The maxLength includes the NULL character
			std::vector<GLchar> infoLog(maxLength);
			glGetProgramInfoLog(program, maxLength, &maxLength, &infoLog[0]);

			// We don't need the program anymore.
			glDeleteProgram(program);

			for (auto id : glShaderIDs)
				glDeleteShader(id);

			HZ_CORE_ERROR("{0}", infoLog.data());
			HZ_CORE_ASSERT(false, "Shader link failure!");
			return;
		}

		for (auto id : glShaderIDs)
			glDetachShader(program, id);
	}


	void OpenGLShader::Bind() const
	{
		// OpenGL에게 이제부터 이 셰이더 프로그램(m_RendererID)을 사용하여 렌더링한다고 알림
		glUseProgram(m_RendererID);
	}

	void OpenGLShader::Unbind() const
	{
		glUseProgram(0); // 0을 전달하면 현재 활성화된 셰이더 프로그램을 비활성화하겠다는 뜻
	}

	// Set* 함수들: 추상 인터페이스(Shader.h)를 통해 외부에서 호출하는 진입점
	// 내부적으로는 UploadUniformXxx()에 그대로 위임한다.
	void OpenGLShader::SetInt(const std::string& name, int value)          { UploadUniformInt(name, value); }
	// int 배열을 셰이더 uniform에 전달. 보통 sampler2D 배열 초기화에 사용됨
	void OpenGLShader::SetIntArray(const std::string& name, int* values, uint32_t count) { UploadUniformIntArray(name, values, count); }
	// float 하나를 셰이더 uniform에 업로드 (u_TilingFactor 등 단일 실수 전달에 사용)
	void OpenGLShader::SetFloat(const std::string& name, float value)      { UploadUniformFloat(name, value); }
	void OpenGLShader::SetFloat3(const std::string& name, const glm::vec3& value) { UploadUniformFloat3(name, value); }
	void OpenGLShader::SetFloat4(const std::string& name, const glm::vec4& value) { UploadUniformFloat4(name, value); }
	void OpenGLShader::SetMat4(const std::string& name, const glm::mat4& value)   { UploadUniformMat4(name, value); }

	// 셰이더 변수에 값을 넘기는 과정은 모두 다음 두 단계를 거친다.
	// 1. glGetUniformLocation: 넘기고자 하는 변수 이름("u_Color" 등)이 셰이더 내부의 몇 번 메모리 슬롯에 있는지 찾는다.
	// 2. UploadUniformFloat,2,3,4등등.. : 찾아낸 슬롯 번호(location)에 실제 값을 밀어넣는다

	void OpenGLShader::UploadUniformInt(const std::string& name, int value)
	{
		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		glUniform1i(location, value); // 1i = 1개의 int 타입 데이터를 보냄
	}

	// int 배열을 셰이더 uniform에 업로드
	// glUniform1iv: int 배열 데이터를 한 번에 업로드하는 OpenGL 함수. count개의 값을 지정된 location에 밀어넣음
	void OpenGLShader::UploadUniformIntArray(const std::string& name, int* values, uint32_t count)
	{
		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		glUniform1iv(location, count, values); // 1iv = int 배열 업로드
	}

	void OpenGLShader::UploadUniformFloat(const std::string& name, float value)
	{
		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		glUniform1f(location, value); // 1f = 1개의 float 타입 데이터를 보냄
	}

	void OpenGLShader::UploadUniformFloat2(const std::string& name, const glm::vec2& value)
	{
		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		glUniform2f(location, value.x, value.y); // 2f = 2개의 float 타입 데이터를 연속으로 보냄
	}

	void OpenGLShader::UploadUniformFloat3(const std::string& name, const glm::vec3& value)
	{
		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		glUniform3f(location, value.x, value.y, value.z); // 3f = 3개의 float 데이터를 보냄
	}

	void OpenGLShader::UploadUniformFloat4(const std::string& name, const glm::vec4& value)
	{
		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		glUniform4f(location, value.x, value.y, value.z, value.w); // 4f = 4개의 float 데이터를 보냄
	}

	void OpenGLShader::UploadUniformMat3(const std::string& name, const glm::mat3& matrix)
	{
		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		// Matrix3fv = 3x3 크기의 행렬을 보냅니다. (GL_FALSE는 행렬을 뒤집어(Transpose) 보낼지 여부)
		// glm::value_ptr은 glm 타입의 행렬 객체를 OpenGL이 읽을 수 있는 C 스타일 배열 포인터로 바꿔준다.
		glUniformMatrix3fv(location, 1, GL_FALSE, glm::value_ptr(matrix));
	}

	void OpenGLShader::UploadUniformMat4(const std::string& name, const glm::mat4& matrix)
	{
		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		// Matrix4fv = 4x4 크기의 행렬을 보낸다
		glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(matrix));
	}

}