//
// Simple sensor class
//

#pragma once

#include <Kinect.h>
#include <Kinect.Face.h>
#include <Windows.h>
#include <d2d1.h>

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

	// Get face count.
	unsigned int GetFaceCount() const { return BODY_COUNT; }

	// Get whether face found
	bool GetFaceFound(unsigned int id) const
	{
		if (id >= GetFaceCount())
			return false;

		return _faceFound[id];
	}

	// Get face box
	RectI GetFaceBox(unsigned int id) const
	{
		RectI faceBox {0};

		if (id >= GetFaceCount())
			return faceBox;

		return _faceBox[id];
	}

	ID2D1Bitmap* GetFace(unsigned int id) const
	{
		if (id >= GetFaceCount())
			return nullptr;

		return _face[id];
	}

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

	// Get latest body data
	HRESULT GetBodyData(IBody** bodies);

	// Update to the latest face data
	void UpdateFaceData(IBody** bodies);

	// Get body point in image space
	HRESULT TransformBodyToImageSpace(IBody* body, D2D1_POINT_2F* faceTextLayout);

private:
	// The default sensor in use.
	IKinectSensor*			_sensor = nullptr;

	// The depth frame.
	IDepthFrameReader*		_depthFrameReader = nullptr;

	// Depth buffer
	RGBQUAD*				_depthBuffer = nullptr;

	// The color frame
	IColorFrameReader*		_colorFrameReader = nullptr;

	// Coordinate mapper
	ICoordinateMapper*		_coordinateMapper = nullptr;

	// Body reader
	IBodyFrameReader*		_bodyFrameReader = nullptr;

	// Color buffer
	RGBQUAD*				_colorBuffer = nullptr;

	// Face sources
	IFaceFrameSource*		_faceFrameSources[BODY_COUNT];

	// Face readers
	IFaceFrameReader*		_faceFrameReaders[BODY_COUNT];

	// Face found
	bool					_faceFound[BODY_COUNT];

	// Face box
	RectI					_faceBox[BODY_COUNT];

	// Face...
	ID2D1Bitmap*			_face[BODY_COUNT];

	// Face points
	PointF					_facePoints[BODY_COUNT][FacePointType::FacePointType_Count];

	// Face properties
	DetectionResult			_faceProperties[BODY_COUNT][FaceProperty::FaceProperty_Count];
};
