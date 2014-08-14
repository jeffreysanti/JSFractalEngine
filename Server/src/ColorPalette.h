/*
 * ColorPalette.h
 *
 *  Created on: Apr 27, 2014
 *      Author: jeffrey
 */

#ifndef COLORPALETTE_H_
#define COLORPALETTE_H_

#include <vector>
#include <iostream>
#include <cmath>
#include <string>
#include <sstream>
#include "ParamaterSchema.h"

struct Color{
	Color(){
		Color(0,0,0);
	}
	Color(int red, int green, int blue){
		r = red;
		g = green;
		b = blue;
	}
	unsigned int r,g,b;
};

class ColorPalette {
public:
	ColorPalette();
	virtual ~ColorPalette();

	bool isUsingMaxIterMethod();
	unsigned int colorCount();
	Color getColorOn1Scale(double val);
	Color getColorByIterations(unsigned int iters);

	void fillDefaultHSVPalette(double saturation=0.5, double value=0.5);

	bool loadPaletteFromParams(Paramaters &p, ParamaterSchema &schem, std::string prefix);

	static Color fromHSV(double, double s, double v);
	static Color fromParam(Paramaters &p, std::string prefix, Color def);
	static Color fromParam(std::string str, Color def);

	Color getBackgroundColor();

private:

	std::vector<Color> C;
	std::vector<int> I;
	std::string type;

	Color bg;

};

#endif /* COLORPALETTE_H_ */
