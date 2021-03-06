//
//
//

#include "stdafx.h"
#include "Sensor.h"
#include "ImageCache.h"

#include <iostream>
#include <assert.h>
#include <d2d1.h>
#include <Kinect.h>

// define the face frame features required to be computed by this application
static const DWORD FACE_FRAME_FEATURES =
FaceFrameFeatures::FaceFrameFeatures_BoundingBoxInColorSpace
| FaceFrameFeatures::FaceFrameFeatures_PointsInColorSpace
| FaceFrameFeatures::FaceFrameFeatures_RotationOrientation
| FaceFrameFeatures::FaceFrameFeatures_Happy
| FaceFrameFeatures::FaceFrameFeatures_RightEyeClosed
| FaceFrameFeatures::FaceFrameFeatures_LeftEyeClosed
| FaceFrameFeatures::FaceFrameFeatures_MouthOpen
| FaceFrameFeatures::FaceFrameFeatures_MouthMoved
| FaceFrameFeatures::FaceFrameFeatures_LookingAway
| FaceFrameFeatures::FaceFrameFeatures_Glasses
| FaceFrameFeatures::FaceFrameFeatures_FaceEngagement;

// Layout offset in X and Y
static const float FACE_TEXT_LAYOUT_OFFSET_X = -0.1f;
static const float FACE_TEXT_LAYOUT_OFFSET_Y = -0.125f;

Sensor::Sensor()
{
	HRESULT result;

	for (unsigned int i = 0; i < BODY_COUNT; ++i)
	{
		_faceFound[i] = false;
		_face[i] = nullptr;
	}

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

	// nullptr out frame sources
	for (int i = 0; i < BODY_COUNT; ++i)
	{
		_faceFrameSources[i] = nullptr;
		_faceFrameReaders[i] = nullptr;
	}

	// Get the coordinate mapper
	if (SUCCEEDED(result))
	{
		result = _sensor->get_CoordinateMapper(&_coordinateMapper);
	}

	// Get the coordinate frame source
	IBodyFrameSource* bodyFrameSource = nullptr;

	if (SUCCEEDED(result))
	{
		result = _sensor->get_BodyFrameSource(&bodyFrameSource);
	}

	if (SUCCEEDED(result))
	{
		result = bodyFrameSource->OpenReader(&_bodyFrameReader);
	}

	if (SUCCEEDED(result))
	{
		// create a face frame source + reader to track each body in the fov
		for (int i = 0; i < BODY_COUNT; i++)
		{
			if (SUCCEEDED(result))
			{
				// create the face frame source by specifying the required face frame features
				result = ::CreateFaceFrameSource(_sensor, 0, FACE_FRAME_FEATURES, &_faceFrameSources[i]);
			}
			if (SUCCEEDED(result))
			{
				// open the corresponding reader
				result = _faceFrameSources[i]->OpenReader(&_faceFrameReaders[i]);
			}
		}
	}

	if (bodyFrameSource)
	{
		bodyFrameSource->Release();
		bodyFrameSource = nullptr;
	}
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

	if (_bodyFrameReader)
	{
		_bodyFrameReader->Release();
		_bodyFrameReader = nullptr;
	}

	if (_colorFrameReader)
	{
		_colorFrameReader->Release();
		_colorFrameReader = nullptr;
	}

	for (int i = 0; i < BODY_COUNT; ++i)
	{
		if (_faceFrameSources[i])
		{
			_faceFrameSources[i]->Release();
			_faceFrameSources[i] = nullptr;
		}

		if (_faceFrameReaders[i])
		{
			_faceFrameReaders[i]->Release();
			_faceFrameReaders[i] = nullptr;
		}
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

	if (!_bodyFrameReader)
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
		IBody* bodies[BODY_COUNT] = { 0 };

		HRESULT result = GetBodyData(bodies);

		if (SUCCEEDED(result))
		{
			UpdateFaceData(bodies);
		}

		for (unsigned int i = 0; i < BODY_COUNT; ++i)
		{
			if (bodies[i])
			{
				bodies[i]->Release();
				bodies[i] = nullptr;
			}
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

HRESULT Sensor::GetBodyData(IBody** bodies)
{
	HRESULT result = S_FALSE;

	if (_bodyFrameReader)
	{
		IBodyFrame* bodyFrame = nullptr;
		HRESULT result = _bodyFrameReader->AcquireLatestFrame(&bodyFrame);
		if (SUCCEEDED(result))
		{
			result = bodyFrame->GetAndRefreshBodyData(BODY_COUNT, bodies);
		}

		if (bodyFrame)
		{
			bodyFrame->Release();
			bodyFrame = nullptr;
		}
	}

	return result;
}

void Sensor::UpdateFaceData(IBody** bodies)
{
	if (!bodies)
		return;

	HRESULT result;

	for (int i = 0; i < BODY_COUNT; ++i)
	{
		// Get a face frame
		IFaceFrame* faceFrame = nullptr;
		result = _faceFrameReaders[i]->AcquireLatestFrame(&faceFrame);

		BOOLEAN foundFace = false;

		if (faceFrame)
		{
			result = faceFrame->get_IsTrackingIdValid(&foundFace);
		}

		if (SUCCEEDED(result))
		{
			_faceFound[i] = false;

			if (foundFace)
			{
				IFaceFrameResult* faceFrameResult = nullptr;
				result = faceFrame->get_FaceFrameResult(&faceFrameResult);
				_faceFound[i] = true;
				
				if (faceFrameResult)
				{
					result = faceFrameResult->get_FaceBoundingBoxInColorSpace(&_faceBox[i]);
				}

				if (SUCCEEDED(result))
				{
					result = faceFrameResult->GetFacePointsInColorSpace(FacePointType::FacePointType_Count, _facePoints[i]);
				}

				Vector4 faceRotation;

				if (SUCCEEDED(result))
				{
					result = faceFrameResult->get_FaceRotationQuaternion(&faceRotation);
				}

				if (SUCCEEDED(result))
				{
					result = faceFrameResult->GetFaceProperties(FaceProperty::FaceProperty_Count, _faceProperties[i]);
				}

				/*D2D1_POINT_2F faceTextLayout;

				if (SUCCEEDED(result))
				{
					result = TransformBodyToImageSpace(bodies[i], &faceTextLayout);
				}*/

				//if (SUCCEEDED(result))
				//{
				//	// draw face frame results
				//	//_drawDataStreams->DrawFaceFrameResults(iFace, &faceBox, facePoints, &faceRotation, faceProperties, &faceTextLayout);
				//}

				if (faceFrameResult)
				{
					faceFrameResult->Release();
					faceFrameResult = nullptr;
				}
			}
			else
			{
				if (bodies[i])
				{
					IBody* body = bodies[i];
					BOOLEAN track = false;

					result = body->get_IsTracked(&track);

					if (SUCCEEDED(result) && track)
					{
						UINT64 bodyId;
						result = body->get_TrackingId(&bodyId);
					
						if (SUCCEEDED(result))
						{
							// update the face frame source with the tracking ID
							_faceFrameSources[i]->put_TrackingId(bodyId);

							//Store the face...
							_face[i] = ImageCache::GetInstance().GetImage();
						}
					}
				}
			}
		}
	}
}

HRESULT Sensor::TransformBodyToImageSpace(IBody* body, D2D1_POINT_2F* faceTextLayout)
{
	HRESULT result = E_FAIL;

	if (body != nullptr)
	{
		BOOLEAN tracked = false;
		result = body->get_IsTracked(&tracked);

		if (tracked)
		{
			Joint joints[JointType_Count];
			result = body->GetJoints(_countof(joints), joints);
			if (SUCCEEDED(result))
			{
				CameraSpacePoint headJoint = joints[JointType_Head].Position;
				CameraSpacePoint textPoint =
				{
					headJoint.X + FACE_TEXT_LAYOUT_OFFSET_X,
					headJoint.Y + FACE_TEXT_LAYOUT_OFFSET_Y,
					headJoint.Z
				};

				ColorSpacePoint colorPoint = { 0 };
				result = _coordinateMapper->MapCameraPointToColorSpace(textPoint, &colorPoint);

				if (SUCCEEDED(result))
				{
					faceTextLayout->x = colorPoint.X;
					faceTextLayout->y = colorPoint.Y;
				}
			}
		}
	}

	return result;
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
