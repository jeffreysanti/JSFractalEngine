/*
 * FractalMultibrot.h
 *
 *  Created on: Apr 29, 2014
 *      Author: jeffrey
 */

#ifndef FRACTALMULTIBROT_H_
#define FRACTALMULTIBROT_H_

#include "FractalGen.h"
#include <complex>
#include <cmath>
#include <algorithm>
#include "mucParser.h"
#include "Fractal.h"

class FractalMultibrot : public Fractal {
public:
	FractalMultibrot(unsigned int id, Paramaters *p, Paramaters *paramsOut, ImageWriter *i);
	virtual ~FractalMultibrot();

	virtual void render(int maxTime);
	virtual void processParams();

protected:


	virtual void processParamsAlgorithm();
	virtual void processParamsGraphics();
	virtual void processParamsShading();
	virtual void processParamsTracing();

	virtual void passAlgoritm();
	virtual void passShade();
	virtual void passTrace();
	virtual void passEffect();
	virtual void passEvaluate();

	void applyTransformations(double sizeX, double sizeY, double centerX, double centerY);

	// graphic general settings (effects & bg)
	unsigned int width;
	unsigned int height;
	double imgBlur;
	double imgSharpen;
	Color bgColor;

	// algorithm parameters
	int algoCopy;
	unsigned int iters;
	double threshold;
	double cornerX;
	double cornerY;
	double multX;
	double multY;
	std::string funct;

	// shading
	std::string shading;
	ColorPalette palette;

	// shading util
	unsigned int iterMin, iterMax;

	// tracing
	std::string tracingMethod;
	unsigned int traceBrushRadius;
	Color traceSolidColor;
	double traceOpacity;
	double traceBlur;


	// algorithm trackers
	int **I; // For tracking iteration count of each pixel
	unsigned int *histogram; // for tracking distribution of each iteration count

	ParamaterSchema schem;


	inline Color atEdge(unsigned int x, unsigned int y){
		// Only check the top & left of the pixel
		int here = I[x][y];
		if(x == 0 || y == 0){ // obviously we are at edge & cannot check
			return img->getColorAt(x, y);
		}
		if(shading != "none"){
			if(I[x-1][y] != here)
				return img->getColorAt(x-1, y);
			if(I[x][y-1] != here)
				return img->getColorAt(x, y-1);
			if(I[x-1][y-1] != here)
				return img->getColorAt(x-1, y-1);
			return img->getColorAt(x, y);
		}else{
			if(I[x-1][y] != here)
				return Color(bgColor.r+25, 0,0); // some color other than bg
			if(I[x][y-1] != here)
				return Color(bgColor.r+25, 0,0); // some color other than bg
			if(I[x-1][y-1] != here)
				return Color(bgColor.r+25, 0,0); // some color other than bg
			return img->getColorAt(x, y);
		}
	}

	inline Color getColor_Histogram(int iteration)
	{
		unsigned int histTotal = width * height;
		double colNo=0;
		for(int i=0; i<iteration; i++){
			colNo += (double)histogram[i] / histTotal;
		}
		if(palette.isUsingMaxIterMethod())
			return palette.getColorByIterations(iteration);
		else
			return palette.getColorOn1Scale(colNo);
	}
	inline Color getColor_Linear(int iteration)
	{
		double val = (double)(iteration-iterMin) / double(iterMax - iterMin);
		return palette.getColorOn1Scale(val);
	}
	inline Color getColor_SimpleLinear(int iteration)
	{
		double val = (double)(iteration) / double(iters - 1);
		return palette.getColorOn1Scale(val);
	}
	inline Color getColor_Root2(int iteration)
	{
		double val = (std::sqrt((double)iteration) - std::sqrt((double)iterMin)) /
							(std::sqrt((double)iterMax) - std::sqrt((double)iterMin));
		return palette.getColorOn1Scale(val);
	}
	inline Color getColor_Root3(int iteration)
	{
		double val = (std::pow((double)iteration, (double)1/3) - std::pow((double)iterMin, (double)1/3)) /
					(std::pow((double)iterMax, (double)1/3) - std::pow((double)iterMin, (double)1/3));
		return palette.getColorOn1Scale(val);
	}
	inline Color getColor_Root4(int iteration)
	{
		double val = (std::pow((double)iteration, (double)1/4) - std::pow((double)iterMin, (double)1/4)) /
					(std::pow((double)iterMax, (double)1/4) - std::pow((double)iterMin, (double)1/4));
		return palette.getColorOn1Scale(val);
	}
	inline Color getColor_Log(int iteration)
	{
		double val = std::log((double)iteration/iterMin) /
							std::log((double)iterMax/iterMin);
		return palette.getColorOn1Scale(val);
	}

	struct RankOrder{
		unsigned int iter;
		unsigned int count;
	};
	static inline bool RankOrderSort(const RankOrder& first, const RankOrder &sec){
		return first.count < sec.count;
	}
	inline Color getColor_RankOrder(int iteration, std::vector<RankOrder> &V)
	{
		for(int i=0; i<V.size(); i++){
			if(V[i].iter == iteration){
				double val = i/((double)V.size()-1);
				return palette.getColorOn1Scale(val);
			}
		}
		return Color(0,0,0);
	}

	inline Color getColor_Manual(int iteration)
	{
		return palette.getColorByIterations(iteration);
	}

};

#endif /* FRACTALMULTIBROT_H_ */
