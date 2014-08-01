/*
 * FractalJulia.h
 *
 *  Created on: May 23, 2014
 *      Author: jeffrey
 */

#ifndef FRACTALJULIA_H_
#define FRACTALJULIA_H_

#include "FractalMultibrot.h"

class FractalJulia: public FractalMultibrot {
public:
	FractalJulia(unsigned int id, Paramaters *p, Paramaters *paramsOut, ImageWriter *i);
	virtual ~FractalJulia();

protected:

	virtual void processParamsAlgorithm();
	virtual void passAlgoritm();


	std::complex<double> C;

};

#endif /* FRACTALJULIA_H_ */
