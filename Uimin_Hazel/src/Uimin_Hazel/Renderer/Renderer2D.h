#pragma once

#include "OrthographicCamera.h"
#include "Texture.h"   // DrawQuad(texture) 선언에 Texture2D 타입 필요

namespace Hazel {
    class Renderer2D
    {
    public:
        static void Init();
        static void Shutdown();
        static void BeginScene(const OrthographicCamera& camera);
        static void EndScene();

        // EndScene 내부에서 호출되며, CPU 버퍼를 GPU에 올리고 드로우콜을 실행한다
        // 나중에 배치가 꽉 찼을 때 중간에 강제로 비워내기 위해 public으로 선언
        static void Flush();

        // 색상으로 사각형 그리기
        static void DrawQuad(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color);
        static void DrawQuad(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color);

        // 텍스처로 사각형 그리기
        // tilingFactor: 텍스처를 몇 배 반복할지 (1.0이면 반복 없음, 기본값)
        // tintColor: 텍스처에 곱할 색상 (흰색이면 텍스처 색상 그대로, 기본값)
        // 기본값이 있으므로 기존 DrawQuad(..., texture) 호출은 수정 없이 그대로 동작
        static void DrawQuad(const glm::vec2& position, const glm::vec2& size, const Ref<Texture2D>& texture, float tilingFactor = 1.0f, const glm::vec4& tintColor = glm::vec4(1.0f));
        static void DrawQuad(const glm::vec3& position, const glm::vec2& size, const Ref<Texture2D>& texture, float tilingFactor = 1.0f, const glm::vec4& tintColor = glm::vec4(1.0f));

        // Z축 기준으로 회전된 사각형 그리기
        // rotation: 회전 각도(라디안). glm::radians(45.0f) 처럼 변환해서 넘기면 됨
        // 단색 버전
        static void DrawRotatedQuad(const glm::vec2& position, const glm::vec2& size, float rotation, const glm::vec4& color);
        static void DrawRotatedQuad(const glm::vec3& position, const glm::vec2& size, float rotation, const glm::vec4& color);
        // 텍스처 버전
        static void DrawRotatedQuad(const glm::vec2& position, const glm::vec2& size, float rotation, const Ref<Texture2D>& texture, float tilingFactor = 1.0f, const glm::vec4& tintColor = glm::vec4(1.0f));
        static void DrawRotatedQuad(const glm::vec3& position, const glm::vec2& size, float rotation, const Ref<Texture2D>& texture, float tilingFactor = 1.0f, const glm::vec4& tintColor = glm::vec4(1.0f));
    };
}
