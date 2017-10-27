#include "Framework.h"
#include <string>

template <typename T>
T GetUserDataPtr(HWND hWnd)
{
	return reinterpret_cast<T>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
}

Framework::Framework()
{
	m_grid = new StaticActor(0, 0, "Resource/Image/grid.png");
}

Framework::~Framework()
{
}

bool Framework::Create(HWND hwnd, RECT rect) {
	m_hwnd = hwnd;
	m_rect = rect;

	srand((unsigned)time(NULL));

	m_rect.right -= m_rect.left;
	m_rect.bottom -= m_rect.top;
	m_rect.left = 0;
	m_rect.top = 0;

	GetClientRect(hwnd, &rect);

	// ���� ����
	CreatebackBuffer();

	// Ŭ������ ������ ���ν��� ����
	LONG_PTR result = ::SetWindowLongPtr(
		hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));

	// Ÿ�̸� �ʱ�ȭ
	m_LastUpdate_time = chrono::system_clock::now();
	m_current_time = chrono::system_clock::now();
	m_fps = 0;


	m_Scene[0] = new InGameScene(m_hwnd);
	m_nowScene = 0;

	return (m_hwnd != NULL);
}

HRESULT Framework::OnProcessingWindowMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	switch (nMessageID)
	{
	default:
		return ::DefWindowProc(hWnd, nMessageID, wParam, lParam);

	}
	return 0;
}

LRESULT Framework::MessageProc(HWND hwnd, UINT iMessage, WPARAM wParam, LPARAM lParam) {
	
	Framework* self = ::GetUserDataPtr<Framework*>(hwnd);
	
	if (!self)
		return ::DefWindowProc(hwnd, iMessage, wParam, lParam);
	
	switch (iMessage) {

	case WM_PAINT:
	{
		PAINTSTRUCT		ps;
		HDC mainHDC = BeginPaint(hwnd, &ps);
#pragma region [Doubble buffer]
		HBITMAP GLay = CreateCompatibleBitmap(mainHDC, SCREEN_WIDTH, SCREEN_HEIGHT);
		HDC hdc = CreateCompatibleDC(mainHDC);
		SelectObject(hdc, GLay);
		Rectangle(hdc, -10, -10, SCREEN_WIDTH + 20, SCREEN_HEIGHT + 20);
#pragma endregion
		SetStretchBltMode(mainHDC, COLORONCOLOR);
		SetStretchBltMode(hdc, COLORONCOLOR);

		Draw(hdc);  
		//map.Draw(hdc);
		//myPawn.Draw(hdc);

		m_grid->Draw(hdc, TRUE, m_isGrid);
		
		SetStretchBltMode(mainHDC, COLORONCOLOR);
		SetStretchBltMode(hdc, COLORONCOLOR);
#pragma region [Doubble buffer]
		BitBlt(mainHDC, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, hdc, 0, 0, SRCCOPY);

		//DeleteDC(Memdc);
		DeleteDC(hdc);
		DeleteObject(GLay);
		EndPaint(hwnd, &ps);
#pragma endregion
		break;
	}
	case WM_KEYUP:
	case WM_KEYDOWN:
	{
		KeyProcess(hwnd, iMessage, wParam, lParam);
		break;
	}
	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_RBUTTONDOWN:
	case WM_RBUTTONUP:
	case WM_MOUSEMOVE: {
		MouseProcess(hwnd, iMessage, wParam, lParam);
		break;
	}
	case WM_DESTROY:
	{
		PostQuitMessage(0);
		return 0;
	}

	default:
		return self->OnProcessingWindowMessage(hwnd, iMessage, wParam, lParam);

	}
}

void Framework::Draw(HDC hdc) {
	m_Scene[m_nowScene]->Draw(hdc);
}

void Framework::Timer() {
		m_timeElapsed = chrono::system_clock::now() - m_current_time;
		if (m_timeElapsed.count() > MAX_FPS)
		{
			m_current_time = chrono::system_clock::now();

			if (m_timeElapsed.count() > 0.0)
				m_fps = 1.0 / m_timeElapsed.count();
		}			// �ִ� FPS �̸��� �ð��� ����ϸ� ���� ����
		else return;

		Update(m_timeElapsed.count());
		//PreproccessingForDraw();


		InvalidateRect(m_hwnd, NULL, FALSE);

#if defined(SHOW_CAPTIONFPS)
			m_UpdateElapsed = chrono::system_clock::now() - m_LastUpdate_time;
			if (m_UpdateElapsed.count() > MAX_UPDATE_FPS)
				m_LastUpdate_time = chrono::system_clock::now();
			else return;

			_itow_s((int)(m_fps + 0.1f), m_CaptionTitle + m_TitleLength, (size_t)(TITLE_MX_LENGTH - m_TitleLength), 10);
			wcscat_s(m_CaptionTitle + m_TitleLength, TITLE_MX_LENGTH - m_TitleLength, TEXT(" FPS)"));
			SetWindowText(m_hwnd, m_CaptionTitle);
#endif
}

void Framework::Update(double val) {
	m_Scene[m_nowScene]->Timer(val);
}

bool Framework::KeyProcess(HWND hwnd, UINT iMessage, WPARAM wParam, LPARAM lParam) {
	
	if (m_Scene[m_nowScene]->KeyProcess(hwnd, iMessage, wParam, lParam)) return false;
	switch (iMessage)
	{
	case WM_KEYDOWN:
	{
		switch (wParam) {
		case 'g':
			if (m_isGrid)
				m_isGrid = false;
			else m_isGrid = true;
			break;
		case 'G':
			if (m_isGrid)
				m_isGrid = false;
			else m_isGrid = true;
			break;
		}
	}
	case WM_KEYUP:
		switch (wParam)
		{
		case VK_ESCAPE:
			DestroyWindow(hwnd);
			break;
		}
		return true;
	}
}

bool Framework::MouseProcess(HWND hwnd, UINT iMessage, WPARAM wParam, LPARAM lParam) {
	m_Scene[m_nowScene]->MouseProcess(hwnd, iMessage, wParam, lParam);

	return true;
}

void Framework::Destory() {

}

void Framework::CreatebackBuffer()
{
	if (m_hDC)
	{
		::SelectObject(m_hDC, NULL);
		::DeleteDC(m_hDC);
	}
	if (m_hBitmapBackBuffer) ::DeleteObject(m_hBitmapBackBuffer);

	HDC hdc = ::GetDC(m_hwnd);
	m_hDC = ::CreateCompatibleDC(hdc);
	m_hBitmapBackBuffer = ::CreateCompatibleBitmap(hdc, m_rect.right, m_rect.bottom);
	::SelectObject(m_hDC, m_hBitmapBackBuffer);

	
	m_clrBackBuffer = RGB(255, 255, 255);
	if (m_hbrBackground) ::DeleteObject(m_hbrBackground);
	m_hbrBackground = ::CreateSolidBrush(m_clrBackBuffer);

	::FillRect(m_hDC, &m_rect, m_hbrBackground);

	::ReleaseDC(m_hwnd, hdc);
}