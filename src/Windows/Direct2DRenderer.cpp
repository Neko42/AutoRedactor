#include "Direct2DRenderer.hpp"
#include "Direct2DHelper.hpp"
#include "ImageCache.h"

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

HRESULT Direct2DRenderer::StartRendering()
{
	HRESULT hr = CreateDeviceResources();
	if (SUCCEEDED(hr))
	{
		m_pRenderTarget->BeginDraw();

		m_pRenderTarget->SetTransform(D2D1::Matrix3x2F::Identity());

		m_pRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::White));
	}
	return hr;
}

void Direct2DRenderer::StopRendering()
{
	HRESULT hr = m_pRenderTarget->EndDraw();

	if (hr == D2DERR_RECREATE_TARGET)
	{
		hr = S_OK;
		DiscardDeviceResources();
	}
}

void Direct2DRenderer::RenderColorFrames(RGBQUAD *color)
{
	HRESULT hr = S_OK;
	int stride = Sensor::COLOR_BUFFER_WIDTH * 4;
	int bufferSize = Sensor::COLOR_BUFFER_HEIGHT * stride;

	hr = m_pWicFactory->CreateBitmapFromMemory(
		Sensor::COLOR_BUFFER_WIDTH,
		Sensor::COLOR_BUFFER_HEIGHT,
		GUID_WICPixelFormat32bppBGR,
		stride,
		bufferSize,
		reinterpret_cast<BYTE *>(color),
		&m_wicBitmap);

	if (SUCCEEDED(hr))
	{
		hr = m_pRenderTarget->CreateBitmapFromWicBitmap(m_wicBitmap, &m_d2dBitmap);
	}

	if (SUCCEEDED(hr))
	{
		D2D1_RECT_F destinationRectangle = D2D1::RectF(
			0.f,
			0.f,
			static_cast<float>(WINDOW_WIDTH),
			static_cast<float>(WINDOW_HEIGHT));

		D2D1_RECT_F sourceRectangle = D2D1::RectF(
			0.0f,
			0.0f,
			static_cast<float>(Sensor::COLOR_BUFFER_WIDTH),
			static_cast<float>(Sensor::COLOR_BUFFER_HEIGHT));

		m_pRenderTarget->DrawBitmap(m_d2dBitmap, &destinationRectangle, 1.f, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR, &sourceRectangle);
	}

	ReleaseBitmaps();
}

void Direct2DRenderer::RenderDepthFrames(RGBQUAD *frame)
{
	HRESULT hr = S_OK;

	const int stride = Sensor::DEPTH_BUFFER_WIDTH * 4;
	const int bufferSize = Sensor::DEPTH_BUFFER_HEIGHT * stride;

	hr = m_pWicFactory->CreateBitmapFromMemory(
		Sensor::DEPTH_BUFFER_WIDTH,
		Sensor::DEPTH_BUFFER_HEIGHT,
		GUID_WICPixelFormat32bppBGR,
		stride,
		bufferSize,
		reinterpret_cast<BYTE *>(frame),
		&m_wicBitmap);

	if (SUCCEEDED(hr))
	{
		hr = m_pRenderTarget->CreateBitmapFromWicBitmap(m_wicBitmap, &m_d2dBitmap);
	}

	if (SUCCEEDED(hr))
	{
		D2D1_RECT_F destinationRectangle = D2D1::RectF(
			static_cast<float>(WINDOW_WIDTH * 0.5f),
			static_cast<float>(WINDOW_HEIGHT * 0.5f),
			static_cast<float>(WINDOW_WIDTH),
			static_cast<float>(WINDOW_HEIGHT));

		D2D1_RECT_F sourceRectangle = D2D1::RectF(
			0.0f,
			0.0f,
			static_cast<float>(Sensor::DEPTH_BUFFER_WIDTH),
			static_cast<float>(Sensor::DEPTH_BUFFER_HEIGHT));

		m_pRenderTarget->DrawBitmap(m_d2dBitmap, &destinationRectangle, 1.f, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR, &sourceRectangle);
	}

	ReleaseBitmaps();
}

void Direct2DRenderer::RenderFaces(const RectI& rect, ID2D1Bitmap* face)
{
	const float widthScale = 1.0f / Sensor::COLOR_BUFFER_WIDTH;
	const float heightScale = 1.0f / Sensor::COLOR_BUFFER_HEIGHT;

	D2D1_RECT_F box;
	box.left = (static_cast<float>(rect.Left) * widthScale) * WINDOW_WIDTH;
	box.top = (static_cast<float>(rect.Top) * heightScale) * WINDOW_HEIGHT;
	box.right = (static_cast<float>(rect.Right) * widthScale) * WINDOW_WIDTH;
	box.bottom = (static_cast<float>(rect.Bottom) * heightScale) * WINDOW_HEIGHT;

	float width = (box.right - box.left);
	float height = (box.bottom - box.top);

	// Keep the aspect...
	float newWidth = width;
	float newHeight = width;
	float offsetX = (newWidth - width) * 0.5f;
	float offsetY = (newHeight - height) * 0.5f;

	box.left -= offsetX;
	box.right += offsetX;
	box.top -= offsetY;
	box.bottom += offsetY;

	// Scale the box...
	height = width;
	newWidth = width * 1.5f;
	newHeight = height * 1.5f;
	offsetX = (newWidth - width) * 0.5f;
	offsetY = (newHeight - height) * 0.5f;

	box.left -= offsetX;
	box.right += offsetX;
	box.top -= offsetY;
	box.bottom += offsetY;

	m_pRenderTarget->DrawRectangle(box, m_pBlackBrush);

	if (face)
	{
		D2D1_SIZE_F size = face->GetSize();
		
		D2D1_RECT_F sourceRectangle = D2D1::RectF(
			0.0f,
			0.0f,
			static_cast<float>(size.width),
			static_cast<float>(size.height));

		m_pRenderTarget->DrawBitmap(face, &box, 1.f, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR, &sourceRectangle);
	}
}

void Direct2DRenderer::ReleaseBitmaps()
{
	if (m_wicBitmap)
	{
		m_wicBitmap->Release();
		m_wicBitmap = nullptr;
	}

	if (m_d2dBitmap)
	{
		m_d2dBitmap->Release();
		m_d2dBitmap = nullptr;
	}
}

void Direct2DRenderer::DiscardDeviceResources()
{
	SafeRelease(&m_pRenderTarget);
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
			&m_pRenderTarget);

		// Create a black brush for rendering...
		hr = m_pRenderTarget->CreateSolidColorBrush(
			D2D1::ColorF(D2D1::ColorF::Black, 1.0f),
			&m_pBlackBrush);

		// Load the various images...
		ImageCache::GetInstance().LoadData();
	}

	return hr;
}

ID2D1Bitmap* Direct2DRenderer::LoadDirect2DImage(const std::wstring& fileName)
{
	FILE* file = nullptr;
	_wfopen_s(&file, fileName.c_str(), L"r");
	if (file)
	{
		fclose(file);
	}
	else
	{
		return nullptr;
	}

	IWICBitmapDecoder* decoder = nullptr;

	HRESULT result = m_pWicFactory->CreateDecoderFromFilename(
		fileName.c_str(),
		NULL,
		GENERIC_READ,
		WICDecodeMetadataCacheOnLoad,
		&decoder);

	IWICBitmapFrameDecode *source = NULL;

	if (SUCCEEDED(result))
	{
		result = decoder->GetFrame(0, &source);
	}

	IWICFormatConverter *converter = NULL;

	if (SUCCEEDED(result))
	{
		// Convert the image format to 32bppPBGRA
		// (DXGI_FORMAT_B8G8R8A8_UNORM + D2D1_ALPHA_MODE_PREMULTIPLIED).
		result = m_pWicFactory->CreateFormatConverter(&converter);
	}

	if (SUCCEEDED(result))
	{
		result = converter->Initialize(
			source,
			GUID_WICPixelFormat32bppPBGRA,
			WICBitmapDitherTypeNone,
			NULL,
			0.f,
			WICBitmapPaletteTypeMedianCut);
	}

	ID2D1Bitmap* bitmap = nullptr;

	if (SUCCEEDED(result))
	{
		result = m_pRenderTarget->CreateBitmapFromWicBitmap(
			converter,
			NULL,
			&bitmap);
	}

	if (decoder)
	{
		decoder->Release();
		decoder = nullptr;
	}

	if (source)
	{
		source->Release();
		source = nullptr;
	}

	if (converter)
	{
		converter->Release();
		converter = nullptr;
	}

	return bitmap;
}