/*
 * FractalMandleJulia.cpp
 *
 *  Created on: Apr 29, 2014
 *      Author: jeffrey
 */

#include "FractalMandleJulia.h"

FractalMandleJulia::FractalMandleJulia(unsigned int id, Paramaters *p, Paramaters *paramsOut, ImageWriter *i)
						: Fractal(id, p, paramsOut, i), schem("SCHEMA_MULTIBROT") {
	I = NULL;
	histogram = NULL;
	algoCopy = -1;

	processParams();

}

FractalMandleJulia::~FractalMandleJulia() {
	if(I != NULL){
		for(unsigned int x=0; x<width; x++){
			delete [] I[x];
			I[x] = NULL;
		}
		delete [] I;
		I = NULL;
	}
	if(histogram != NULL){
		delete [] histogram;
		histogram = NULL;
	}
}

void FractalMandleJulia::render(int maxTime)
{
	if(!isOkay())
		return;

	Fractal::render(maxTime);

	passAlgoritm();
	passShade();
	passTrace();
	passEffect();
	passEvaluate();

	Fractal::postRender();
}

void FractalMandleJulia::processParams()
{
	Fractal::processParams();

	processParamsGraphics();
	processParamsAlgorithm();
	processParamsShading();
	processParamsTracing();

	Fractal::postProcessParams();

	if(schem.isErr()){
		err(schem.clearErrors());
		return;
	}
	try{
		I = new int*[width];
		for(int x=0; x<width; x++){
			I[x] = new int[height];
			for(int y=0; y<height; y++){
				I[x][y] = 0;
			}
		}
		histogram = new unsigned int[iters];
		for(int i=0; i<iters; i++){
			histogram[i] = 0;
		}
	}catch(std::bad_alloc &e){
		err("Out Of Memory!");
		return;
	}
}

inline std::complex<double> getComplexParam(std::string val, FractalMandleJulia* here){
	try{
		muc::Parser p;
		p.SetExpr(val);
		std::complex<double> z = p.Eval();
		return z;
	}catch (muc::Parser::exception_type &e)
	{
		here->err("Complex Value Invalid: "+e.GetMsg());
		return std::complex<double>(0,0); // if failed
	}
}

void FractalMandleJulia::processParamsAlgorithm()
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
			p->setValue("Kj", pCopy.getValue("Kj"));
			p->setValue("Kk", pCopy.getValue("Kk"));
			p->setValue("Kl", pCopy.getValue("Kl"));
			p->setValue("Km", pCopy.getValue("Km"));
			p->setValue("Kn", pCopy.getValue("Kn"));
			p->setValue("zInitial", pCopy.getValue("zInitial"));
		}else{
			err("algocopy job either does not exist or is inconsistent with specified image size!\n");
			return;
		}
	}

	funct = schem.getString(*p, "funct");
	threshold = schem.getDouble(*p, "threshold");

	if(schem.getString(*p, "zInitial") == "zero"){
		zeroZ = true;
	}else{
		zeroZ = false;
	}

	// constants
	Kj = getComplexParam(schem.getString(*p, "Kj"), this);
	Kk = getComplexParam(schem.getString(*p, "Kk"), this);
	Kl = getComplexParam(schem.getString(*p, "Kl"), this);
	Km = getComplexParam(schem.getString(*p, "Km"), this);
	Kn = getComplexParam(schem.getString(*p, "Kn"), this);


	double centerX = schem.getDouble(*p, "centR");
	double centerY = schem.getDouble(*p, "centI");

	double sizeX = schem.getDouble(*p, "radR");
	double sizeY = schem.getDouble(*p, "radI");

	if(width == 0 || height == 0)
		return;


	cornerX = (centerX - sizeX);
	cornerY = (centerY + sizeY);
	multX = sizeX*2/(double)width;
	multY = -1 * sizeY*2/(double)height;

	iters = schem.getInt(*p, "iters");

	applyTransformations(sizeX, sizeY, centerX, centerY);
}

void FractalMandleJulia::applyTransformations(double sizeX, double sizeY, double centerX, double centerY){
	// now handle transformations :)
	double transX = schem.getDouble(*p, "translateX");
	double transY = schem.getDouble(*p, "translateY");
	double scaleX = schem.getDouble(*p, "scaleX");
	double scaleY = schem.getDouble(*p, "scaleY");
	if(scaleX <= 0 || scaleY <= 0)
		return;

	sizeX *= 1.0/scaleX;
	sizeY *= 1.0/scaleY;

	multX = sizeX*2/(double)width;
	multY = -1 * sizeY*2/(double)height;

	cornerX += (transX*multX);
	cornerY += (transY*multY);

	centerX = cornerX + sizeX;
	centerY = cornerY - sizeY;

	p->setValue("radR", concat("",sizeX));
	p->setValue("radI", concat("",sizeY));
	p->setValue("centR", concat("",centerX));
	p->setValue("centI", concat("",centerY));
	p->setValue("translateX", "0");
	p->setValue("translateY", "0");
	p->setValue("scaleX", "1");
	p->setValue("scaleY", "1");
	p->writeToFile();
}

void FractalMandleJulia::processParamsGraphics()
{
	width = img->getWidth();
	height = img->getHeight();

	imgBlur = schem.getDouble(*p, "imgBlur");
	imgSharpen = schem.getDouble(*p, "imgSharpen");
}

void FractalMandleJulia::processParamsShading()
{
	shading = schem.getString(*p, "shading");
		if(shading != "none" && shading != "histogram" && shading != "linear" &&
				shading != "root2" && shading != "root3" && shading != "root4" &&
				shading != "log" && shading != "rankorder" && shading != "simplin")
			err("Error: Invalid shading multibrot parameter: " + shading + "!\n");

	double defSaturation = schem.getDouble(*p, "defColSat");
	double defValue = schem.getDouble(*p, "defColVal");

	bgColor = schem.getColor(*p, "bgColor");
	palette.fillDefaultHSVPalette(defSaturation, defValue);
	palette.loadPaletteFromParams(*p, schem, "fillColPal");
}

void FractalMandleJulia::processParamsTracing()
{
	tracingMethod = schem.getString(*p, "tracing");
	if(tracingMethod != "none" && tracingMethod != "blend" && tracingMethod != "solid")
		err("Error: Invalid tracing multibrot parameter: " + tracingMethod + "!\n");

	traceBrushRadius = schem.getInt(*p, "traceRadius");
	traceBlur = schem.getDouble(*p, "traceBlur");
	traceSolidColor = schem.getColor(*p, "traceCol");
	traceOpacity = schem.getDouble(*p, "traceOpacity");
}

void FractalMandleJulia::passAlgoritm()
{
	if(updateStatus("Algo 1/5", 0))
		return;

	if(algoCopy < 0){
		try{
			// establish function parser
			muc::Parser p;
			std::complex<double> z,c;
			p.DefineVar("z", &z);
			p.DefineVar("c", &c);

			// constants
			p.DefineVar("j", &Kj);
			p.DefineVar("k", &Kk);
			p.DefineVar("l", &Kl);
			p.DefineVar("m", &Km);
			p.DefineVar("n", &Kn);

			p.SetExpr(funct);

			for(unsigned int x=0; x<width; x++){
				for(unsigned int y=0; y<height; y++){

					// Simulate This Pixel

					c = std::complex<double>((double)x*multX + cornerX, (double)y*multY + cornerY);

					// inital z value
					if(zeroZ)
						std::complex<double>(0.0, 0.0);
					else
						z = c;

					int iteration = -1;

					// Iteration on [0, iters]
					while((int)iteration+1 < (int)iters && abs(z) < threshold){
						z = p.Eval();

						iteration ++;
					}
					if(iteration < 0)
						iteration = 0;
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
}


void FractalMandleJulia::passShade()
{
	if(updateStatus("Shade 2/5", 0))
			return;

	// Now Color It -- using equal distribution!
	if(shading != "none"){

		// set iterMin, iterMax values
		iterMin = 0; iterMax = 0;
		for(unsigned int i=0; i<iters; i++){
			if(histogram[i] > 0 && iterMin == 0)
				iterMin = i;
			if(histogram[i] > 0)
				iterMax = i;
		}

		if(palette.isUsingMaxIterMethod()){
			for(unsigned int x=0; x<width; x++){
				for(unsigned int y=0; y<height; y++){
					img->setPixel(x, y, getColor_Manual(I[x][y]));
				}
				if(updateStatus("Shade 2/5", (double)x/width * 100))
					return;
			}
		}else if(shading=="linear"){
			for(unsigned int x=0; x<width; x++){
				for(unsigned int y=0; y<height; y++){
					img->setPixel(x, y, getColor_Linear(I[x][y]));
				}
				if(updateStatus("Shade 2/5", (double)x/width * 100))
					return;
			}
		}else if(shading=="simplin"){
			for(unsigned int x=0; x<width; x++){
				for(unsigned int y=0; y<height; y++){
					img->setPixel(x, y, getColor_SimpleLinear(I[x][y]));
				}
				if(updateStatus("Shade 2/5", (double)x/width * 100))
					return;
			}
		}else if(shading=="root2"){
			for(unsigned int x=0; x<width; x++){
				for(unsigned int y=0; y<height; y++){
					img->setPixel(x, y, getColor_Root2(I[x][y]));
				}
				if(updateStatus("Shade 2/5", (double)x/width * 100))
					return;
			}
		}else if(shading=="root3"){
			for(unsigned int x=0; x<width; x++){
				for(unsigned int y=0; y<height; y++){
					img->setPixel(x, y, getColor_Root3(I[x][y]));
				}
				if(updateStatus("Shade 2/5", (double)x/width * 100))
					return;
			}
		}else if(shading=="root4"){
			for(unsigned int x=0; x<width; x++){
				for(unsigned int y=0; y<height; y++){
					img->setPixel(x, y, getColor_Root4(I[x][y]));
				}
				if(updateStatus("Shade 2/5", (double)x/width * 100))
					return;
			}
		}else if(shading=="log"){
			for(unsigned int x=0; x<width; x++){
				for(unsigned int y=0; y<height; y++){
					img->setPixel(x, y, getColor_Log(I[x][y]));
				}
				if(updateStatus("Shade 2/5", (double)x/width * 100))
					return;
			}
		}else if(shading=="rankorder"){
			// first sort histogram
			std::vector<RankOrder> rankOrder;
			for(int i=0; i<iters; i++){
				if(histogram[i] > 0){
					RankOrder ro;
					ro.iter = i;
					ro.count = histogram[i];
					rankOrder.push_back(ro);
				}
			}
			std::sort(rankOrder.begin(), rankOrder.end(), FractalMandleJulia::RankOrderSort);

			for(unsigned int x=0; x<width; x++){
				for(unsigned int y=0; y<height; y++){
					img->setPixel(x, y, getColor_RankOrder(I[x][y], rankOrder));
				}
				if(updateStatus("Shade 2/5", (double)x/width * 100))
					return;
			}
		}else{ // histogram
			for(unsigned int x=0; x<width; x++){
				for(unsigned int y=0; y<height; y++){
					img->setPixel(x, y, getColor_Histogram(I[x][y]));
				}
				if(updateStatus("Shade 2/5", (double)x/width * 100))
					return;
			}
		}


	}else{ // no shading interally
		img->fillImage(bgColor.r, bgColor.g, bgColor.b);
	}
}

void FractalMandleJulia::passTrace()
{
	if(updateStatus("Trace 2/5", 0))
		return;

	if(tracingMethod == "none")
		return;

	ImageWriter imgOverlay(width, height);

	for(unsigned int x=0; x<width; x++){
		for(unsigned int y=0; y<height; y++){
			Color iter1 = img->getColorAt(x,y);
			Color iter2 = atEdge(x,y);
			if(iter1.r != iter2.r || iter1.g != iter2.g || iter1.b != iter2.b){
				//draw tracing here
				if(tracingMethod == "blend" && shading != "none"){ // we know it is a solid shade
					Color colInterpolated;
					colInterpolated.r = linearInterpolate(iter1.r, iter2.r, 0.5);
					colInterpolated.g = linearInterpolate(iter1.g, iter2.g, 0.5);
					colInterpolated.b = linearInterpolate(iter1.b, iter2.b, 0.5);
					imgOverlay.drawCircle(x,y,traceBrushRadius, colInterpolated);
					//imgOverlay.setPixel(x, y, colInterpolated);
				}else if(tracingMethod == "solid"){
					imgOverlay.drawCircle(x,y,traceBrushRadius, traceSolidColor);
					//imgOverlay.setPixel(x, y, traceSolidColor);
				}
			}
		}
		if(updateStatus("Trace 3/5", (double)x/width * 100))
			return;
	}
	if(traceBlur > 0)
		imgOverlay.blurImage(traceBlur);
	img->overlayImage(imgOverlay, traceOpacity);
}

void FractalMandleJulia::passEffect()
{
	if(updateStatus("Effects 3/5", 0))
		return;

	if(imgBlur > 0){
		img->blurImage(imgBlur);
	}if(imgSharpen > 0){
		img->sharpenImage(imgSharpen);
	}

	unsigned long end = time(NULL);
}

void FractalMandleJulia::passEvaluate()
{
	if(updateStatus("Eval 5/5", 0))
		return;

	// how interesting is this?
	int uniqueIterCount = 0;
	int sum = 0;
	for(int i=0; i<iters; i++)
	{
		if(histogram[i] > 0)
			uniqueIterCount ++;
		sum += (i+1)*histogram[i];
	}
	pOut->setValue("uniqueSecs", concat("",uniqueIterCount));

	// get standard deviation
	double avg = sum / (width*height);
	pOut->setValue("avgIterCount", concat("",avg));
	double variance = 0;
	for(int i=0; i<iters; i++)
	{
		variance += histogram[i] * std::pow(avg - (i+1), 2);
	}
	variance = variance / (width * height);
	pOut->setValue("stdDeviation", concat("",std::sqrt(variance)));
}



