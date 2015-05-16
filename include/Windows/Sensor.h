//
// Simple sensor class
//

#pragma once

#include <Kinect.h>
#include <Windows.h>

#include <memory>

class Sensor
{
public:

	// Depth buffer dimensions
	static const int DEPTH_BUFFER_WIDTH = 512;
	static const int DEPTH_BUFFER_HEIGHT = 424;

	// Colour buffer dimensions
	static const int COLOR_BUFFER_WIDTH = 1920;
	static const int COLOR_BUFFER_HEIGHT = 1080;

	// Singleton accessor
	static Sensor& GetInstance();

	// Simple destructor.
	virtual ~Sensor();
	
	// Update the current content of the frame.
	void Update();

	// Notify that we have a sensor frame ready.
	static const int SENSOR_FRAME_READY = WM_COMMAND + 1;

	// Get the depth buffer data
	RGBQUAD* GetDepthBuffer() const;
	
	// Get the colour buffer data
	RGBQUAD* GetColorBuffer() const;

private:
	// Simple constructor.
	Sensor();

	// Remove the default operator and copy constructor.
	Sensor& operator=(Sensor&) = delete;
	Sensor(Sensor&) = delete;

private:
	// Get the depth data
	void GetDepthData(
		unsigned int bufferSize,
		unsigned short* buffer,
		unsigned int width,
		unsigned int height);

private:
	// The default sensor in use.
	IKinectSensor* _sensor = nullptr;

	// The depth frame.
	IDepthFrameReader* _depthFrameReader = nullptr;

	// Depth buffer
	RGBQUAD* _depthBuffer = nullptr;

	// The color frame
	IColorFrameReader* _colorFrameReader = nullptr;

	// Color buffer
	RGBQUAD* _colorBuffer = nullptr;
};
