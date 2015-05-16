//
// Simple sensor class
//

#pragma once

#include <Kinect.h>

class Sensor
{
public:
	// Singleton accessor
	static Sensor& GetInstance();

	// Simple destructor.
	virtual ~Sensor();
	
	// Update the current content of the frame.
	void Update();
	
private:
	// Simple constructor.
	Sensor();

	// Remove the default operator and copy constructor.
	Sensor& operator=(Sensor&) = delete;
	Sensor(Sensor&) = delete;

private:
	// The default sensor in use.
	IKinectSensor* _sensor = nullptr;

	// The depth frame.
	IDepthFrameReader* _depthFrameReader = nullptr;
};
