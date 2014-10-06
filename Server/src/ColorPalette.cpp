/*
 * ColorPalette.cpp
 *
 *  Created on: Apr 27, 2014
 *      Author: jeffrey
 */

#include "ColorPalette.h"
#include "FractalGen.h"

ColorPalette::ColorPalette() {
	fillDefaultHSVPalette();
}

ColorPalette::~ColorPalette() {
	// TODO Auto-generated destructor stub
}

unsigned int ColorPalette::colorCount()
{
	return C.size();
}

bool ColorPalette::isUsingMaxIterMethod()
{
	if(type == "iterMax")
		return true;
	return false;
}

Color ColorPalette::getColorByIterations(unsigned int iters)
{
	if(!isUsingMaxIterMethod()){
		std::cerr << "Error getColorByIterations called when method is not iterMax!\n";
		return Color(0,0,0);
	}
	int c = 0;
	Color ccur = C[0];
	while(true){
		if(c >= C.size())
			return C[C.size()-1];
		if(I[c] == iters)
			return C[c];
		if(I[c] > iters && I[c] != -1)
			return ccur;
		ccur = C[c];
		c ++;
	}
}

Color ColorPalette::getBackgroundColor(){
	return bg;
}


Color ColorPalette::getColorOn1Scale(double val)
{
	if(isUsingMaxIterMethod()){
		std::cerr << "Error getColorOn1Scale called when method is iterMax!\n";
		return Color(0,0,0);
	}
	if(val > 1 || val < 0){
		std::cerr << "Error in getColorOn1Scale: value: " << val << " not on [0,1]!\n";
		Color c;
		c.b = c.g = c.r = 0;
		return c;
	}
	if(type == "discrete"){
		unsigned int index = (float)(val * (colorCount()-1)+0.5f); // on [0, colorCount()-1]
		if(index < 0 || index >= C.size()){
			std::cerr << "Error in getColorOn1Scale: value: " << val << " gave color out of range!\n";
			std::cerr << "    There are " << colorCount() << " colors in palette.\n";
			Color c;
			c.b = c.g = c.r = 0;
			return c;
		}
		return C[index];
	}else{
		// find 2 closest colors, then interpolate
		// get segement # (there are colorCount()-1 segements)
		unsigned int segNum = (float)(val * (colorCount()-2)+0.5f); // on [0, colorCount()-2]
		unsigned int indexLow = segNum;
		unsigned int indexHigh = segNum + 1;
		double posAtHigh = indexHigh / (colorCount() - 1);

		// now interpolate
		Color c;
		c.r = linearInterpolate(C[indexHigh].r, C[indexLow].r, posAtHigh - val);
		c.g = linearInterpolate(C[indexHigh].g, C[indexLow].g, posAtHigh - val);
		c.b = linearInterpolate(C[indexHigh].b, C[indexLow].b, posAtHigh - val);
		return c;
	}
}

void ColorPalette::fillDefaultHSVPalette(double saturation, double value)
{
	bg = Color(0,0,0); // black bg

	C.clear();

	// Default palette filled with [0,320] hue values
	for(int hue=0; hue<320; hue++){
		Color col = fromHSV((double)hue/360.0, saturation, value);
		C.push_back(col);
	}
}

void ColorPalette::loadPaletteFromParams(Json::Value &colorArray, std::string paletteType)
{
	int numOfColors = colorArray.size();
	type = paletteType;

	C.clear();
	I.clear();
	for(int i=1; i< numOfColors; i++){
		Color col = fromParam(colorArray[i]["color"]);
		C.push_back(col);
		if(type == "iterMax"){
			int maxIter = colorArray[i]["maxIter"].asInt();
			I.push_back(maxIter);
		}
	}

	bg = fromParam(colorArray[0]["color"]);
}


Color ColorPalette::fromHSV(double h, double s, double v)
{
	// Thanks to : http://www.cs.rit.edu/~ncs/color/t_convert.html
	h = h * 360.0;

	double r, g, b;
	int i;
	float f, p, q, t;
	if( s == 0 ) {
		// achromatic (grey)
		r = g = b = v;
	}else{
		h /= 60;			// sector 0 to 5
		i = floor( h );
		f = h - i;			// factorial part of h
		p = v * ( 1 - s );
		q = v * ( 1 - s * f );
		t = v * ( 1 - s * ( 1 - f ) );
		switch( i ) {
			case 0:
				r = v;
				g = t;
				b = p;
				break;
			case 1:
				r = q;
				g = v;
				b = p;
				break;
			case 2:
				r = p;
				g = v;
				b = t;
				break;
			case 3:
				r = p;
				g = q;
				b = v;
				break;
			case 4:
				r = t;
				g = p;
				b = v;
				break;
			default:		// case 5:
				r = v;
				g = p;
				b = q;
				break;
		}
	}
	Color c;
	c.r = r * 255.0;
	c.g = g * 255.0;
	c.b = b * 255.0;
	return c;
}

Color ColorPalette::fromParam(Json::Value &val)
{
	int r = val[0].asInt();
	int g = val[1].asInt();
	int b = val[2].asInt();
	return Color(r, g, b);
}


