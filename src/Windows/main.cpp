//
// Random main program file
//

#include <Windows.h>

#include <string>

const std::wstring _className = L"RedactorClass";

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_CLOSE:
		DestroyWindow(hwnd);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hwnd, msg, wParam, lParam);
	}
	return 0;
}

int WINAPI WinMain(
	HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR lpCmdLine,
	int nCmdShow)
{
	MSG       msg = { 0 };

	WNDCLASSEX  wc;

	// Dialog custom window class
	::ZeroMemory(&wc, sizeof(wc));
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = 0;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = _className.c_str();
	wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

	if (!::RegisterClassEx(&wc))
	{
		return 0;
	}

	// Create main application window
	HWND handle = CreateWindowEx(
		WS_EX_CLIENTEDGE,
		_className.c_str(),
		L"Redactor",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		300,
		300,
		NULL,
		NULL,
		hInstance,
		NULL);

	// Show window
	::ShowWindow(handle, nCmdShow);
	::UpdateWindow(handle);

	// Initialise the sensor
	//_kinect = std::make_shared<Sensor>();

	// Main message loop
	while (WM_QUIT != msg.message)
	{
		//_kinect->Update();

		while (PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE))
		{
			// If a dialog message will be taken care of by the dialog proc
			if (handle && IsDialogMessageW(handle, &msg))
			{
				continue;
			}

			TranslateMessage(&msg);
			DispatchMessageW(&msg);
		}
	}

	return static_cast<int>(msg.wParam);
}
