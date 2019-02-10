
#include <iostream>
#include <fstream>
#include <string>

#include "BasicFile.h"

using namespace std;

int FileLength(std::string * filename)
{
    int TotalBytes;
    ifstream infile;

    infile.open(filename->c_str(),ifstream::binary);
    if(!infile.is_open()) {
        return -1;
    }
    infile.seekg(0,ifstream::end);
    TotalBytes=infile.tellg();
    infile.seekg(0);
    infile.close();
    return TotalBytes;
}

int loadFile(unsigned char ** data, int * size, std::string * filename)
{
    int TotalBytes;
    std::ifstream infile;

    infile.open(filename->c_str(), fstream::in | fstream::binary);
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

int saveFile(unsigned char * data, int size, std::string * filename)
{
    ofstream ofile;
    ofile.open(filename->c_str(), ofstream::binary);
    if(!ofile.is_open())return -1;
    ofile.write((char*)data,size);
    ofile.close();
    return 0;   
}


