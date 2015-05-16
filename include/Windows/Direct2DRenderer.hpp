#ifndef __DIRECT2DRENDERER_HPP

#include <d2d1.h>
#include <d2d1helper.h>
#include <dwrite.h>
#include <wincodec.h>

class Direct2DRenderer
{
public:
	Direct2DRenderer(HWND handle);
	~Direct2DRenderer();
	HRESULT CreateDeviceIndependentResources();

	HRESULT CreateDeviceResources();

	void OnResize(UINT width, UINT height);

	void RenderKinectFrame(RGBQUAD *frame);

	void DiscardDeviceResources();

private:

	Direct2DRenderer& operator=(Direct2DRenderer&) = delete;
	Direct2DRenderer(Direct2DRenderer &) = delete;

	HWND m_hwnd;
	ID2D1Factory* m_pDirect2dFactory;
	ID2D1HwndRenderTarget* m_pRenderTarget;
	IWICImagingFactory *m_pWicFactory;
};

#endif