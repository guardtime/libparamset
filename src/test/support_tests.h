#ifndef SUPPORT_TESTS_H
#define	SUPPORT_TESTS_H

#include "cutest/CuTest.h"
#include <stdio.h>

#ifdef	__cplusplus
extern "C" {
#endif

void addSuite(CuSuite *suite, CuSuite* (*fn)(void));

void printStats(CuSuite *suite, const char *heding);

void initFullResourcePath(const char* rootDir);
const char *getFullResourcePath(const char* resource);

void writeXmlReport(CuSuite *suite, const char *fname);

#ifdef _WIN32
#define snprintf _snprintf
#endif


#ifdef	__cplusplus
}
#endif

#endif	/* SUPPORT_TESTS_H */

