//
//
//

#include "stdafx.h"
#include "Sensor.h"

#include <iostream>
#include <assert.h>

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

	IColorFrameSource* colorFrameSource = nullptr;

	result = _sensor->get_ColorFrameSource(&colorFrameSource);

	if (FAILED(result))
	{
		throw std::exception("Cannot get color frame source");
	}

	result = colorFrameSource->OpenReader(&_colorFrameReader);

	if (FAILED(result))
	{
		throw std::exception("Cannot get color frame reader");
	}

	if (!colorFrameSource)
	{
		colorFrameSource->Release();
		colorFrameSource = nullptr;
	}

	// Create a depth buffer...	
	_depthBuffer = new RGBQUAD[DEPTH_BUFFER_WIDTH * DEPTH_BUFFER_HEIGHT];

	// Create a colour buffer...
	_colorBuffer = new RGBQUAD[COLOR_BUFFER_WIDTH * COLOR_BUFFER_HEIGHT];
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

	if (_colorFrameReader)
	{
		_colorFrameReader->Release();
		_colorFrameReader = nullptr;
	}

	delete [] _depthBuffer;
	delete[] _colorBuffer;
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

	if (!_colorFrameReader)
	{
		return;
	}

	// Get the depth frame...
	IDepthFrame* depthFrame = nullptr;
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
		}

		if (frameDescription)
		{
			frameDescription->Release();
			frameDescription = nullptr;
		}
	}

	// Get the colour frame...
	IColorFrame* colorFrame = NULL;

	result = _colorFrameReader->AcquireLatestFrame(&colorFrame);

	if (SUCCEEDED(result))
	{
		INT64 time = 0;
		IFrameDescription* frameDescription = NULL;

		result = colorFrame->get_RelativeTime(&time);

		if (SUCCEEDED(result))
		{
			result = colorFrame->get_FrameDescription(&frameDescription);
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

		ColorImageFormat imageFormat = ColorImageFormat_None;

		if (SUCCEEDED(result))
		{
			result = colorFrame->get_RawColorImageFormat(&imageFormat);
		}

		unsigned int bufferSize = 0;
		RGBQUAD* buffer = nullptr;

		if (SUCCEEDED(result))
		{
			buffer = _colorBuffer;
			bufferSize = COLOR_BUFFER_WIDTH * COLOR_BUFFER_HEIGHT * sizeof(RGBQUAD);
			result = colorFrame->CopyConvertedFrameDataToArray(bufferSize, reinterpret_cast<BYTE*>(buffer), ColorImageFormat_Bgra);
		}

		if (frameDescription)
		{
			frameDescription->Release();
			frameDescription = nullptr;
		}
	}

	if (SUCCEEDED(result))
	{
		::PostMessage(HWND_BROADCAST, SENSOR_FRAME_READY, 0, 0);
	}

	if (depthFrame)
	{
		depthFrame->Release();
		depthFrame = nullptr;
	}

	if (colorFrame)
	{
		colorFrame->Release();
		colorFrame = nullptr;
	}
}

RGBQUAD* Sensor::GetDepthBuffer() const
{
	return _depthBuffer;
}

RGBQUAD* Sensor::GetColorBuffer() const
{
	return _colorBuffer;
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

		static unsigned int lowest = 0xffffffff;
		static unsigned int highest = 0;

		if (depth < lowest)
			lowest = depth;

		if (depth > highest)
			highest = depth;

		float intensity = ((float)depth / 0x1FFF);

		if (intensity < 0.0f || intensity > 1.f)
		{
			std::cout << "";
		}
		
		depthBuffer->rgbRed = static_cast<BYTE>(intensity * 255.0f);
		depthBuffer->rgbGreen = static_cast<BYTE>(intensity * 255.0f);
		depthBuffer->rgbBlue = static_cast<BYTE>(intensity * 255.0f);

		++depthBuffer;
		++buffer;
	}
}
