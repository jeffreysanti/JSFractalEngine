/*
 * ArtifactDistributer.h
 *
 *  Created on: Nov 26, 2014
 *      Author: jeffrey
 */

#ifndef ARTIFACTDISTRIBUTER_H_
#define ARTIFACTDISTRIBUTER_H_

#include <cstdio>
#include <string>

class FractalMeta;


class ArtifactDistributer {
public:
	ArtifactDistributer();
	virtual ~ArtifactDistributer();

	FILE *getFractalThumbnail(FractalMeta &meta);
	unsigned int getArtifactData(FractalMeta &meta, char **dta);

	std::string getFractType(FractalMeta &meta);

	static ArtifactDistributer *getSingleton();

private:

	static ArtifactDistributer singleton;

};

#endif /* ARTIFACTDISTRIBUTER_H_ */
