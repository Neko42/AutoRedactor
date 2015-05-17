//
// Random main program file
//

#include <Windows.h>

#include <iostream>

#include <string>

#include "Sensor.h"

#include "Direct2DRenderer.hpp"
const std::wstring _className = L"RedactorClass";
static Direct2DRenderer *renderer = nullptr;

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case Sensor::SENSOR_FRAME_READY:
	{
		Sensor &kinect = Sensor::GetInstance();
		HRESULT hr = renderer->StartRendering();
		if (SUCCEEDED(hr))
		{
			renderer->RenderColorFrames(kinect.GetColorBuffer());
			renderer->RenderDepthFrames(kinect.GetDepthBuffer());

			for (unsigned int i = 0; i < kinect.GetFaceCount(); ++i)
			{
				if (!kinect.GetFaceFound(i))
					continue;

				RectI faceBox = kinect.GetFaceBox(i);				
				renderer->RenderFaces(faceBox);
			}

			renderer->StopRendering();
		}

		break;
	}
	case WM_CLOSE:
		DestroyWindow(hwnd);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_KEYUP:
	{
		if (wParam == VK_ESCAPE)
		{
			PostQuitMessage(0);
		}
	}
	case WM_SIZE:
	{
		UINT width = LOWORD(lParam);
		UINT height = HIWORD(lParam);
		renderer->OnResize(width, height);
	}
		//result = 0;
		//wasHandled = true;
		break;

	case WM_DISPLAYCHANGE:
	{
		InvalidateRect(hwnd, NULL, FALSE);
	}
		//result = 0;
		//wasHandled = true;
		break;

	case WM_PAINT:
	{
		ValidateRect(hwnd, NULL);
	}
		//result = 0;
		//wasHandled = true;
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
		Direct2DRenderer::WINDOW_WIDTH,
		Direct2DRenderer::WINDOW_HEIGHT,
		NULL,
		NULL,
		hInstance,
		NULL);

	if (renderer == nullptr)
	{
		renderer = new Direct2DRenderer(handle);
		renderer->CreateDeviceIndependentResources();
	}

	// Show window
	::ShowWindow(handle, nCmdShow);
	::UpdateWindow(handle);

	// Main message loop
	while (WM_QUIT != msg.message)
	{
		Sensor::GetInstance().Update();

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
