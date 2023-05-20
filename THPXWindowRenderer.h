#pragma once

#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glu32")

#include <windows.h>
#include <cstdint>
#include <vector>
#include <iostream>
#include <chrono>
#include <ctime>
#include <string>
#include <algorithm>

#include <gl/GL.h>
#include <gl/GLU.h>


namespace THPX {

	class WindowRenderer;
	WindowRenderer* winPtr = nullptr;

	struct Pixel {
		uint8_t r, g, b;

		Pixel() {};

		Pixel(uint8_t red, uint8_t green, uint8_t blue) {
			r = red;
			g = green;
			b = blue;
		}
	};


	struct Vec2D {
		int x, y;

		Vec2D() {};

		Vec2D(int posX, int posY) {
			x = posX;
			y = posY;
		};
	};


	static const Pixel 
		WHITE(255, 255, 255), BLACK(0, 0, 0),
		RED(255, 0, 0), GREEN(0, 255, 0),
		BLUE(0, 0, 255);



	class Utils {
		//===== UTILITY =====//

		static std::wstring ConvertS2W(std::string s) {
#ifdef __MINGW32__
			wchar_t* buffer = new wchar_t[s.length() + 1];
			mbstowcs(buffer, s.c_str(), s.length());
			buffer[s.length()] = L'\0';
#else
			int count = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, NULL, 0);
			wchar_t* buffer = new wchar_t[count];
			MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, buffer, count);
#endif
			std::wstring w(buffer);
			delete[] buffer;
			return w;
		}

		static LRESULT CALLBACK WindowProcedure(HWND, UINT, WPARAM, LPARAM);

		friend class WindowRenderer;
	};



	//===== MAIN WINDOW RENDERER CLASS =====//

	class WindowRenderer {

	private:
		// Base name
		std::wstring m_sBaseName = L"THPXWindowRenderer | ";
		
		// Pixel buffer
		std::vector<Pixel> m_pixelBuffer;
		std::vector<Pixel> *m_pixelBufferPtr;

		float nElapsedTime;

		// Width, height
		int m_nWindowWidth;
		int m_nWindowHeight;

		// Pixel scale
		int m_nPixelSize;

		// Window and context handles
		HINSTANCE m_hInst;
		HWND m_hWnd;
		HDC m_hDC;
		HGLRC m_hRC;

		friend class THPX::Utils;

		// FPS
		std::chrono::system_clock::time_point m_PrevTime;

	protected:
		// User app name
		std::wstring m_sAppName;


	public:
		bool m_isRunning;


	private:
		int MainLoop() {
			
			onUpdate();
			DrawToScreen(m_pixelBuffer);

			return 0;
		}



		void SetFPS() {


			auto current = std::chrono::system_clock::now();
			std::chrono::duration<double> elapsedSeconds = current - m_PrevTime;
			
			nElapsedTime += elapsedSeconds.count();
			
			m_PrevTime = current;
			std::string sTitle = std::string(m_sBaseName.begin(), m_sBaseName.end()) + std::string(m_sAppName.begin(), m_sAppName.end()) + " @FPS: " + std::to_string((int)(1.0f / elapsedSeconds.count()));
			
			if (nElapsedTime >= 1.0f) {
				SetWindowText(m_hWnd, Utils::ConvertS2W(sTitle).c_str());
				nElapsedTime = 0;
			}
		}



		void DrawToScreen(std::vector<Pixel> &buffer) {

			glBegin(GL_POINTS);

			for (int i = 0; i < buffer.capacity(); i++) {
				Pixel p = m_pixelBuffer[i];
				glColor3f(float(p.r) / 255.0f, float(p.g) / 255.0f, float(p.b) / 255.0f);

				glVertex2f((i % ScreenWidth()) * m_nPixelSize, (i / ScreenWidth()) * m_nPixelSize);
			}
			
			glEnd();
			SwapBuffers(m_hDC);
		}



		void onWindowCreate(HWND hWnd) {
			m_isRunning = true;
		}



		void onDestroy() {
			m_isRunning = false;
		}



	public:
		WindowRenderer() {
			m_hInst = GetModuleHandle(NULL);
		}



		bool Construct(int nWidth = 800, int nHeight = 600, int nPixelSize = 1) {

			WNDCLASSEX wc = { 0 };

			wc.cbSize = sizeof(WNDCLASSEX);
			wc.cbWndExtra = NULL;
			wc.hbrBackground = (HBRUSH)GetStockObject(GRAY_BRUSH);
			wc.hCursor = LoadCursor(NULL, IDC_ARROW);
			wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
			wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
			wc.hInstance = m_hInst;
			wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;

			wc.lpszClassName = L"myWindowClass";

			wc.lpfnWndProc = Utils::WindowProcedure;

			if (!RegisterClassEx(&wc))
				return false;

			m_nPixelSize = nPixelSize;

			m_nWindowWidth = nWidth * m_nPixelSize;
			m_nWindowHeight = nHeight * m_nPixelSize;

			std::wstring sWindowTitle = m_sBaseName + m_sAppName;

			m_pixelBuffer.resize(ScreenWidth() * ScreenHeight());
			m_pixelBufferPtr = &m_pixelBuffer;

			RECT rc = { 0, 0, m_nWindowWidth, m_nWindowHeight };
			AdjustWindowRect(&rc, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU, false);

			int clientWidth = rc.right - rc.left;
			int clientHeight = rc.bottom - rc.top;

			winPtr = this;

			m_hWnd = CreateWindowEx(WS_EX_OVERLAPPEDWINDOW, L"myWindowClass", sWindowTitle.c_str(), WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, m_nWindowWidth, m_nWindowHeight, NULL, NULL, NULL, NULL);

			if (!m_hWnd)
				return false;

			m_hDC = GetDC(m_hWnd);

			
			PIXELFORMATDESCRIPTOR pfd;
			ZeroMemory(&pfd, sizeof(PIXELFORMATDESCRIPTOR));

			
			pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
			pfd.nVersion = 1;
			pfd.dwFlags = PFD_DOUBLEBUFFER | PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW;
			pfd.iPixelType = PFD_TYPE_RGBA;
			pfd.cColorBits = 32;
			pfd.cDepthBits = 24;
			pfd.iLayerType = PFD_MAIN_PLANE;
			

			int pF = ChoosePixelFormat(m_hDC, &pfd);
			SetPixelFormat(m_hDC, pF, &pfd);

			m_hRC = wglCreateContext(m_hDC);
			wglMakeCurrent(m_hDC, m_hRC);

			glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
			glPointSize(m_nPixelSize);

			m_pixelBuffer = std::vector<Pixel>(ScreenWidth() * ScreenHeight(), Pixel(THPX::BLACK));

			ShowWindow(m_hWnd, SW_SHOW);
			UpdateWindow(m_hWnd);
			
			

			gluOrtho2D(-1 * m_nWindowWidth / 2.0f, 1 * m_nWindowWidth / 2.0f, -1 * m_nWindowHeight / 2.0f, 1 * m_nWindowHeight / 2.0f);
			glScalef(1, -1, 1);
			glTranslatef(-m_nWindowWidth / 2 + m_nPixelSize / 2, -m_nWindowHeight / 2 + m_nPixelSize / 2, 0);

			onCreate();

			return true;
		}



		int StartWindowProcedure() {
			MSG msg = { 0 };

			while (m_isRunning) {
				if (PeekMessage(&msg, m_hWnd, 0, 0, PM_REMOVE)) {
					TranslateMessage(&msg);
					DispatchMessage(&msg);
				}
				else {
					SetFPS();
					MainLoop();
				}
			}

			return 0;
		}



		void DrawPixel(int x, int y, uint8_t r, uint8_t g, uint8_t b) {
			if (x >= 0 && x < ScreenWidth() && y >= 0 && y < ScreenHeight()) {
				Pixel p(r, g, b);
				(*m_pixelBufferPtr)[y * ScreenWidth() + x] = p;
			}
		}
		void DrawPixel(int x, int y, Pixel p) {
			if (x >= 0 && x < ScreenWidth() && y >= 0 && y < ScreenHeight()) {
				(*m_pixelBufferPtr)[y * ScreenWidth() + x] = p;
			}
		}



		void DrawLine(int x0, int y0, int x1, int y1, uint8_t r, uint8_t g, uint8_t b) {

			int startX = min(x0, x1);
			int endX = max(x0, x1);

			int startY, endY;

			if (startX == x0) {
				startY = y0;
				endY = y1;
			}
			else {
				startY = y1;
				endY = y0;
			}

			int dx = endX - startX;
			int dy = endY - startY;


			float prevY = startY;
			float a = float(dy) / float(dx);


			DrawPixel(startX, startY, r, g, b);

			for (int x = startX + 1; x <= endX; x++) {
				
				float y = prevY + a;

				// If the slope is more vertical
				int deltaY = y - prevY;

				// If the slope goes downwards
				if (deltaY > 0) {
					for (int i = 1; i < deltaY; i++) {
						DrawPixel(x, y - i, r, g, b);
					}
				}
				// If the slope goes upwards
				else {
					for (int i = 1; i < abs(deltaY); i++) {
						DrawPixel(x, y + i, r, g, b);
					}
				}
				
				DrawPixel(x, y, r, g, b);
				prevY = y;
			}
		}
		void DrawLine(int x0, int y0, int x1, int y1, Pixel p) {

			int startX = min(x0, x1);
			int endX = max(x0, x1);

			int startY, endY;

			if (startX == x0) {
				startY = y0;
				endY = y1;
			}
			else {
				startY = y1;
				endY = y0;
			}

			int dx = endX - startX;
			int dy = endY - startY;


			float prevY = startY;
			float a = float(dy) / float(dx);


			DrawPixel(startX, startY, p);

			for (int x = startX + 1; x <= endX; x++) {

				float y = prevY + a;

				// If the slope is more vertical
				int deltaY = y - prevY;

				// If the slope goes downwards
				if (deltaY > 0) {
					for (int i = 1; i < deltaY; i++) {
						DrawPixel(x, y - i, p);
					}
				}
				// If the slope goes upwards
				else {
					for (int i = 1; i < abs(deltaY); i++) {
						DrawPixel(x, y + i, p);
					}
				}

				DrawPixel(x, y, p);
				prevY = y;
			}
		}



		void FillRectangle(int x, int y, int width, int height, uint8_t r, uint8_t g, uint8_t b) {
			for (int xPos = x; xPos <= x + width; xPos++) {
				for (int yPos = y; yPos <= y + height; yPos++) {
					DrawPixel(xPos, yPos, r, g, b);
				}
			}
		}
		void FillRectangle(int x, int y, int width, int height, Pixel p) {
			for (int xPos = x; xPos <= x + width; xPos++) {
				for (int yPos = y; yPos <= y + height; yPos++) {
					DrawPixel(xPos, yPos, p);
				}
			}
		}



		void FillTriangle(Vec2D p0, Vec2D p1, Vec2D p2, uint8_t r, uint8_t g, uint8_t b) {

			std::vector<Vec2D> verts = { p0, p1, p2 };

			//for (Vec2D v : verts) {
			//	printf("x: %d, y: %d\n", v.x, v.y);
			//}

			std::sort(verts.begin(), verts.end(), [](Vec2D a, Vec2D b) { return a.y > b.y; });

			// If the triangle is bottom-flat
			if (verts[0].y == verts[1].y) {

			}

			// If the triangle is top-flat
			else if (verts[1].y == verts[2].y) {
				DrawPixel(verts[0].x, verts[0].y, r, g, b);
				DrawPixel(verts[1].x, verts[1].y, r, g, b);
				DrawPixel(verts[2].x, verts[2].y, r, g, b);
			}

			// The triangle consists of two top / bottom flat triangles
			else {
				//Vec2D midPoint
			}
		}



		void Clear(Pixel clearPixel) {
			m_pixelBuffer = std::vector<Pixel>(ScreenWidth() * ScreenHeight(), clearPixel);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		}



		virtual void onUpdate() {

		}



		virtual void onCreate() {

		}



		int ScreenWidth() {
			return m_nWindowWidth / m_nPixelSize;
		}



		int ScreenHeight() {
			return m_nWindowHeight / m_nPixelSize;
		}
	};



	LRESULT CALLBACK Utils::WindowProcedure(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp) {	
		switch (msg) {
		case WM_CREATE:
			winPtr->onWindowCreate(hWnd);
			break;

		case WM_DESTROY:
			winPtr->onDestroy();
			PostQuitMessage(0);
			break;
		}
		return DefWindowProcW(hWnd, msg, wp, lp);
	}

}
