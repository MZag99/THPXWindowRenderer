#pragma once

#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glu32")

#define NOMINMAX

#include <windows.h>
#include <cstdint>
#include <vector>
#include <iostream>
#include <chrono>
#include <ctime>
#include <string>
#include <algorithm>
#include <map>
#include <tuple>

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

		Vec2D operator+(const Vec2D &v) const
		{
			return Vec2D(x + v.x, y + v.y);
		}
	};

	enum Key
	{
		NONE,
		A, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q, R, S, T, U, V, W, X, Y, Z,
		K0, K1, K2, K3, K4, K5, K6, K7, K8, K9,
		F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12,
		UP, DOWN, LEFT, RIGHT,
		SPACE, TAB, SHIFT, CTRL, INS, DEL, HOME, END, PGUP, PGDN,
		BACK, ESCAPE, RETURN, ENTER, PAUSE, SCROLL,
		NP0, NP1, NP2, NP3, NP4, NP5, NP6, NP7, NP8, NP9,
		NP_MUL, NP_DIV, NP_ADD, NP_SUB, NP_DECIMAL, PERIOD,
		EQUALS, COMMA, MINUS,
		OEM_1, OEM_2, OEM_3, OEM_4, OEM_5, OEM_6, OEM_7, OEM_8,
		CAPS_LOCK, ENUM_END
	};

	static std::map<size_t, uint8_t> mapKeys;

	static const Pixel
		WHITE(255, 255, 255), BLACK(0, 0, 0),
		RED(255, 0, 0), GREEN(0, 255, 0),
		BLUE(0, 0, 255);

	struct HWButton
	{
		bool bPressed = false;	
		bool bReleased = false;	
		bool bHeld = false;
	};



	//===== UTILITY =====//
	class Utils {

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
	
		static void ScanHardware(HWButton* pKeys, bool* pStateOld, bool* pStateNew, uint32_t nKeyCount)
		{
			for (uint32_t i = 0; i < nKeyCount; i++)
			{
				pKeys[i].bPressed = false;
				pKeys[i].bReleased = false;
				if (pStateNew[i] != pStateOld[i])
				{
					if (pStateNew[i])
					{
						pKeys[i].bPressed = !pKeys[i].bHeld;
						pKeys[i].bHeld = true;
					}
					else
					{
						pKeys[i].bReleased = true;
						pKeys[i].bHeld = false;
					}
				}
				pStateOld[i] = pStateNew[i];
			}
		};
	};



	//===== MAIN WINDOW RENDERER CLASS =====//

	class WindowRenderer {

	private:
		// Base name
		std::wstring m_sBaseName = L"THPXWindowRenderer | ";

		// Pixel buffer
		std::vector<Pixel> m_pixelBuffer;
		std::vector<Pixel>* m_pixelBufferPtr;

		float nElapsedTime;

		// Width, height
		int         m_nWindowWidth;
		int         m_nWindowHeight;

		// Pixel scale
		int         m_nPixelSize;

		// Window and context handles
		HINSTANCE   m_hInst;
		HWND        m_hWnd;
		HDC         m_hDC;
		HGLRC       m_hRC;

		friend class THPX::Utils;

		// FPS
		std::chrono::system_clock::time_point m_PrevTime;

		// State of keyboard		
		bool		m_KeyNewState[256] = { 0 };
		bool		m_KeyOldState[256] = { 0 };
		HWButton	m_KeyboardState[256] = { 0 };

		// State of mouse
		bool		m_MouseNewState[3] = { 0 };
		bool		m_MouseOldState[3] = { 0 };
		HWButton	m_MouseState[3] = { 0 };

	protected:
		// User app name
		std::wstring m_sAppName;


	public:
		bool        m_isRunning;


	private:
		int MainLoop() {

			Utils::ScanHardware(m_KeyboardState, m_KeyOldState, m_KeyNewState, 256);

			if (GetKey(Key::ESCAPE).bPressed) {
				m_isRunning = false;
			}

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



		void DrawToScreen(std::vector<Pixel>& buffer) {

			glBegin(GL_POINTS);

			for (int i = 0; i < buffer.capacity(); i++) {
				Pixel p = m_pixelBuffer[i];
				glColor3f(float(p.r) / 255.0f, float(p.g) / 255.0f, float(p.b) / 255.0f);

				glVertex2f((i % ScreenWidth()) * m_nPixelSize, (i / ScreenWidth()) * m_nPixelSize);
			}

			glEnd();
			SwapBuffers(m_hDC);
		}



		void UpdateKeyState(uint32_t keycode, bool value) {
			m_KeyNewState[keycode] = value;
		}



		void AssignKeys() {
			mapKeys[0x00] = Key::NONE;
			mapKeys[0x41] = Key::A; mapKeys[0x42] = Key::B; mapKeys[0x43] = Key::C; mapKeys[0x44] = Key::D; mapKeys[0x45] = Key::E;
			mapKeys[0x46] = Key::F; mapKeys[0x47] = Key::G; mapKeys[0x48] = Key::H; mapKeys[0x49] = Key::I; mapKeys[0x4A] = Key::J;
			mapKeys[0x4B] = Key::K; mapKeys[0x4C] = Key::L; mapKeys[0x4D] = Key::M; mapKeys[0x4E] = Key::N; mapKeys[0x4F] = Key::O;
			mapKeys[0x50] = Key::P; mapKeys[0x51] = Key::Q; mapKeys[0x52] = Key::R; mapKeys[0x53] = Key::S; mapKeys[0x54] = Key::T;
			mapKeys[0x55] = Key::U; mapKeys[0x56] = Key::V; mapKeys[0x57] = Key::W; mapKeys[0x58] = Key::X; mapKeys[0x59] = Key::Y;
			mapKeys[0x5A] = Key::Z;

			mapKeys[0x1B] = Key::ESCAPE;

			mapKeys[VK_F1] = Key::F1; mapKeys[VK_F2] = Key::F2; mapKeys[VK_F3] = Key::F3; mapKeys[VK_F4] = Key::F4;
			mapKeys[VK_F5] = Key::F5; mapKeys[VK_F6] = Key::F6; mapKeys[VK_F7] = Key::F7; mapKeys[VK_F8] = Key::F8;
			mapKeys[VK_F9] = Key::F9; mapKeys[VK_F10] = Key::F10; mapKeys[VK_F11] = Key::F11; mapKeys[VK_F12] = Key::F12;

			mapKeys[VK_DOWN] = Key::DOWN; mapKeys[VK_LEFT] = Key::LEFT; mapKeys[VK_RIGHT] = Key::RIGHT; mapKeys[VK_UP] = Key::UP;
		}



		bool ConstructWindow(int nWidth = 800, int nHeight = 600, int nPixelSize = 2, bool fullScreen = false) {
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

			if (fullScreen) {
				m_nWindowWidth = nWidth;
				m_nWindowHeight = nHeight;
			}
			else {
				m_nWindowWidth = nWidth * m_nPixelSize;
				m_nWindowHeight = nHeight * m_nPixelSize;
			}

			std::wstring sWindowTitle = m_sBaseName + m_sAppName;

			m_pixelBuffer.resize(ScreenWidth() * ScreenHeight());
			m_pixelBufferPtr = &m_pixelBuffer;

			winPtr = this;

			m_hWnd = CreateWindowEx(WS_EX_APPWINDOW, L"myWindowClass", sWindowTitle.c_str(), fullScreen ? WS_POPUP : WS_OVERLAPPED | WS_MINIMIZEBOX | WS_SYSMENU, CW_USEDEFAULT, CW_USEDEFAULT, m_nWindowWidth, m_nWindowHeight, NULL, NULL, NULL, NULL);

			if (!m_hWnd)
				return false;

			if (fullScreen)
			SetWindowLong(m_hWnd, GWL_STYLE, 0);

			m_hDC = GetDC(m_hWnd);

			AssignKeys();

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

			ShowWindow(m_hWnd, fullScreen ? SW_MAXIMIZE : SW_SHOW);

			UpdateWindow(m_hWnd);

			gluOrtho2D(-1 * m_nWindowWidth / 2.0f, 1 * m_nWindowWidth / 2.0f, -1 * m_nWindowHeight / 2.0f, 1 * m_nWindowHeight / 2.0f);
			glScalef(1, -1, 1);
			glTranslatef(-m_nWindowWidth / 2 + m_nPixelSize / 2, -m_nWindowHeight / 2 + m_nPixelSize / 2, 0);

			onCreate();

			return true;
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



		bool Construct(int nWidth, int nHeight, int nPixelSize = 2) {
			return ConstructWindow(nWidth, nHeight, nPixelSize);
		}
		bool Construct(int nPixelSize = 2) {
			int width = GetSystemMetrics(SM_CXSCREEN);
			int height = GetSystemMetrics(SM_CYSCREEN);

			return ConstructWindow(width, height, nPixelSize, true);
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

			int startX = std::min(x0, x1);
			int endX = std::max(x0, x1);

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

			int startX = std::min(x0, x1);
			int endX = std::max(x0, x1);

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
		void FillRectangle(Vec2D position, int width, int height, uint8_t r, uint8_t g, uint8_t b) {
			for (int xPos = position.x; xPos <= position.x + width; xPos++) {
				for (int yPos = position.y; yPos <= position.y + height; yPos++) {
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
		void FillRectangle(Vec2D position, int width, int height, Pixel p) {
			for (int xPos = position.x; xPos <= position.x + width; xPos++) {
				for (int yPos = position.y; yPos <= position.y + height; yPos++) {
					DrawPixel(xPos, yPos, p);
				}
			}
		}
		void FillRectangle(Vec2D v0, Vec2D v1, Vec2D v2, Vec2D v3, Pixel p) {

			std::vector<Vec2D> verts = { v0, v1, v2, v3 };
			Vec2D lTop, rTop, bot;

			std::sort(verts.begin(), verts.end(), [](const Vec2D& a, const Vec2D& b) { return a.y > b.y; });

			bot = verts[0];

			lTop = std::min(verts[2], verts[3], [](const Vec2D& a, const Vec2D& b) { return a.x < b.x; });
			rTop = std::max(verts[2], verts[3], [](const Vec2D& a, const Vec2D& b) { return a.x < b.x; });
			
			for (int xPos = lTop.x; xPos <= rTop.x; xPos++) {
				for (int yPos = lTop.y; yPos <= bot.y; yPos++) {
					DrawPixel(xPos, yPos, p);
				}
			}
		}



		void FillTriangle(Vec2D p0, Vec2D p1, Vec2D p2, uint8_t r, uint8_t g, uint8_t b) {

			std::vector<Vec2D> verts = { p0, p1, p2 };

			std::sort(verts.begin(), verts.end(), [](Vec2D a, Vec2D b) { return a.y > b.y; });

			// If the triangle is bottom-flat
			if (verts[0].y == verts[1].y) {

				float invslope1 = ((float)verts[0].x - (float)verts[2].x) / ((float)verts[0].y - (float)verts[2].y);
				float invslope2 = ((float)verts[1].x - (float)verts[2].x) / ((float)verts[1].y - (float)verts[2].y);

				float curx1 = verts[2].x;
				float curx2 = verts[2].x;

				for (int scanlineY = verts[2].y; scanlineY <= verts[1].y; scanlineY++)
				{
					DrawLine((int)curx1, scanlineY, (int)curx2, scanlineY, r, g, b);
					curx1 += invslope1;
					curx2 += invslope2;
				}
			}

			// If the triangle is top-flat
			else if (verts[1].y == verts[2].y) {

				float invslope1 = ((float)verts[0].x - (float)verts[1].x) / ((float)verts[0].y - (float)verts[1].y);
				float invslope2 = ((float)verts[0].x - (float)verts[2].x) / ((float)verts[0].y - (float)verts[2].y);

				float curx1 = verts[0].x;
				float curx2 = verts[0].x;

				for (int scanlineY = verts[0].y; scanlineY >= verts[1].y; scanlineY--)
				{
					DrawLine((int)curx1, scanlineY, (int)curx2, scanlineY, r, g, b);
					curx1 -= invslope1;
					curx2 -= invslope2;
				}
			}

			// The triangle consists of two top / bottom flat triangles
			else {
				Vec2D newVert((int)(verts[2].x + ((float)(verts[1].y - verts[2].y) / (float)(verts[0].y - verts[2].y)) * (verts[0].x - verts[2].x)), verts[1].y);

				// Draw bottom-flat first
				float botInvslope1 = ((float)verts[1].x - (float)verts[2].x) / ((float)verts[1].y - (float)verts[2].y);
				float botInvslope2 = ((float)newVert.x - (float)verts[2].x) / ((float)newVert.y - (float)verts[2].y);

				float botCurx1 = verts[2].x;
				float botCurx2 = verts[2].x;

				for (int scanlineY = verts[2].y; scanlineY <= verts[1].y; scanlineY++)
				{
					DrawLine((int)botCurx1, scanlineY, (int)botCurx2, scanlineY, r, g, b);
					botCurx1 += botInvslope1;
					botCurx2 += botInvslope2;
				}

				// Then draw top-flat
				float topInvslope1 = ((float)verts[0].x - (float)verts[1].x) / ((float)verts[0].y - (float)verts[1].y);
				float topInvslope2 = ((float)verts[0].x - (float)newVert.x) / ((float)verts[0].y - (float)newVert.y);

				float topCurx1 = verts[0].x;
				float topCurx2 = verts[0].x;

				for (int scanlineY = verts[0].y; scanlineY >= verts[1].y; scanlineY--)
				{
					DrawLine((int)topCurx1, scanlineY, (int)topCurx2, scanlineY, r, g, b);
					topCurx1 -= topInvslope1;
					topCurx2 -= topInvslope2;
				}
			}
		}
		void FillTriangle(Vec2D p0, Vec2D p1, Vec2D p2, THPX::Pixel p) {

			std::vector<Vec2D> verts = { p0, p1, p2 };

			std::sort(verts.begin(), verts.end(), [](Vec2D a, Vec2D b) { return a.y > b.y; });

			// If the triangle is bottom-flat
			if (verts[0].y == verts[1].y) {

				float invslope1 = ((float)verts[0].x - (float)verts[2].x) / ((float)verts[0].y - (float)verts[2].y);
				float invslope2 = ((float)verts[1].x - (float)verts[2].x) / ((float)verts[1].y - (float)verts[2].y);

				float curx1 = verts[2].x;
				float curx2 = verts[2].x;

				for (int scanlineY = verts[2].y; scanlineY <= verts[1].y; scanlineY++)
				{
					DrawLine((int)curx1, scanlineY, (int)curx2, scanlineY, p.r, p.g, p.b);
					curx1 += invslope1;
					curx2 += invslope2;
				}
			}

			// If the triangle is top-flat
			else if (verts[1].y == verts[2].y) {

				float invslope1 = ((float)verts[0].x - (float)verts[1].x) / ((float)verts[0].y - (float)verts[1].y);
				float invslope2 = ((float)verts[0].x - (float)verts[2].x) / ((float)verts[0].y - (float)verts[2].y);

				float curx1 = verts[0].x;
				float curx2 = verts[0].x;

				for (int scanlineY = verts[0].y; scanlineY >= verts[1].y; scanlineY--)
				{
					DrawLine((int)curx1, scanlineY, (int)curx2, scanlineY, p.r, p.g, p.b);
					curx1 -= invslope1;
					curx2 -= invslope2;
				}
			}

			// The triangle consists of two top / bottom flat triangles
			else {
				Vec2D newVert((int)(verts[2].x + ((float)(verts[1].y - verts[2].y) / (float)(verts[0].y - verts[2].y)) * (verts[0].x - verts[2].x)), verts[1].y);

				// Draw bottom-flat first
				float botInvslope1 = ((float)verts[1].x - (float)verts[2].x) / ((float)verts[1].y - (float)verts[2].y);
				float botInvslope2 = ((float)newVert.x - (float)verts[2].x) / ((float)newVert.y - (float)verts[2].y);

				float botCurx1 = verts[2].x;
				float botCurx2 = verts[2].x;

				for (int scanlineY = verts[2].y; scanlineY <= verts[1].y; scanlineY++)
				{
					DrawLine((int)botCurx1, scanlineY, (int)botCurx2, scanlineY, p.r, p.g, p.b);
					botCurx1 += botInvslope1;
					botCurx2 += botInvslope2;
				}

				// Then draw top-flat
				float topInvslope1 = ((float)verts[0].x - (float)verts[1].x) / ((float)verts[0].y - (float)verts[1].y);
				float topInvslope2 = ((float)verts[0].x - (float)newVert.x) / ((float)verts[0].y - (float)newVert.y);

				float topCurx1 = verts[0].x;
				float topCurx2 = verts[0].x;

				for (int scanlineY = verts[0].y; scanlineY >= verts[1].y; scanlineY--)
				{
					DrawLine((int)topCurx1, scanlineY, (int)topCurx2, scanlineY, p.r, p.g, p.b);
					topCurx1 -= topInvslope1;
					topCurx2 -= topInvslope2;
				}
			}
		}



		void Clear(Pixel clearPixel) {
			m_pixelBuffer = std::vector<Pixel>(ScreenWidth() * ScreenHeight(), clearPixel);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		}



		THPX::HWButton GetKey(uint32_t keycode) {
			return m_KeyboardState[keycode];
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
		
		case WM_SYSKEYDOWN:
			winPtr->UpdateKeyState(mapKeys[wp], true);
			break;

		case WM_SYSKEYUP:
			winPtr->UpdateKeyState(mapKeys[wp], false);
			break;

		case WM_KEYDOWN:
			winPtr->UpdateKeyState(mapKeys[wp], true);
			break;

		case WM_KEYUP:
			winPtr->UpdateKeyState(mapKeys[wp], false);
			break;
		}

		return DefWindowProcW(hWnd, msg, wp, lp);
	}

}
