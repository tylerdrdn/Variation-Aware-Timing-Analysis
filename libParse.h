#ifndef LIBPARSE_H_
#define LIBPARSE_H_

#include "types.h"

#define NOFMETALS 2

/****   Metal Related Variables   ****/
extern int sigma[NOFMETALS];
extern double resScaleFactor[NOFMETALS];
extern double capScaleFactor[NOFMETALS];

extern libNode *cellLib;

/**
 * parseLib
 *
 * Main function to parse the library information contained in file fileName.
 * Returns success or error code.
 *
 * @param fileName	-File path to cell library
 * @return result	-Success or Error Code
 */
extern int parseLib(char *fileName);

/**
 * findCell
 *
 * @param key
 * @param history
 *
 * @return result
 */
extern cell *findCell(libNode *libStruct, char *key, int *history);

#endif /* LIBPARSE_H_ */
