/*
 * FractalMandleJulia.cpp
 *
 *  Created on: Apr 29, 2014
 *      Author: jeffrey
 */

#include "FractalMandleJulia.h"

FractalMandleJulia::FractalMandleJulia(unsigned int id, ParamsFile *p, ParamsFileNotSchema *paramsOut)
						: Fractal(id, p, paramsOut) {
	I = NULL;
	histogram = NULL;
	algoCopy = -1;


	width = p->getJson()["type.juliamandle"]["imgWidth"].asInt();
	height = p->getJson()["type.juliamandle"]["imgHeight"].asInt();
	img = new ImageWriter(width, height);

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
	SAFE_DELETE(img);
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

	if(isOkay())
		img->saveFile(concat(FractalGen::getSaveDir()+"/", getId())+".png");

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

	if(!isOkay())
			return;

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
		here->err("Complex Value Invalid: "+e.GetMsg()+"\n");
		return std::complex<double>(0,0); // if failed
	}
}

void FractalMandleJulia::processParamsAlgorithm()
{
	algoCopy = p->getJson()["type.juliamandle"]["algocopy"].asInt();
	if(algoCopy > -1){
		std::string errTmp;
		ParamsFile pCopy(FractalGen::getSaveDir() + concat("/", algoCopy)+".job", true);
		if(pCopy.validate(errTmp) &&
				pCopy.getJson()["basic"]["imgWidth"].asInt()==p->getJson()["basic"]["imgWidth"].asInt() &&
				pCopy.getJson()["basic"]["imgHeight"].asInt()==p->getJson()["basic"]["imgHeight"].asInt()){
			std::string algoFl = FractalGen::getSaveDir() + concat("/", algoCopy)+".algo";
			FILE *fp = fopen(algoFl.c_str(), "rb");
			if(fp == NULL){
				err("algocopy job is missing algo file!\n");
				return;
			}

			// Copy all elements
			p->getJson()["type.juliamandle"]["func"] = pCopy.getJson()["type.juliamandle"]["func"];
			p->getJson()["type.juliamandle"]["threshold"] = pCopy.getJson()["type.juliamandle"]["threshold"];
			p->getJson()["type.juliamandle"]["centR"] = pCopy.getJson()["type.juliamandle"]["centR"];
			p->getJson()["type.juliamandle"]["centI"] = pCopy.getJson()["type.juliamandle"]["centI"];
			p->getJson()["type.juliamandle"]["radR"] = pCopy.getJson()["type.juliamandle"]["radR"];
			p->getJson()["type.juliamandle"]["radI"] = pCopy.getJson()["type.juliamandle"]["radI"];
			p->getJson()["type.juliamandle"]["iters"] = pCopy.getJson()["type.juliamandle"]["iters"];
			p->getJson()["type.juliamandle"]["Kj"] = pCopy.getJson()["type.juliamandle"]["Kj"];
			p->getJson()["type.juliamandle"]["Kk"] = pCopy.getJson()["type.juliamandle"]["Kk"];
			p->getJson()["type.juliamandle"]["Kl"] = pCopy.getJson()["type.juliamandle"]["Kl"];
			p->getJson()["type.juliamandle"]["Km"] = pCopy.getJson()["type.juliamandle"]["Km"];
			p->getJson()["type.juliamandle"]["Kn"] = pCopy.getJson()["type.juliamandle"]["Kn"];
			p->getJson()["type.juliamandle"]["zInitial"] = pCopy.getJson()["type.juliamandle"]["zInitial"];
		}else{
			err("algocopy job either does not exist or is inconsistent with specified image size!\n");
			return;
		}
	}

	funct = p->getJson()["type.juliamandle"]["func"].asString();
	threshold = p->getJson()["type.juliamandle"]["threshold"].asDouble();

	if(p->getJson()["type.juliamandle"]["zInitial"]["selected"].asString() == "zero"){
		zeroZ = true;
	}else{
		zeroZ = false;
	}

	// constants
	Kj = getComplexParam(p->getJson()["type.juliamandle"]["Kj"].asString(), this);
	Kk = getComplexParam(p->getJson()["type.juliamandle"]["Kk"].asString(), this);
	Kl = getComplexParam(p->getJson()["type.juliamandle"]["Kl"].asString(), this);
	Km = getComplexParam(p->getJson()["type.juliamandle"]["Km"].asString(), this);
	Kn = getComplexParam(p->getJson()["type.juliamandle"]["Kn"].asString(), this);


	double centerX = p->getJson()["type.juliamandle"]["centR"].asDouble();
	double centerY = p->getJson()["type.juliamandle"]["centI"].asDouble();

	double sizeX = p->getJson()["type.juliamandle"]["radR"].asDouble();
	double sizeY = p->getJson()["type.juliamandle"]["radI"].asDouble();

	if(width == 0 || height == 0)
		return;


	cornerX = (centerX - sizeX);
	cornerY = (centerY + sizeY);
	multX = sizeX*2/(double)width;
	multY = -1 * sizeY*2/(double)height;

	iters = p->getJson()["type.juliamandle"]["iters"].asInt();

	applyTransformations(sizeX, sizeY, centerX, centerY);
}

void FractalMandleJulia::applyTransformations(double sizeX, double sizeY, double centerX, double centerY){
	// now handle transformations :)
	double transX = p->getJson()["basic"]["translateX"].asDouble();
	double transY = p->getJson()["basic"]["translateY"].asDouble();
	double scaleX = p->getJson()["basic"]["scaleX"].asDouble();
	double scaleY = p->getJson()["basic"]["scaleY"].asDouble();
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

	p->getJson()["type.juliamandle"]["radR"] = sizeX;
	p->getJson()["type.juliamandle"]["radI"] = sizeY;
	p->getJson()["type.juliamandle"]["centR"] = centerX;
	p->getJson()["type.juliamandle"]["centI"] = centerY;
	p->getJson()["basic"]["translateX"] = 0;
	p->getJson()["basic"]["translateY"] = 0;
	p->getJson()["basic"]["scaleX"] = 1;
	p->getJson()["basic"]["scaleY"] = 1;
	// TODO: DO i need to save here? p->saveToFile()
}

void FractalMandleJulia::processParamsGraphics()
{
	width = img->getWidth();
	height = img->getHeight();

	imgBlur = p->getJson()["posteffects"]["imgBlur"].asDouble();
	imgSharpen = p->getJson()["posteffects"]["imgSharpen"].asDouble();
}

void FractalMandleJulia::processParamsShading()
{
	shading = p->getJson()["type.juliamandle"]["shading"]["selected"].asString();

	if(shading == "none"){
		bgColor = palette.fromParam(p->getJson()["jmshader.bg"]["bgColor"]);
	}else{
		palette.loadPaletteFromParams(p->getJson()["jmshader.normal"]["palette"],
				p->getJson()["jmshader.normal"]["fillColPalType"]["selected"].asString());
	}
}

void FractalMandleJulia::processParamsTracing()
{
	tracingMethod = p->getJson()["type.juliamandle"]["tracing"]["selected"].asString();

	if(tracingMethod != "none"){
		traceBrushRadius = p->getJson()["type.juliamandle"]["tracing"]["t"]["traceRadius"].asInt();
		traceBlur = p->getJson()["type.juliamandle"]["tracing"]["t"]["traceBlur"].asDouble();
		traceOpacity = p->getJson()["type.juliamandle"]["tracing"]["t"]["traceOpacity"].asDouble();
		if(tracingMethod == "solid"){
			traceSolidColor = palette.fromParam(p->getJson()["type.juliamandle"]["tracing"]["t"]["traceCol"]);
		}
	}
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

			threshold = threshold * threshold; // square it so roots are not needed for comparison

			for(unsigned int x=0; x<width; x++){
				for(unsigned int y=0; y<height; y++){

					// Simulate This Pixel

					c = std::complex<double>((double)x*multX + cornerX, (double)y*multY + cornerY);

					// inital z value
					if(zeroZ)
						z = std::complex<double>(0.0, 0.0);
					else
						z = c;

					int iteration = 1;

					// Iteration on [0, iters]
					while(norm(z) < threshold){
						if(iteration > iters)
							break;

						z = p.Eval();

						iteration ++;
					}
					if(iteration > iters){
						I[x][y] = iters;
					}else{
						I[x][y] = iteration - 1;
						histogram[iteration-1] ++;
					}
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
					if(I[x][y] == iters)
						img->setPixel(x, y, palette.getBackgroundColor());
					else
						img->setPixel(x, y, getColor_Manual(I[x][y]));
				}
				if(updateStatus("Shade 2/5", (double)x/width * 100))
					return;
			}
		}else if(shading=="linear"){
			for(unsigned int x=0; x<width; x++){
				for(unsigned int y=0; y<height; y++){
					if(I[x][y] == iters)
						img->setPixel(x, y, palette.getBackgroundColor());
					else
						img->setPixel(x, y, getColor_Linear(I[x][y]));
				}
				if(updateStatus("Shade 2/5", (double)x/width * 100))
					return;
			}
		}else if(shading=="simplin"){
			for(unsigned int x=0; x<width; x++){
				for(unsigned int y=0; y<height; y++){
					if(I[x][y] == iters)
						img->setPixel(x, y, palette.getBackgroundColor());
					else
						img->setPixel(x, y, getColor_SimpleLinear(I[x][y]));
				}
				if(updateStatus("Shade 2/5", (double)x/width * 100))
					return;
			}
		}else if(shading=="root2"){
			for(unsigned int x=0; x<width; x++){
				for(unsigned int y=0; y<height; y++){
					if(I[x][y] == iters)
						img->setPixel(x, y, palette.getBackgroundColor());
					else
						img->setPixel(x, y, getColor_Root2(I[x][y]));
				}
				if(updateStatus("Shade 2/5", (double)x/width * 100))
					return;
			}
		}else if(shading=="root3"){
			for(unsigned int x=0; x<width; x++){
				for(unsigned int y=0; y<height; y++){
					if(I[x][y] == iters)
						img->setPixel(x, y, palette.getBackgroundColor());
					else
						img->setPixel(x, y, getColor_Root3(I[x][y]));
				}
				if(updateStatus("Shade 2/5", (double)x/width * 100))
					return;
			}
		}else if(shading=="root4"){
			for(unsigned int x=0; x<width; x++){
				for(unsigned int y=0; y<height; y++){
					if(I[x][y] == iters)
						img->setPixel(x, y, palette.getBackgroundColor());
					else
						img->setPixel(x, y, getColor_Root4(I[x][y]));
				}
				if(updateStatus("Shade 2/5", (double)x/width * 100))
					return;
			}
		}else if(shading=="log"){
			for(unsigned int x=0; x<width; x++){
				for(unsigned int y=0; y<height; y++){
					if(I[x][y] == iters)
						img->setPixel(x, y, palette.getBackgroundColor());
					else
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
					if(I[x][y] == iters)
						img->setPixel(x, y, palette.getBackgroundColor());
					else
						img->setPixel(x, y, getColor_RankOrder(I[x][y], rankOrder));
				}
				if(updateStatus("Shade 2/5", (double)x/width * 100))
					return;
			}
		}else{ // histogram
			for(unsigned int x=0; x<width; x++){
				for(unsigned int y=0; y<height; y++){
					if(I[x][y] == iters)
						img->setPixel(x, y, palette.getBackgroundColor());
					else
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
	pOut->getJson()["uniqueSecs"] = uniqueIterCount;

	// get standard deviation
	double avg = sum / (width*height);
	int hmax = 0;
	pOut->getJson()["avgIterCount"] = avg;
	double variance = 0;
	for(int i=0; i<iters; i++)
	{
		variance += histogram[i] * std::pow(avg - (i+1), 2);
		if(histogram[i] > hmax)
			hmax = histogram[i];
	}
	variance = variance / (width * height);
	pOut->getJson()["stdDeviation"] = std::sqrt(variance);

	// populate charts
	Json::Value graphs = Json::Value(Json::arrayValue);

	Json::Value histo = Json::Value(Json::objectValue);
	histo["title"] = "Histogram";
	histo["xaxis"] = "Iteration Count";
	histo["yaxis"] = "Pixel Count";
	histo["xmin"] = 1;
	histo["xmax"] = iters;
	histo["ymin"] = 0;
	histo["ymax"] = hmax;
	histo["trace"] = Json::Value(Json::arrayValue);
	for(int i=0; i<iters; i++)
	{
		Json::Value pt = Json::Value(Json::arrayValue);
		pt[0] = i+1;
		pt[1] = histogram[i];
		histo["trace"][i] = pt;
	}

	graphs[0] = histo;
	pOut->getJson()["graphs"] = graphs;
}

unsigned int FractalMandleJulia::returnArtifacts(FractalMeta &meta, char **dta)
{
	unsigned int len = 4;
	unsigned int artificatCount = 0;

	std::string imgPath = concat(DirectoryManager::getSingleton()->getRootDirectory()+"renders/", meta.jobID) + ".png";
	FILE *fp = fopen(imgPath.c_str(), "rb");
	if(fp == NULL){ // no image
		*dta = new char[len];
		char *ptr = *dta;

		artificatCount = htonl(artificatCount);
		memcpy(ptr, &artificatCount, 4);
		ptr += 4;
	}else{
		artificatCount = 1;
		len += 8;

		fseek(fp, 0L, SEEK_END);
		int dtaSz = ftell(fp);
		fseek(fp, 0L, SEEK_SET);
		len += dtaSz;
		*dta = new char[len];
		char *ptr = *dta;

		// now copy data
		artificatCount = htonl(artificatCount);
		memcpy(ptr, &artificatCount, 4);
		ptr += 4;

		// image artifact
		memcpy(ptr, "IMGS", 4); // image, scalable
		ptr += 4;

		unsigned int reportedSize = htonl(dtaSz);
		memcpy(ptr, &reportedSize, 4);
		ptr += 4;

		for(int i=0; i<dtaSz; i++){
			char byte;
			fread(&byte, 1, 1, fp);
			*ptr = byte;
			ptr ++;
		}
		fclose(fp);
	}
	return len;
}



