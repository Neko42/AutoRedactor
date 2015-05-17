//
// Image cache
//

#pragma once

#include <vector>
#include <d2d1.h>

class ImageCache
{
public:
	// Default destructor.
	~ImageCache();

	// Get the singleton
	static ImageCache& GetInstance();

	// Load data
	void LoadData();

	// Get a random image
	ID2D1Bitmap* GetImage();

private:
	// Default constructor.
	ImageCache();

	// Remove operator = and copy constructor
	ImageCache& operator=(ImageCache&);
	ImageCache(ImageCache&);

private:
	// The image cache...
	std::vector<ID2D1Bitmap*> _images;
};
