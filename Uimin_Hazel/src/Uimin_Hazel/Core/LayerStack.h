#pragma once

#include "Uimin_Hazel/Core/Core.h"
#include "Layer.h"

#include <vector>

// 레이어 스택. 일반 레이어는 앞쪽에(vector) 삽입되고, 오버레이 레이어는 뒤쪽에 추가된다.

// * 자세한 구현은 LayerStack.cpp를 참고

namespace Hazel {

	class HAZEL_API LayerStack {
	public:
		LayerStack();
		~LayerStack();

		void PushLayer(Layer* layer); // 일반 레이어를 추가한다.
		void PushOverlay(Layer* overlay); // 오버레이를 추가한다.
		void PopLayer(Layer* layer);	// 일반 레이어를 제거한다.
		void PopOverlay(Layer* overlay); // 오버레이를 제거한다.


		std::vector<Layer*>::iterator begin() { return m_Layers.begin(); }
		std::vector<Layer*>::iterator end() { return m_Layers.end(); }
	private:
		std::vector<Layer*> m_Layers; // 레이어들을 저장하는 벡터
		// 이 iterator는 일반 레이어와 오버레이 레이어의 경계를 나타내며, 힙 메모리의 주소값이 아닌 벡터 내 삽입 위치를 추적하는 인덱스이다.
		unsigned int m_LayerInsertIndex = 0; 
		
	};


}

