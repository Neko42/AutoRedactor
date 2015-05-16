//
//
//

#include "stdafx.h"
#include "Sensor.h"

#include <iostream>

Sensor::Sensor()
{
	HRESULT result;

	result = ::GetDefaultKinectSensor(&_sensor);
	if (FAILED(result))
	{
		throw std::exception("Cannot get sensor");
	}

	IDepthFrameSource* depthFrameSource = nullptr;

	result = _sensor->Open();

	if (FAILED(result))
	{
		throw std::exception("Cannot open sensor");
	}

	result = _sensor->get_DepthFrameSource(&depthFrameSource);

	if (FAILED(result))
	{
		throw std::exception("Cannot get depth frame source");
	}

	result = depthFrameSource->OpenReader(&_depthFrameReader);

	if (FAILED(result))
	{
		throw std::exception("Cannot get depth frame reader");
	}

	if (!depthFrameSource)
	{
		depthFrameSource->Release();
		depthFrameSource = nullptr;
	}

	// Create a depth buffer...	
	_depthBuffer = new RGBQUAD[DEPTH_BUFFER_WIDTH * DEPTH_BUFFER_HEIGHT];
}

Sensor::~Sensor()
{
	if (_sensor)
	{
		_sensor->Close();
		_sensor->Release();
		_sensor = nullptr;
	}

	if (_depthFrameReader)
	{
		_depthFrameReader->Release();
		_depthFrameReader = nullptr;
	}

	delete [] _depthBuffer;
}

Sensor& Sensor::GetInstance()
{
	static Sensor* _instance = new Sensor();
	return *_instance;
}

void Sensor::Update()
{
	if (!_depthFrameReader)
	{
		return;
	}

	IDepthFrame* depthFrame = nullptr;

	// Get latest depth frame...
	HRESULT result = _depthFrameReader->AcquireLatestFrame(&depthFrame);

	if (SUCCEEDED(result))
	{
		INT64 time = 0;
		IFrameDescription* frameDescription = NULL;

		result = depthFrame->get_RelativeTime(&time);

		if (SUCCEEDED(result))
		{
			result = depthFrame->get_FrameDescription(&frameDescription);
		}

		int width = 0;

		if (SUCCEEDED(result))
		{
			result = frameDescription->get_Width(&width);
		}

		int height = 0;

		if (SUCCEEDED(result))
		{
			result = frameDescription->get_Height(&height);
		}

		unsigned short minDistance = 0;

		if (SUCCEEDED(result))
		{
			result = depthFrame->get_DepthMinReliableDistance(&minDistance);
		}

		unsigned short maxDistance = USHRT_MAX;

		//if (SUCCEEDED(result))
		//{
		//	//// hr = pDepthFrame->get_DepthMaxReliableDistance(&maxDistance);
		//}

		unsigned int bufferSize = 0;
		unsigned short* buffer = NULL;

		if (SUCCEEDED(result))
		{
			result = depthFrame->AccessUnderlyingBuffer(
				&bufferSize,
				&buffer);
		}

		if (SUCCEEDED(result))
		{
			GetDepthData(
				bufferSize,
				buffer,
				width,
				height);

			::PostMessage(HWND_BROADCAST, SENSOR_FRAME_READY, 0, 0);
		}

		if (frameDescription)
		{
			frameDescription->Release();
			frameDescription = nullptr;
		}
	}

	if (depthFrame)
	{
		depthFrame->Release();
		depthFrame = nullptr;
	}
}

RGBQUAD* Sensor::GetDepthBuffer() const
{
	return _depthBuffer;
}

void Sensor::GetDepthData(
	unsigned int bufferSize,
	unsigned short* buffer,
	unsigned int width,
	unsigned int height)
{
	if (!buffer || !_depthBuffer)
		return;

	if (width != DEPTH_BUFFER_WIDTH)
		return;

	if (height != DEPTH_BUFFER_HEIGHT)
		return;

	RGBQUAD* depthBuffer = _depthBuffer;
	RGBQUAD* depthBufferEnd = _depthBuffer + (width * height);

	while (depthBuffer < depthBufferEnd)
	{
		unsigned short depth = *buffer;
		
		float intensity = 1.0f / (depth / static_cast<float>(SHRT_MAX));
		depthBuffer->rgbRed = static_cast<BYTE>(intensity * 255.0f);
		depthBuffer->rgbGreen = static_cast<BYTE>(intensity * 255.0f);
		depthBuffer->rgbBlue = static_cast<BYTE>(intensity * 255.0f);

		++depthBuffer;
		++buffer;
	}
}
