#include "Direct2DRenderer.hpp"
#include "Direct2DHelper.hpp"

#include <Windows.h>

#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <wchar.h>
#include <math.h>

#include <d2d1.h>
#include <d2d1helper.h>
#include <dwrite.h>
#include <wincodec.h>

#include <wincodecsdk.h>
#include "Sensor.h"
Direct2DRenderer::Direct2DRenderer(HWND handle)
	: m_hwnd(handle),
	m_pDirect2dFactory(NULL),
	m_pRenderTarget(NULL),
	m_pWicFactory(NULL)
{
}

Direct2DRenderer::~Direct2DRenderer()
{
}

HRESULT Direct2DRenderer::CreateDeviceIndependentResources()
{
	HRESULT hr = S_OK;

	// Create a Direct2D factory.
	hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &m_pDirect2dFactory);

	CoInitialize(NULL);
	hr = CoCreateInstance(CLSID_WICImagingFactory, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&m_pWicFactory));

	return hr;
}

void Direct2DRenderer::OnResize(UINT width, UINT height)
{
	if (m_pRenderTarget)
	{
		// Note: This method can fail, but it's okay to ignore the
		// error here, because the error will be returned again
		// the next time EndDraw is called.
		m_pRenderTarget->Resize(D2D1::SizeU(width, height));
	}
}

void Direct2DRenderer::RenderKinectFrame(RGBQUAD *frame)
{
	HRESULT hr = S_OK;

	hr = CreateDeviceResources();
	if (SUCCEEDED(hr))
	{
		m_pRenderTarget->BeginDraw();

		m_pRenderTarget->SetTransform(D2D1::Matrix3x2F::Identity());

		m_pRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::White));

		D2D1_SIZE_F rtSize = m_pRenderTarget->GetSize();
		IWICBitmap* wicBitmap = nullptr;
		ID2D1Bitmap *d2Bitmap = nullptr;

		const int stride = Sensor::DEPTH_BUFFER_WIDTH * 4;
		const int bufferSize = Sensor::DEPTH_BUFFER_HEIGHT * stride;
		
		hr = m_pWicFactory->CreateBitmapFromMemory(
			Sensor::DEPTH_BUFFER_WIDTH,
			Sensor::DEPTH_BUFFER_HEIGHT,
			GUID_WICPixelFormat32bppBGR,
			stride,
			bufferSize,
			reinterpret_cast<BYTE *>(frame),
			&wicBitmap);

		if (SUCCEEDED(hr))
		{
			hr = m_pRenderTarget->CreateBitmapFromWicBitmap(wicBitmap, &d2Bitmap);
		}

		if (SUCCEEDED(hr))
		{
			D2D1_RECT_F destinationRectangle = D2D1::RectF(
				0.0f,
				0.0f,
				Sensor::DEPTH_BUFFER_WIDTH,
				Sensor::DEPTH_BUFFER_HEIGHT);

			D2D1_RECT_F sourceRectangle = D2D1::RectF(
				0.0f,
				0.0f,
				Sensor::DEPTH_BUFFER_WIDTH,
				Sensor::DEPTH_BUFFER_HEIGHT);

			m_pRenderTarget->DrawBitmap(d2Bitmap, &destinationRectangle, 1.f, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR, &sourceRectangle);
		}

		hr = m_pRenderTarget->EndDraw();

		if (wicBitmap)
		{
			wicBitmap->Release();
			wicBitmap = nullptr;
		}

		if (d2Bitmap)
		{
			d2Bitmap->Release();
			d2Bitmap = nullptr;
		}
		if (hr == D2DERR_RECREATE_TARGET)
		{
			hr = S_OK;
			DiscardDeviceResources();
		}
	}
}

void Direct2DRenderer::DiscardDeviceResources()
{
	SafeRelease(&m_pRenderTarget);
	SafeRelease(&m_pWicFactory);
}

HRESULT Direct2DRenderer::CreateDeviceResources()
{
	HRESULT hr = S_OK;

	if (!m_pRenderTarget)
	{
		RECT rc;
		GetClientRect(m_hwnd, &rc);

		D2D1_SIZE_U size = D2D1::SizeU(
			rc.right - rc.left,
			rc.bottom - rc.top
			);

		// Create a Direct2D render target.
		hr = m_pDirect2dFactory->CreateHwndRenderTarget(
			D2D1::RenderTargetProperties(),
			D2D1::HwndRenderTargetProperties(m_hwnd, size),
			&m_pRenderTarget
			);

	}

	return hr;
}