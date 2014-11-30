/*
 * FractalHeightMap.cpp
 *
 *  Created on: Nov 26, 2014
 *      Author: jeffrey
 */

#include <random>

#include "FractalHeightMap.h"
#include "FractalGen.h"
#include "DBManager.h"

FractalHeightMap::FractalHeightMap(unsigned int id, ParamsFile *p, ParamsFileNotSchema *paramsOut)
				: Fractal(id, p, paramsOut) {
	/*I = NULL;
	histogram = NULL;
	algoCopy = -1;*/

	width = p->getJson()["type.heightmap"]["imgWidth"].asInt();
	height = p->getJson()["type.heightmap"]["imgHeight"].asInt();
	img = new ImageWriter(width, height);
}

FractalHeightMap::~FractalHeightMap()
{
	if(I != NULL){
		for(unsigned int x=0; x<width; x++){
			delete [] I[x];
			I[x] = NULL;
		}
		delete [] I;
		I = NULL;
	}

	SAFE_DELETE(img);
}


std::uniform_real_distribution<double> distribution(0.0,1.0);
std::default_random_engine generator;

inline int d2i(double val){
	return (int)(val + 0.5);
}

/*void FractalHeightMap::linearInterpolate(double minX, double minY, double maxX, double maxY, int iters)
{
	//std::cout << "LININT: " << minX << " " << minY << " " << maxX << " " << maxY << "\n";
	double lenx = maxX - minX;
	double leny = maxY - minY;
	if(lenx < 1 || leny < 1)
		return;

	double midX = minX + (lenx / 2);
	double midY = minY + (leny / 2);

	if(midX >= width)
		midX = width-1;

	if(midY >= height)
			midY = height-1;

	I[d2i(midX)][d2i(minY)] = (I[d2i(minX)][d2i(minY)] + I[d2i(maxX)][d2i(minY)]) / 2.0; // bottom
	I[d2i(midX)][d2i(maxY)] = (I[d2i(minX)][d2i(maxY)] + I[d2i(maxX)][d2i(maxY)]) / 2.0; // top
	I[d2i(minX)][d2i(midY)] = (I[d2i(minX)][d2i(minY)] + I[d2i(minX)][d2i(maxY)]) / 2.0; // left
	I[d2i(maxX)][d2i(midY)] = (I[d2i(maxX)][d2i(minY)] + I[d2i(maxX)][d2i(maxY)]) / 2.0; // right

	// now center
	double centerVal = (I[d2i(minX)][d2i(minY)] + I[d2i(maxX)][d2i(maxY)] + I[d2i(minX)][d2i(maxY)] +
						I[d2i(maxX)][d2i(minY)]) / 4.0;
	centerVal += 0.00;
	double rnd = 0.5-distribution(generator);
	double add = rnd / ((iters+1) * (iters+1));
	centerVal += add;

	I[d2i(midX)][d2i(midY)] = centerVal;


	// now subdivide
	linearInterpolate(minX, minY, midX, midY, iters+1); // bottom-left
	linearInterpolate(midX, midY, maxX, maxY, iters+1); // top-right
	linearInterpolate(minX, midY, midX, maxY, iters+1); // top-left
	linearInterpolate(midX, minY, maxX, midY, iters+1); // bottom-right
}*/

void FractalHeightMap::linearInterpolate(double minX, double minY, double wid, double hei, double cNW, double cNE, double cSE, double cSW, int iters)
{
	double nextHei = hei / 2.0;
	double nextWid = wid / 2.0;

	if(hei <= 1 || wid <= 1){
		// finished recursing -> draw
		double centerVal = cNW + cNE + cSW + cSE;
		centerVal = centerVal / 4.0;
		I[int(minX)][int(minY)] = centerVal;
		//std::cout << minX << ", " << minY << "\n";
		return;
	}

	// center
	double centerVal = (cNW + cNE + cSW + cSE) / 4.0;
	centerVal += 0.00;
	double Max = (nextHei+nextWid)/ (width+height) * 20;
	double rnd = 0.5-distribution(generator);
	double add = Max*rnd;// / (iters * iters);
	centerVal += add;

	// edges
	double eN = (cNW + cNE) / 2;
	double eS = (cSW + cSE) / 2;
	double eE = (cNE + cSE) / 2;
	double eW = (cNW + cSW) / 2;

	// recurse
	linearInterpolate(minX, minY+nextHei, nextWid, nextHei, eW, centerVal, eS, cSW, iters+1); // bottom-left
	linearInterpolate(minX+nextWid, minY, nextWid, nextHei, eN, cNE, eE, centerVal, iters+1); // top-right
	linearInterpolate(minX, minY, nextWid, nextHei, cNW, eN, centerVal, eW, iters+1); // top-left
	linearInterpolate(minX+nextWid, minY+nextHei, nextWid, nextHei, centerVal, eE, cSE, eS, iters+1); // bottom-right
}

void FractalHeightMap::render(int maxTime)
{
	/*if(!isOkay())
		return;

	Fractal::render(maxTime);

	passAlgoritm();
	passShade();
	passTrace();
	passEffect();
	passEvaluate();

	*/

	for(int x=0; x<width; x++){
		for(int y=0; y<height; y++){
			I[x][y] = 0;
		}
	}
	/*I[0][0] = 0.3;
	I[width-1][0] = 0.2;
	I[width-1][height-1] = 0.4;
	I[0][height-1] = 0.5;
*/

	linearInterpolate(0,0, width, height, 0.3, 0.2, 0.4, 0.5, 1);

	double min =  999999;
	double max = -999999;
	for(int x=0; x<width; x++){
		for(int y=0; y<height; y++){
			if(I[x][y] > max)
				max = I[x][y];
			if(I[x][y] < min)
				min = I[x][y];
		}
	}

	img->fillImage(0, 0, 0);
	for(int x=0; x<width; x++){
		for(int y=0; y<height; y++){
			double adjusted = (I[x][y] - min) / (max-min);
			int val = adjusted * 255.0;
			img->setPixel(x, y, Color(val, val, val));
		}
	}

	Fractal::postRender();

	if(isOkay()){
		img->saveFile(concat(FractalGen::getSaveDir()+"/", p->getJson()["internal"]["id"].asInt())+".png");
	}
}

void FractalHeightMap::processParams()
{
	Fractal::processParams();
	/*
	processParamsGraphics();
	processParamsAlgorithm();
	processParamsShading();
	processParamsTracing();
*/
	Fractal::postProcessParams();

	if(!isOkay())
			return;

	try{
		I = new double*[width];
		for(int x=0; x<width; x++){
			I[x] = new double[height];
			for(int y=0; y<height; y++){
				I[x][y] = 0;
			}
		}
	}catch(std::bad_alloc &e){
		err("Out Of Memory!");
		return;
	}
}

