#include <iostream>
#include "THPXWindowRenderer.h"

class Example : public THPX::WindowRenderer {
public:
	Example() {
		m_sAppName = L"Dupsko";
	}


	void onCreate() override {
		
	}

	
	void onUpdate() override {
		Clear();

		for (int x = 0; x < ScreenWidth(); x++) {
			for (int y = 0; y < ScreenHeight(); y++) {
				DrawPixel(x, y, rand() % 255, rand() % 255, rand() % 255);
			}
		}

	}
};

int main() {

	Example demo;

	if (demo.Construct(640, 480, 2)) {
		demo.StartWindowProcedure();
	}

	return 0;
}