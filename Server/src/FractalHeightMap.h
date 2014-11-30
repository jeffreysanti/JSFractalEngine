/*
 * FractalHeightMap.h
 *
 *  Created on: Nov 26, 2014
 *      Author: jeffrey
 */

#ifndef FRACTALHEIGHTMAP_H_
#define FRACTALHEIGHTMAP_H_

#include "Fractal.h"

class FractalMeta;

class FractalHeightMap: public Fractal {
public:
	FractalHeightMap(unsigned int id, ParamsFile *p, ParamsFileNotSchema *paramsOut);
	virtual ~FractalHeightMap();

	virtual void render(int maxTime);
	virtual void processParams();

	static unsigned int returnArtifacts(FractalMeta &meta, char **dta);


private:

	void linearInterpolate(double minX, double minY, double wid, double hei, double cNW, double cNE, double cSE, double cSW, int iters);


	ImageWriter *img;
	double **I;

	unsigned int width;
	unsigned int height;
};

#endif /* FRACTALHEIGHTMAP_H_ */
