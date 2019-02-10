

#ifndef _BASIC_FILE_H_
#define _BASIC_FILE_H_

using namespace std;

int loadFile(unsigned char ** data, int * size, std::string * filename);
int saveFile(unsigned char * data, int size, std::string * filename);
int FileLength(std::string * filename);

#endif
