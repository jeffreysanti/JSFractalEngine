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
	C.clear();

	// Default palette filled with [0,320] hue values
	for(int hue=0; hue<320; hue++){
		Color col = fromHSV((double)hue/360.0, saturation, value);
		C.push_back(col);
	}
}

bool ColorPalette::loadPaletteFromParams(Paramaters &p, ParamaterSchema &schem, std::string prefix)
{
	int numOfColors = schem.getInt(p, prefix+"Count");
	if(numOfColors < 2) // need at least 2 colors
		return false;

	std::string tmp = schem.getString(p, "fillColPalType");
	if(tmp != "discrete" && tmp != "continuous" && tmp != "iterMax"){
		return false;
	}
	type = tmp;

	C.clear();
	I.clear();
	for(int i=1; i<= numOfColors; i++){
		Color col = schem.getColor(p, concat(prefix+"$", i));
		C.push_back(col);
		if(type == "iterMax"){
			int maxIter = schem.getInt(p, concat(prefix+"IterMax"+"$", i));
			I.push_back(maxIter);
		}
	}
	return true;
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

Color ColorPalette::fromParam(Paramaters &p, std::string prefix, Color def)
{
	std::string colorType = p.getValue(prefix+"Type", "rgb");
	if(colorType != "rgb" && colorType != "hsv"){
		std::cout << "Invalid Palette Color Type Parameter ("+prefix+"Type): " << colorType << "\n";
		return def;
	}
	if(colorType == "rgb"){
		int r = atoi(p.getValue(prefix+"R", "-1").c_str());
		int g = atoi(p.getValue(prefix+"G", "-1").c_str());
		int b = atoi(p.getValue(prefix+"B", "-1").c_str());
		if(r < 0 || g < 0 || b < 0){
			std::cout << "Invalid RGB Color Set For: " << prefix << "\n";
			return def;
		}
		def.r = r;
		def.g = g;
		def.b = b;
	}else{
		int h = atoi(p.getValue(prefix+"H", "-1").c_str());
		int s = atoi(p.getValue(prefix+"S", "-1").c_str());
		int v = atoi(p.getValue(prefix+"V", "-1").c_str());
		if(h < 0 || s < 0 || v < 0){
			std::cout << "Invalid HSV Color Set For: " << prefix << "\n";
			return def;
		}
		def = fromHSV(h/255.0, s/255.0, v/255.0);
	}
	return def;
}

Color ColorPalette::fromParam(std::string str, Color def)
{
	std::string colorType = str.substr(0, 3);
	str = str.substr(3);
	if(colorType != "RGB" && colorType != "HSV"){
		std::cerr << "Invalid Color Parameter " << str << "\n";
		return def;
	}
	if(colorType == "RGB"){
		int r = atoi(str.substr(0, str.find(',')).c_str());
		str = str.substr(str.find(',')+1);
		int g = atoi(str.substr(0, str.find(',')).c_str());
		str = str.substr(str.find(',')+1);
		int b = atoi(str.substr(0, str.find(',')).c_str());
		if(r < 0 || g < 0 || b < 0){
			std::cout << "Invalid RGB Color Set\n";
			return def;
		}
		def.r = r;
		def.g = g;
		def.b = b;
	}else{
		int h = atoi(str.substr(0, str.find(',')).c_str());
		str = str.substr(str.find(',')+1);
		int s = atoi(str.substr(0, str.find(',')).c_str());
		str = str.substr(str.find(',')+1);
		int v = atoi(str.substr(0, str.find(',')).c_str());
		if(h < 0 || s < 0 || v < 0){
			std::cout << "Invalid HSV Color Set\n";
			return def;
		}
		def = fromHSV(h/255.0, s/255.0, v/255.0);
	}
	return def;
}

