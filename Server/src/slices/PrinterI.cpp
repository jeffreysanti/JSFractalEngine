/*
 * PrinterI.cpp
 *
 *  Created on: Aug 3, 2014
 *      Author: jeffrey
 */

#include "PrinterI.h"

namespace Demo {

void PrinterI::printString(const std::string& s, const Ice::Current& current)
{
	i++;
    std::cout << s << " # " << (long)current.con._ptr << std::endl;
}


} /* namespace Demo */
