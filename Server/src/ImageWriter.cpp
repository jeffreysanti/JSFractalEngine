/*
 * ImageWriter.cpp
 *
 *  Created on: Apr 26, 2014
 *      Author: jeffrey
 */

#include "ImageWriter.h"


#define cimg_display 0 // Disable X11 Library Requirements
#define cimg_use_png 1
#include "CImg/CImg.h"
using namespace cimg_library;

#define MIN(x,y,z) std::min(x, std::min(y, z))
#define MAX(x,y,z) std::max(x, std::max(y, z))


ImageWriter::ImageWriter(unsigned int w, unsigned int h) {
	// TODO Auto-generated constructor stub
	CImg<unsigned char> *img = new CImg<unsigned char>(w,h,1,4, 0);
	_img = (CImg<unsigned char> *)img;
}

ImageWriter::~ImageWriter() {
	// TODO Auto-generated destructor stub
	CImg<unsigned char> *img = (CImg<unsigned char> *)_img;
	delete img;
	_img = 0;
}

void ImageWriter::saveFile(std::string flName)
{
	CImg<unsigned char> *img = (CImg<unsigned char> *)_img;
	img->save(flName.c_str());
}

void ImageWriter::fillImage(unsigned char r, unsigned char g, unsigned char b)
{
	CImg<unsigned char> *img = (CImg<unsigned char> *)_img;
	for(int x=0; x<img->width(); x ++){
		for(int y=0; y<img->height(); y ++){
			img->atXY(x,y,0) = r;
			img->atXY(x,y,1) = g;
			img->atXY(x,y,2) = b;
			img->atXY(x,y,3) = 255;
		}
	}
}

unsigned int ImageWriter::getWidth()
{
	CImg<unsigned char> *img = (CImg<unsigned char> *)_img;
	return img->width();
}

unsigned int ImageWriter::getHeight()
{
	CImg<unsigned char> *img = (CImg<unsigned char> *)_img;
	return img->height();
}

void ImageWriter::setPixel(unsigned int x, unsigned int y, unsigned char r, unsigned char g, unsigned char b)
{
	CImg<unsigned char> *img = (CImg<unsigned char> *)_img;
	img->atXY(x, y, 0) = r;
	img->atXY(x, y, 1) = g;
	img->atXY(x, y, 2) = b;
	img->atXY(x, y, 3) = 255;
}

void ImageWriter::setPixel(unsigned int x, unsigned int y, Color col)
{
	setPixel(x, y, col.r, col.g, col.b);
}

void ImageWriter::setPixelHSV(unsigned int x, unsigned int y, double h, double s, double v)
{
	Color c = ColorPalette::fromHSV(h,s,v);
	setPixel(x, y, c.r, c.g, c.b);
}

void ImageWriter::drawCircle(unsigned int x, unsigned int y, unsigned int rad, Color col)
{
	CImg<unsigned char> *img = (CImg<unsigned char> *)_img;
	int color[4];
	color[0] = col.r;
	color[1] = col.g;
	color[2] = col.b;
	color[3] = 255;
	if(rad == 0)
		img->draw_rectangle(x, y, x, y, color);
	else if(rad == 1)
		img->draw_rectangle(x-1, y-1, x, y, color);
	else if(rad == 2)
		img->draw_rectangle(x-1, y-1, x+1, y+1, color);
	else{
		img->draw_rectangle(x-rad/2, y-rad/2, x+rad/2, y+rad/2, color);
	}

}

void ImageWriter::overlayImage(ImageWriter &writer, double opacity)
{
	CImg<unsigned char> *img = (CImg<unsigned char> *)_img;
	CImg<unsigned char> *imgOverlay = (CImg<unsigned char> *)writer._img;

	if(img->width() != imgOverlay->width() || img->height() != imgOverlay->height()){
		std::cerr << "Logic Error: Overlay and Main Image Different Dimensions!\n";
	}

	for(int x=0; x<img->width(); x++){
		for(int y=0; y<img->height(); y++){
			if(imgOverlay->atXY(x, y, 3) > 0){ // non-transparent
				float alpha = imgOverlay->atXY(x, y, 3)/255.0f;
				img->atXY(x, y, 0) = (alpha*imgOverlay->atXY(x, y, 0) + (1-alpha)*img->atXY(x, y, 0));
				img->atXY(x, y, 1) = (alpha*imgOverlay->atXY(x, y, 1) + (1-alpha)*img->atXY(x, y, 1));
				img->atXY(x, y, 2) = (alpha*imgOverlay->atXY(x, y, 2) + (1-alpha)*img->atXY(x, y, 2));
				img->atXY(x, y, 3) = 255;
			}
		}
	}
}

void ImageWriter::blurImage(double sigma)
{
	CImg<unsigned char> *img = (CImg<unsigned char> *)_img;
	img->blur(sigma, true, true);
}
void ImageWriter::sharpenImage(double amp)
{
	CImg<unsigned char> *img = (CImg<unsigned char> *)_img;
	img->sharpen(amp);
}

Color ImageWriter::getColorAt(unsigned int x, unsigned int y)
{
	CImg<unsigned char> *img = (CImg<unsigned char> *)_img;
	Color c;
	c.r = img->atXY(x, y, 0);
	c.g = img->atXY(x, y, 1);
	c.b = img->atXY(x, y, 2);
	return c;
}

