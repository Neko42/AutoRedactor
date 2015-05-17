#ifndef __DIRECT2DRENDERER_HPP

#include <d2d1.h>
#include <d2d1helper.h>
#include <dwrite.h>
#include <wincodec.h>
#include <Kinect.Face.h>
#include <string>

class Direct2DRenderer
{
public:
	static const int WINDOW_WIDTH = 1280;
	static const int WINDOW_HEIGHT = 720;

	Direct2DRenderer(HWND handle);
	~Direct2DRenderer();
	HRESULT CreateDeviceIndependentResources();
	HRESULT CreateDeviceResources();

	void OnResize(UINT width, UINT height);

	HRESULT StartRendering();
	
	void RenderColorFrames(RGBQUAD *color);
	void RenderDepthFrames(RGBQUAD *frame);
	void RenderFaces(const RectI& rect, ID2D1Bitmap* face);
	ID2D1Bitmap* LoadDirect2DImage(const std::wstring& fileName);

	void StopRendering();
private:
	void ReleaseBitmaps();
	void DiscardDeviceResources();

	Direct2DRenderer& operator=(Direct2DRenderer&) = delete;
	Direct2DRenderer(Direct2DRenderer &) = delete;

	HWND m_hwnd;
	ID2D1Factory* m_pDirect2dFactory;
	ID2D1HwndRenderTarget* m_pRenderTarget;
	IWICImagingFactory *m_pWicFactory;
	ID2D1SolidColorBrush* m_pBlackBrush;

	IWICBitmap* m_wicBitmap;
	ID2D1Bitmap* m_d2dBitmap;
};

#endif