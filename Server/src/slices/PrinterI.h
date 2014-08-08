/*
 * PrinterI.h
 *
 *  Created on: Aug 3, 2014
 *      Author: jeffrey
 */

#ifndef PRINTERI_H_
#define PRINTERI_H_

#include "Printer.h"

namespace Demo {

class PrinterI: public Printer {
public:
	virtual void printString(const std::string& s, const Ice::Current&);

private:
	int i=0;
};

} /* namespace Demo */

#endif /* PRINTERI_H_ */
