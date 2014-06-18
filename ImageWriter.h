/*
 * ImageWriter.h
 *
 *  Created on: Apr 26, 2014
 *      Author: jeffrey
 */

#ifndef IMAGEWRITER_H_
#define IMAGEWRITER_H_

#include <string>
#include "ColorPalette.h"

class ImageWriter {
public:
	ImageWriter(unsigned int w, unsigned int h);
	virtual ~ImageWriter();

	void saveFile(std::string flName);

	void fillImage(unsigned char r, unsigned char g, unsigned char b);

	unsigned int getWidth();
	unsigned int getHeight();
	void setPixel(unsigned int x, unsigned int y, unsigned char r, unsigned char g, unsigned char b);
	void setPixelHSV(unsigned int x, unsigned int y, double h, double s, double v);
	void setPixel(unsigned int x, unsigned int y, Color col);

	void drawCircle(unsigned int x, unsigned int y, unsigned int rad, Color col);
	void overlayImage(ImageWriter &writer, double opacity);

	void blurImage(double sigma);
	void sharpenImage(double amp);

	Color getColorAt(unsigned int x, unsigned int y);

private:
	void *_img;
};

#endif /* IMAGEWRITER_H_ */
