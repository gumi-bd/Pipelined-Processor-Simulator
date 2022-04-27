/*
    Class Funcitonality:: Cache File
    Author :: Aditya Patil / CS20B004
*/

#include <bits/stdc++.h>
#include <climits>
#include <iomanip>

using namespace std;

typedef unsigned char Byte;
typedef unsigned int uint;	//using typedef to change names for easier use
typedef unsigned short usint;

class cache
{
    public:

    uint CacheSize = 256;        //Could be 256
	uint BlockSize = 4;         //Cache specifications

    ifstream * inpfile;
    vector<uint> Blocks;

    cache(ifstream * file)
    {
        inpfile=file;
        Blocks = vector<uint> (CacheSize,0);

        string code;

        uint val;
        int curr_block=0;
        int offset=0;

        while(*file >> code)
        {
            if(offset==4)               //Change Block
            {   
                curr_block++;           
                offset=0;               //reset offset
            }
            
            val=stoi(code,0,16);

            for(int i=0;i<offset;i++)
            {
                val=val<<8;
            }

            Blocks[curr_block] +=val;
            offset++;
        }
        
        file->close();
    }

    Byte read(Byte);
    uint readBlock(Byte);
    void write(Byte,Byte);
    void writeBlock(Byte,uint);
    void store(string);
}; 

Byte cache::read(Byte addr)
{
    uint data=readBlock(addr);
    uint offset=(addr&(BlockSize-1));

    for(int i=offset;i<3;i++)
    {
        data = data<<8;         //Select the 8 bits of data
    }

    for(int i=0;i<3;i++)
    {
        data = data>>8;         //Set the 8 bits of data as the least significant bits
    }

    return(Byte)data;
}

uint cache::readBlock(Byte addr)
{
    return(Blocks[addr>>2]);
}

void cache::write(Byte addr,Byte data)
{
    uint temp=(uint)data;
    Byte block=addr>>2;
    Byte offset=addr&3;
    uint mask = CacheSize-1;
    for(int i=0;i<(int)offset;i++)
    {
        mask=mask<<8;
        temp=temp<<8;
    }
    mask= UINT_MAX-mask;
    Blocks[block]=(Blocks[block]&mask)+temp;
}

void cache::writeBlock(Byte addr,uint data)
{
    Blocks[addr>>2]=data;
}

void cache::store(string filename)
{
    ofstream outfile;
    outfile.open(filename,ofstream::trunc);

    uint i;

    outfile<<setfill('0')<<setw(2)<<hex;

    for(i=0;i<CacheSize;i++)
    outfile<<setw(2)<<((int)read(i))<<endl;

    outfile.close();
}