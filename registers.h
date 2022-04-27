
/*
    Register File
    Author : Karan Bardhan
*/

#include <bits/stdc++.h>
#include <climits>
#include <iomanip>

using namespace std;

typedef unsigned char Byte;
typedef unsigned int uint;	//using typedef to change names for easier use
typedef unsigned short usint;

class regFile{
    //register file has 16 registers
    int regSize = 16;

	// source file stream
	std::ifstream * srcFile;
    //register file containing the registers
    vector<Byte> regF;
    //array containing the status of each register(busy = 1/ free =)
    vector<bool> regStat;
    
    public:
        // init regfile from a file
	    regFile(std::ifstream * fp);
        Byte read(Byte index);
        void write(Byte index, Byte value);
        bool giveStatus(Byte index);
        void setStatus(Byte index, bool currStatus);
};

regFile::regFile(std::ifstream * fp){
	this->srcFile = fp;
	regF = std::vector<Byte> (regSize);
	regStat = std::vector<bool> (regSize,false);

    // read file into regfile
	std::string hexCode;
	Byte value;
	int regNum = 0;
	while(*fp >> hexCode){
		value = std::stoi(hexCode,0,16);
		regF[regNum] = value;
		regNum++;
	}
    fp->close();
};

//read/write functions
Byte regFile :: read(Byte index){
    return regF[index];
}

void regFile :: write(Byte index, Byte value){
    if(index != 0) regF[index] = value;
}

//status functions
bool regFile :: giveStatus(Byte index){
    return regStat[index];
}

void regFile :: setStatus(Byte index, bool currStatus){
    regStat[index] = currStatus;
}