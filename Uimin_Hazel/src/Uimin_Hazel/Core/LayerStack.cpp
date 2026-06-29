#include "hzpch.h"
#include "LayerStack.h"

namespace Hazel {

	// 생성자 : m_LayerInsert를 벡터의 맨 처음으로 설정
	LayerStack::LayerStack() {
		
	}
	// 소멸자 : 모든 레이어 메모리 해제
	LayerStack::~LayerStack() {
		for (Layer* layer : m_Layers) {
			layer->OnDetach(); // 삭제 전에 레이어 자신이 쓰던 자원을 먼저 정리하도록 호출
			delete layer;
		}
	}

	// 일반 레이어 추가 함수
	void LayerStack::PushLayer(Layer* layer) {
		m_Layers.emplace(m_Layers.begin() + m_LayerInsertIndex, layer);
		m_LayerInsertIndex++;
	}

	// 오버레이 추가
	void LayerStack::PushOverlay(Layer* overlay)
	{
		m_Layers.emplace_back(overlay); // 벡터 맨 뒤에 추가(항상 일반 레이어 뒤에 위치)
	}


	// 일반 레이어 제거
	void LayerStack::PopLayer(Layer* layer) {
		auto it = std::find(m_Layers.begin(), m_Layers.end(), layer); // 지울 레이어를 탐색하고 그 위치를 it에 저장한다.
		// find는 만약 못 찾았다면 end()를 반환.

		if (it != m_Layers.end()) { // 지울 대상의 위치(it)가 레이어들의 end()가 아니고 다른 반환이라면 = 찾았다는 뜻
			layer->OnDetach(); // 스택에서 꺼내기 전에 레이어 자신이 정리 작업을 하도록 호출
			m_Layers.erase(it);
			m_LayerInsertIndex--; // 지웠으니 경계선도 하나 앞으로 당겨줘야 한다.
		}
	}

	// 오버레이 제거
	void LayerStack::PopOverlay(Layer* overlay) {
		auto it = std::find(m_Layers.begin(), m_Layers.end(), overlay);

		if (it != m_Layers.end()) {
			overlay->OnDetach(); // 스택에서 꺼내기 전에 오버레이 자신이 정리 작업을 하도록 호출
			m_Layers.erase(it);
		}
	}
}