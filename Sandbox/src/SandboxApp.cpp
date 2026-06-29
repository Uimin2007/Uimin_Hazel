#include <Hazel.h>
#include <Uimin_Hazel/Core/EntryPoint.h>

#include "Sandbox2D.h"

#include "Platform/OpenGL/OpenGLShader.h"
#include "imgui.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace Hazel;

class ExampleLayer : public Hazel::Layer {
public:
  ExampleLayer() : Layer("Example"), m_CameraController(1280.0f / 720.0f, true) {

    m_VertexArray = Hazel::VertexArray::Create();

    // 삼각형 정점 데이터 정의
    float vertices[3 * 7] = {-0.5f, -0.5f, 0.0f, 0.8f, 0.2f, 0.8f, 1.0f,
                             0.5f,  -0.5f, 0.0f, 0.2f, 0.3f, 0.8f, 1.0f,
                             0.0f,  0.5f,  0.0f, 0.8f, 0.8f, 0.2f, 1.0f};

    // [참고] 이 안에서 m_VertexBuffer가 생성되고 관리되도록 설계되어
    // 있다.
    Hazel::Ref<VertexBuffer> vertexBuffer = VertexBuffer::Create(vertices, sizeof(vertices));
    vertexBuffer->SetLayout({{ShaderDataType::Float3, "a_Position"},
                             {ShaderDataType::Float4, "a_Color"}});

    m_VertexArray->AddVertexBuffer(vertexBuffer); // 버퍼를 VAO에 추가

    uint32_t indices[3] = {0, 1, 2};
    Hazel::Ref<IndexBuffer> indexBuffer = IndexBuffer::Create(indices, sizeof(indices) / sizeof(uint32_t));
    m_VertexArray->SetIndexBuffer(indexBuffer); // 인덱스 버퍼를 설정

    // 2. 사각형 정점 배열 객체 생성
    m_SquareVA = VertexArray::Create();
    
    // [각 정점의 위치 좌표(xyz)와 텍스처 좌표(uv)를 담은 정점당 5칸짜리 배열
    float squareVertices[5 * 4] = {
        -0.5f, -0.5f, 0.0f, 0.0f, 0.0f,
         0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
         0.5f,  0.5f, 0.0f, 1.0f, 1.0f,
        -0.5f,  0.5f, 0.0f, 0.0f, 1.0f
    };
    Hazel::Ref<VertexBuffer> squareVB = VertexBuffer::Create(squareVertices, sizeof(squareVertices));
    squareVB->SetLayout({
        {ShaderDataType::Float3, "a_Position"}, // 위치 좌표
        {ShaderDataType::Float2, "a_TexCoord"}  // 텍스처(UV) 좌표 설정!
    });
    m_SquareVA->AddVertexBuffer(squareVB); // 정점 버퍼를 VAO에 추가!
    
    uint32_t squareIndices[6] = {0, 1, 2, 2, 3, 0}; // 사각형을 삼각형 2개로 쪼개서 그린다.
    Hazel::Ref<IndexBuffer> squareIB = IndexBuffer::Create(squareIndices, sizeof(squareIndices) / sizeof(uint32_t));
    m_SquareVA->SetIndexBuffer(squareIB);

    // [삼각형 전용 셰이더를 위한 m_Shader 코드
    std::string vertexSrc = R"(
        #version 330 core
        layout(location = 0) in vec3 a_Position;
        layout(location = 1) in vec4 a_Color;
        uniform mat4 u_ViewProjection;
        uniform mat4 u_Transform;
        out vec3 v_Position;
        out vec4 v_Color;
        void main() {
            v_Position = a_Position;
            v_Color = a_Color;
            gl_Position = u_ViewProjection * u_Transform * vec4(a_Position, 1.0);	
        }
    )";
    std::string fragmentSrc = R"(
        #version 330 core
        layout(location = 0) out vec4 color;
        in vec3 v_Position;
        in vec4 v_Color;
        void main() {
            color = vec4(v_Position * 0.5 + 0.5, 1.0);
            color = v_Color;
        }
    )";
    m_Shader = Hazel::Shader::Create("VertexPosColor", vertexSrc, fragmentSrc);

    // [사각형 전용 단색 셰이더를 위한 m_FlatColorShader 코드
    std::string flatColorShaderVertexSrc = R"(
        #version 330 core
        layout(location = 0) in vec3 a_Position;
        uniform mat4 u_ViewProjection;
        uniform mat4 u_Transform;
        out vec3 v_Position;
        void main() {
            v_Position = a_Position;
            gl_Position = u_ViewProjection * u_Transform * vec4(a_Position, 1.0);	
        }
    )";
    std::string flatColorShaderFragmentSrc = R"(
        #version 330 core
        layout(location = 0) out vec4 color;
        in vec3 v_Position;
        uniform vec3 u_Color;
        void main() {
            color = vec4(u_Color, 1.0);
        }
    )";
    m_FlatColorShader = Hazel::Shader::Create("FlatColor", flatColorShaderVertexSrc, flatColorShaderFragmentSrc);

    // 텍스처 셰이더 로드
    // [셰이더 파일을 라이브러리에 불러오면서 "Texture"라는 이름으로 셰이더를 등록한다.
    auto textureShader = m_ShaderLibrary.Load("assets/shaders/Texture.glsl");

    // [텍스처 이미지를 불러온다.
    m_Texture = Hazel::Texture2D::Create("assets/textures/Checkerboard.png");
    m_ChernoLogoTexture = Hazel::Texture2D::Create("assets/textures/ChernoLogo.png");

    std::dynamic_pointer_cast<Hazel::OpenGLShader>(textureShader)->Bind();
    std::dynamic_pointer_cast<Hazel::OpenGLShader>(textureShader)->UploadUniformInt("u_Texture", 0);
  }

  void OnUpdate(Timestep ts) override {
    // OrthographicCameraController의 업데이트를 수행하여 카메라 이동 및 회전 입력을 처리함
    m_CameraController.OnUpdate(ts);

    RenderCommand::SetClearColor({0.1f, 0.1f, 0.1f, 1});
    RenderCommand::Clear();

    // 카메라 컨트롤러(OrthographicCameraController)가 관리하는 카메라를 씬 렌더링에 사용한다.
    Renderer::BeginScene(m_CameraController.GetCamera());

    glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(0.1f));

    std::dynamic_pointer_cast<OpenGLShader>(m_FlatColorShader)->Bind();
    std::dynamic_pointer_cast<OpenGLShader>(m_FlatColorShader)->UploadUniformFloat3("u_Color", m_SquareColor);

    for (int y = 0; y < 20; y++) {
      for (int x = 0; x < 20; x++) {
        glm::vec3 pos(x * 0.11f, y * 0.11f, 0.0f); 
        glm::mat4 transform = glm::translate(glm::mat4(1.0f), pos) * scale;
        Renderer::Submit(m_FlatColorShader, m_SquareVA, transform);
      }
    }

    // [셰이더 라이브러리에서 "Texture" 셰이더를 꺼낸다.
    auto textureShader = m_ShaderLibrary.Get("Texture");

    // [텍스처를 바인딩하고 렌더링한다.
    m_Texture->Bind();
    Renderer::Submit(textureShader, m_SquareVA, glm::scale(glm::mat4(1.0f), glm::vec3(1.5f)));
    m_ChernoLogoTexture->Bind();
    Renderer::Submit(textureShader, m_SquareVA, glm::scale(glm::mat4(1.0f), glm::vec3(1.5f)));

    // [기본 삼각형을 렌더링한다.
    Renderer::Submit(m_Shader, m_VertexArray);

    Renderer::EndScene();
  }

  virtual void OnImGuiRender() override {
    ImGui::Begin("Settings");
    ImGui::ColorEdit3("Square Color", glm::value_ptr(m_SquareColor));
    ImGui::End();
  }

  void OnEvent(Event& e) override
  {
        // 이벤트를 카메라 컨트롤러로 전달하여 카메라 관련 이벤트를 처리하게 한다.
        m_CameraController.OnEvent(e);
  }

private:
  Hazel::ShaderLibrary m_ShaderLibrary;
  Hazel::Ref<Hazel::Shader> m_Shader;
  Hazel::Ref<Hazel::VertexArray> m_VertexArray;

  Hazel::Ref<Hazel::Shader> m_FlatColorShader;
  Hazel::Ref<Hazel::VertexArray> m_SquareVA;

  Hazel::Ref<Hazel::Texture2D> m_Texture;
  Hazel::Ref<Hazel::Texture2D> m_ChernoLogoTexture;

  OrthographicCameraController m_CameraController;
  glm::vec3 m_SquareColor = {0.2f, 0.3f, 0.8f};
};

class Sandbox : public Hazel::Application {
public:
  Sandbox() {
    // PushLayer(new ExampleLayer());
    PushLayer(new Sandbox2D());
  }

  ~Sandbox() {}
};

Hazel::Application *Hazel::CreateApplication() {

  return new Sandbox(); // Sandbox 객체를 힙에 생성하여 반환하는 함수
                        // 엔트리 포인트에서 호출됨
}
