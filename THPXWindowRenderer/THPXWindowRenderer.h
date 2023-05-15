#pragma once

#pragma comment(lib, "opengl32.lib")

#include <windows.h>
#include <cstdint>
#include <vector>
#include <iostream>
#include <chrono>
#include <ctime>
#include <string>

#include <gl/GL.h>
#include <gl/GLU.h>


namespace THPX {

	//===== UTILITY =====//

	std::wstring ConvertS2W(std::string s) {
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

	//===== DEFINITIONS =====//

	class WindowRenderer;
	LRESULT CALLBACK WindowProcedure(HWND, UINT, WPARAM, LPARAM);

	WindowRenderer* winPtr = nullptr;


	struct Pixel {
		int x, y;
		uint8_t r, g, b;
	};



	//===== MAIN WINDOW RENDERER CLASS =====//

	class WindowRenderer {

	private:
		std::wstring m_sBaseName = L"THPXWindowRenderer | ";
		std::vector<Pixel> m_pixelBuffer;
		std::vector<Pixel> *m_pixelBufferPtr;

		float nElapsedTime;

	public:
		bool m_isRunning;

	protected:
		// Width, height & name
		int m_nWindowWidth;
		int m_nWindowHeight;

		std::wstring m_sAppName;

		// Pixel scale
		int m_nPixelSize;

		// Window and context handles
		HINSTANCE m_hInst;
		HWND m_hWnd;
		HDC m_hDC;
		HGLRC m_hRC;

		// FPS
		std::chrono::system_clock::time_point m_PrevTime;

	private:
		int MainLoop() {
			
			onUpdate();
			DrawToScreen();

			return 0;
		}



		void SetFPS() {


			auto current = std::chrono::system_clock::now();
			std::chrono::duration<double> elapsedSeconds = current - m_PrevTime;
			
			nElapsedTime += elapsedSeconds.count();
			
			m_PrevTime = current;
			std::string sTitle = std::string(m_sBaseName.begin(), m_sBaseName.end()) + std::string(m_sAppName.begin(), m_sAppName.end()) + " @FPS: " + std::to_string((int)(1.0f / elapsedSeconds.count()));
			
			if (nElapsedTime >= 1.0f) {
				SetWindowText(m_hWnd, ConvertS2W(sTitle).c_str());
				nElapsedTime = 0;
			}
		}



		void DrawToScreen() {

			glBegin(GL_POINTS);

			for (Pixel p : m_pixelBuffer) {
				glColor3f(float(p.r) / 255.0f, float(p.g) / 255.0f, float(p.b) / 255.0f);
				glVertex2f(p.x, p.y);
			}
			
			glEnd();
			SwapBuffers(m_hDC);
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

			wc.lpfnWndProc = WindowProcedure;

			if (!RegisterClassEx(&wc))
				return false;

			m_nPixelSize = nPixelSize;

			m_nWindowWidth = nWidth * m_nPixelSize;
			m_nWindowHeight = nHeight * m_nPixelSize;

			std::wstring sWindowTitle = m_sBaseName + m_sAppName;

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

			glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
			glPointSize(m_nPixelSize);
			
			glOrtho(-1 * m_nWindowWidth / 2.0, 1 * m_nWindowWidth / 2.0, -1 * m_nWindowHeight / 2.0, 1 * m_nWindowHeight / 2.0, 0.0f, 1.0f);
			glScalef(1, -1, 1);
			glTranslatef(-(m_nWindowWidth / 2.0f) + (float)(m_nPixelSize / 2), -(m_nWindowHeight / 2.0f) + (float)(m_nPixelSize / 2), 0.0f);
			
			m_pixelBuffer.resize(ScreenWidth() * ScreenHeight());

			m_pixelBufferPtr = &m_pixelBuffer;


			ShowWindow(m_hWnd, SW_SHOW);
			UpdateWindow(m_hWnd);

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

			Pixel p;

			p.x = x * m_nPixelSize;
			p.y = y * m_nPixelSize;

			p.r = r;
			p.g = g;
			p.b = b;

			(*m_pixelBufferPtr)[y * ScreenWidth() + x] = p;
		}



		void Clear() {
			m_pixelBuffer.reserve(ScreenWidth() * ScreenHeight());
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		}



		virtual void onUpdate() {

		}



		virtual void onCreate() {

		}



		void onWindowCreate(HWND hWnd) {
			m_isRunning = true;

			onCreate();
		}



		void onDestroy() {
			m_isRunning = false;
		}



		int ScreenWidth() {
			return m_nWindowWidth / m_nPixelSize;
		}



		int ScreenHeight() {
			return m_nWindowHeight / m_nPixelSize;
		}
	};



	//===== WINDOW PROCEDURE LOOP =====//

	LRESULT CALLBACK WindowProcedure(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp) {	
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