#include "BasicFile.h"
#include <iostream>
#include <fstream>

using namespace std;

int FileLength(char * filename)
{
    int TotalBytes;
    ifstream infile;

    infile.open(filename,ifstream::binary);
    if(!infile.is_open()) {
        return -1;
    }
    infile.seekg(0,ifstream::end);
    TotalBytes=infile.tellg();
    infile.seekg(0);
    infile.close();
    return TotalBytes;
}

int loadFile(unsigned char ** data, int * size, char * filename)
{
    int TotalBytes;
    ifstream infile;

    infile.open(filename,ifstream::binary);
    if(!infile.is_open()) {
        return -1;
    }
    infile.seekg(0,ifstream::end);
    TotalBytes=infile.tellg();

    infile.seekg(0);
    *data=new unsigned char[TotalBytes];

    infile.read((char*)*data,TotalBytes);
    infile.close();   
    *size = TotalBytes;
    return TotalBytes;
}



