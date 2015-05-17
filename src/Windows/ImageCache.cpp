//
//
//

#include "stdafx.h"

#include "ImageCache.h"
#include "Direct2DRenderer.hpp"

#include <math.h>
#include <sstream>
#include <d2d1.h>

// FIXME: Horrible hack to get renderer...
extern Direct2DRenderer* renderer;

ImageCache::ImageCache()
{

}

ImageCache::~ImageCache()
{

}

ImageCache& ImageCache::GetInstance()
{
	static ImageCache* instance = new ImageCache();
	return *instance;
}

void ImageCache::LoadData()
{
	for (unsigned int i = 0; i < 300; ++i)
	{
		// FIXME: Hardcoding the face location relative to Debug/Release dirs... 
		std::wstringstream fileName;
		fileName << "..\\..\\..\\img\\face_";
		fileName << i;
		fileName << ".jpg";

		ID2D1Bitmap* faceImage = renderer->LoadDirect2DImage(fileName.str());

		if (faceImage)
		{
			_images.push_back(faceImage);
		}
	}
}

ID2D1Bitmap* ImageCache::GetImage()
{
	if (_images.size() == 0)
		return nullptr;

	int id = ::rand() % _images.size();
	
	id = max(0, id);
	id = min(_images.size() - 1, static_cast<unsigned int>(id));

	return _images[id];
}
