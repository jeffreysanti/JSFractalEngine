/*
 * FractalJulia.cpp
 *
 *  Created on: May 23, 2014
 *      Author: jeffrey
 */

#include "FractalJulia.h"

FractalJulia::FractalJulia(unsigned int id, Paramaters *p, Paramaters *paramsOut, ImageWriter *i)
	: FractalMultibrot(id, p, paramsOut, i) {
	// TODO Auto-generated constructor stub
	C.real(-2);
	C.imag(-3);

	processParams(); // now call the overriden methods
}

FractalJulia::~FractalJulia() {
	// TODO Auto-generated destructor stub
}

void FractalJulia::processParamsAlgorithm()
{
	algoCopy = schem.getInt(*p, "algocopy");
	if(algoCopy > -1){
		Paramaters pCopy;
		if(pCopy.loadFromFile(FractalGen::getSaveDir() + concat("/", algoCopy)+".job") &&
				pCopy.getValue("imgWidth")==p->getValue("imgWidth") &&
				pCopy.getValue("imgHeight") == p->getValue("imgHeight")){
			std::string algoFl = FractalGen::getSaveDir() + concat("/", algoCopy)+".algo";
			FILE *fp = fopen(algoFl.c_str(), "rb");
			if(fp == NULL){
				err("algocopy job is missing algo file!\n");
				return;
			}

			// Copy all elements
			p->setValue("funct", pCopy.getValue("funct"));
			p->setValue("threshold", pCopy.getValue("threshold"));
			p->setValue("centR", pCopy.getValue("centR"));
			p->setValue("centI", pCopy.getValue("centI"));
			p->setValue("radR", pCopy.getValue("radR"));
			p->setValue("radI", pCopy.getValue("radI"));
			p->setValue("iters", pCopy.getValue("iters"));
			p->setValue("Cr", pCopy.getValue("Cr"));
			p->setValue("Ci", pCopy.getValue("Ci"));
		}else{
			err("algocopy job either does not exist or is inconsistent with specified image size!\n");
			return;
		}
	}

	funct = schem.getString(*p, "funct");
	threshold = schem.getDouble(*p, "threshold");

	double centerX = schem.getDouble(*p, "centR");
	double centerY = schem.getDouble(*p, "centI");

	double cR = schem.getDouble(*p, "Cr");
	double cI = schem.getDouble(*p, "Ci");
	C.real(cR);
	C.imag(cI);

	if(width == 0 || height == 0)
		return;

	double sizeX = schem.getDouble(*p, "radR");
	double sizeY = schem.getDouble(*p, "radI");

	cornerX = (centerX - sizeX);
	cornerY = (centerY + sizeY);
	multX = sizeX*2/(double)width;
	multY = -1 * sizeY*2/(double)height;

	iters = schem.getInt(*p, "iters");

	applyTransformations(sizeX, sizeY, centerX, centerY);
}

void FractalJulia::passAlgoritm()
{
	if(updateStatus("Algo 1/5", 0))
		return;
	unsigned long start = time(NULL);

	if(algoCopy < 0){
		try{
			// establish function parser
			muc::Parser p;
			std::complex<double> z;
			p.DefineVar("z", &z);
			p.DefineVar("c", &C);
			p.SetExpr(funct);

			for(unsigned int x=0; x<width; x++){
				for(unsigned int y=0; y<height; y++){

					// Simulate This Pixel

					z = std::complex<double>((double)x*multX + cornerX, (double)y*multY + cornerY);
					int iteration = -1;

					// Iteration on [0, iters]
					while((int)iteration+1 < (int)iters && abs(z) < threshold){
						z = p.Eval();

						iteration ++;
					}
					I[x][y] = iteration;
					histogram[iteration] ++;
				}
				if(updateStatus("Algo 1/5", (double)x/width * 100))
					return;
			}
		}catch (muc::Parser::exception_type &e)
		{
			err(e.GetMsg());
			return;
		}
	}else{
		// Copy .algo file
		flogFile << "Copying ALGO FILE # " << algoCopy << ".\n";
		std::string algoFl = FractalGen::getSaveDir() + concat("/", algoCopy)+".algo";
		FILE *fp = fopen(algoFl.c_str(), "rb");
		for(int x=0; x<width; x++)
			fread(I[x], sizeof(int), height, fp);
		fread(histogram, sizeof(unsigned int), iters, fp);
		fclose(fp);
	}

	// now cache the algo data for faster changes in color/shading
	std::string algoFl = FractalGen::getSaveDir() + concat("/", getId())+".algo";
	FILE *fp = fopen(algoFl.c_str(), "wb");
	for(int x=0; x<width; x++)
		fwrite(I[x], sizeof(int), height, fp);
	fwrite(histogram, sizeof(unsigned int), iters, fp);
	fclose(fp);

	unsigned long end = time(NULL);
	pOut->setValue("timeAlgo", concat("", (end-start)));
}

