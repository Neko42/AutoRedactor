//
//
//

#include "stdafx.h"
#include "Sensor.h"

Sensor::Sensor()
{
	HRESULT result;

	result = ::GetDefaultKinectSensor(&_sensor);
	if (FAILED(result))
	{
		throw std::exception("Cannot get sensor");
	}

	IDepthFrameSource* pDepthFrameSource = NULL;

	result = _sensor->Open();

	if (FAILED(result))
	{
		throw std::exception("Cannot open sensor");
	}

	result = _sensor->get_DepthFrameSource(&pDepthFrameSource);

	if (FAILED(result))
	{
		throw std::exception("Cannot get depth frame source");
	}

	result = pDepthFrameSource->OpenReader(&_depthFrameReader);

	if (FAILED(result))
	{
		throw std::exception("Cannot get depth frame reader");
	}

	pDepthFrameSource->Release();
	pDepthFrameSource = nullptr;
}

Sensor::~Sensor()
{

}

void Sensor::Update()
{
	if (!_depthFrameReader)
	{
		return;
	}

	IDepthFrame* depthFrame = NULL;

	HRESULT hr = _depthFrameReader->AcquireLatestFrame(&depthFrame);

	if (SUCCEEDED(hr))
	{
		INT64 nTime = 0;
		IFrameDescription* pFrameDescription = NULL;
		int nWidth = 0;
		int nHeight = 0;
		USHORT nDepthMinReliableDistance = 0;
		USHORT nDepthMaxDistance = 0;
		UINT nBufferSize = 0;
		UINT16 *pBuffer = NULL;

		hr = depthFrame->get_RelativeTime(&nTime);

		if (SUCCEEDED(hr))
		{
			hr = depthFrame->get_FrameDescription(&pFrameDescription);
		}

		if (SUCCEEDED(hr))
		{
			hr = pFrameDescription->get_Width(&nWidth);
		}

		if (SUCCEEDED(hr))
		{
			hr = pFrameDescription->get_Height(&nHeight);
		}

		if (SUCCEEDED(hr))
		{
			hr = depthFrame->get_DepthMinReliableDistance(&nDepthMinReliableDistance);
		}

		if (SUCCEEDED(hr))
		{
			// In order to see the full range of depth (including the less reliable far field depth)
			// we are setting nDepthMaxDistance to the extreme potential depth threshold
			nDepthMaxDistance = USHRT_MAX;

			// Note:  If you wish to filter by reliable depth distance, uncomment the following line.
			//// hr = pDepthFrame->get_DepthMaxReliableDistance(&nDepthMaxDistance);
		}

		if (SUCCEEDED(hr))
		{
			hr = depthFrame->AccessUnderlyingBuffer(&nBufferSize, &pBuffer);
		}

		if (SUCCEEDED(hr))
		{
			//ProcessDepth(nTime, pBuffer, nWidth, nHeight, nDepthMinReliableDistance, nDepthMaxDistance);
		}

		if (pFrameDescription)
			pFrameDescription->Release();
	}

	if (depthFrame)
		depthFrame->Release();
}

