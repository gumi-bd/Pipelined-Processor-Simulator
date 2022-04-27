
/*
    Class Functionality:: Pipelined Processor Simulator
    Author:: Aditya Patil and Karan Bardhan || CS20B004,CS20B036
*/

#include <bits/stdc++.h>
#include <climits>
#include <iomanip>
#include "cache.h"
#include "registers.h"

typedef unsigned char Byte;
typedef unsigned int uint;	//using typedef to change names for easier use
typedef unsigned short usint;

using namespace std;

class Processor
{
    private:
    cache* ICache;              //Instruction Cache
    cache* DCache;              //Data Cache
    regFile* RF;           //Register File

    bool haltSchedule=false;    //Halts till current pipelines stages are not over

    bool halt=false;            //Indicates Halted or not

    bool stall_IF = false;      //Stall this stage
	bool stall_ID = false;
	bool stall_EX = false;
	bool stall_MM = false;
	bool stall_WB = false;

    Byte REG_IF_PC = 0u;
    bool IF_run = true;
    void fetch_Stage();

    // ID pipeline registers here
	usint REG_ID_IR = 0u; // add R0 to R0 and store in R0
	Byte  REG_ID_PC = 0u;
	bool  ID_run = false;
	void decode_Stage();

	// EX pipeline registers here
	usint REG_EX_IR = 0u;
	Byte  REG_EX_PC = 0u;
	Byte  REG_EX_A  = 0u;
	Byte  REG_EX_B  = 0u;
	bool  EX_run = false;
	void execute_Stage();

	// MM pipeline registers here
	usint REG_MM_IR = 0u;
	Byte  REG_MM_AO = 0u;
    bool  REG_MM_COND = false;
	bool  MM_run = false;
	void memory_Stage();

	// WB pipeline registers here
	usint REG_WB_IR   = 0u;
	Byte  REG_WB_AO   = 0u;
	Byte  REG_WB_LMD  = 0u;
	bool  REG_WB_COND = false;
	bool  WB_run = false;
	void writeback_Stage();

	void flushPipeline();

    public:
	Processor(ifstream *iCache,ifstream *dCache,ifstream *registerfile)    //Initialise caches and register
	{
        ICache=new cache(iCache);
        DCache=new cache(dCache);
        RF=new regFile(registerfile);
    }

    ~Processor()
    {
        delete ICache;
        delete DCache;
        delete RF;
    }

	// initiates run, runs until halted
	void run();
	// run one cycle
	void cycle();

	// Checks if processor is in halted state?
	bool isHalted();

	// Stores data
	void store_data(std::string , std::string );

	// stats
	int instruction_count = 0;
	int instruction_count_arith = 0;
	int instruction_count_logic = 0;
	int instruction_count_data = 0;
	int instruction_count_control = 0;
	int instruction_count_halt = 0;
	int cycles = 0;
	int stalls = 0;
	int stalls_data = 0;
	int stalls_control = 0;
};


// runs until halted
void Processor::run()  {
	// while halt is not called execute instructions
	while(!isHalted())  {
		cycle();	//call the cycle function, the equivalent of one cycle of the processor
		}
	instruction_count = instruction_count_arith + instruction_count_control + 
                                     instruction_count_data + instruction_count_logic +
                                     instruction_count_halt;
}

void Processor::cycle()  {
	// to simulate parallelism we execute the pipeline stages
	// in the order WB MEM EX ID IF in one cycle,
	// so WB's buffer is cleared before MEM tries
	// to write to it
	cycles++;

	writeback_Stage();
	memory_Stage();
	execute_Stage();
	decode_Stage();
	fetch_Stage();
}

bool Processor::isHalted()  {
	return halt;
}

//Instruction Fetch stage
void Processor :: fetch_Stage(){
    if(!IF_run){
        return;
    }
    if(ID_run || stall_ID){
        IF_run = true;
		return;
    }
    REG_ID_IR = (((usint) ICache->read(REG_IF_PC)) << 8) + ((usint) ICache->read(REG_IF_PC+1));
	ID_run = true;

	// inc PC
	REG_IF_PC += 2;
}

//Instruction Decode Stage
void Processor ::decode_Stage(){
    if((!ID_run) || stall_ID)  {
		return;
	}

	if(EX_run || stall_EX)  {
		ID_run = true;
		return;
	}

	REG_EX_IR = REG_ID_IR;
	Byte opcode = (REG_ID_IR & 0xf000) >> 12;
	
	// halt class
	if(opcode == 15)  {
		IF_run = false;
		ID_run = false;
		EX_run = true;
		instruction_count_halt++;	//counting number of halt instructions
		// let the other pipeline stages (previous instructions) complete
		return;
	}
	ID_run = false;
	EX_run = true;

	// control class - HAZ
	if(opcode == 10)  {
		stall_ID = true;
		
		REG_EX_A = (REG_ID_IR & 0x0ff0) >> 4;
		REG_EX_PC = REG_IF_PC;
		stalls_control += 2;
        stalls+=2;
		return;
	}
	if(opcode == 11)  {
		stall_ID = true;

		if(RF->giveStatus((REG_ID_IR & 0x0f00) >> 8))
		{
			stalls_data++;
            stalls+=1;
			EX_run = false;
			ID_run = true;
            stall_ID = false; // RAW stalling is different
			return;
		}
		stalls_control += 2;
        stalls+=2;
		REG_EX_PC = REG_IF_PC;
		REG_EX_A = RF->read((REG_ID_IR & 0x0f00) >> 8);
		REG_EX_B = REG_ID_IR & 0x00ff;
		return;
	}
	// data class
	usint addr1;
	usint addr2;
	if(opcode == 9)  {
		addr1 = (REG_ID_IR & 0x00f0) >> 4;					//
		addr2 = (REG_ID_IR & 0x0f00) >> 8;					//
		if(RF->giveStatus(addr2))
		{
			stalls_data++;
            stalls+=1;
			EX_run = false;
			ID_run = true;
			return;
		}
		if(RF->giveStatus(addr1))
		{
			stalls_data++;
            stalls+=1;
			EX_run = false;
			ID_run = true;
			return;
		}

		REG_EX_A = RF->read((REG_ID_IR & 0x00f0) >> 4);
		REG_EX_B = REG_ID_IR & 0x000f;
		return;
	}

	if(opcode == 8)  {
		addr1 = (REG_ID_IR & 0x00f0) >> 4;
		if(RF->giveStatus(addr1))
		{
			stalls_data+=1;
            stalls+=1;
			EX_run = false;
			ID_run = true;
			return;
		}
		REG_EX_A = RF->read((REG_ID_IR & 0x00f0) >> 4);
		REG_EX_B = REG_ID_IR & 0x000f;

        RF->setStatus((Byte) ((REG_ID_IR & 0x0f00) >> 8), true);
		return;
	}

	// arithmetic class - HAZ
	if((opcode >= 0) && (opcode <= 2))  {
		addr1 = (REG_ID_IR & 0x00f0) >> 4;
		addr2 = REG_ID_IR & 0x000f;
		if(RF->giveStatus(addr1) || RF->giveStatus(addr2))
		{
			stalls_data++;
            stalls+=1;
			EX_run = false;
			ID_run = true;
			return;
		}

		REG_EX_A = RF->read((REG_ID_IR & 0x00f0) >> 4);
		REG_EX_B = RF->read(REG_ID_IR & 0x000f);
        RF->setStatus((Byte) ((REG_ID_IR & 0x0f00) >> 8), true);
		return;
	}
	if(opcode == 3)  {
		addr1 = (REG_ID_IR & 0x0f00) >> 8;
		if(RF->giveStatus(addr1))
		{
			stalls_data++;
            stalls+=1;
			EX_run = false;
			ID_run = true;
			return;
		}

		REG_EX_A = RF->read((REG_ID_IR & 0x0f00) >> 8);
        RF->setStatus((Byte) ((REG_ID_IR & 0x0f00) >> 8), true);
		return;
	}

	// logical class - HAZ
	if((opcode != 6))  {
		addr1 = (REG_ID_IR & 0x00f0) >> 4;
		addr2 = REG_ID_IR & 0x000f;
		if(RF->giveStatus(addr1) || RF->giveStatus(addr2))
		{
			stalls_data++;
            stalls+=1;
			EX_run = false;
			ID_run = true;
			return;
		}

		REG_EX_A = RF->read((REG_ID_IR & 0x00f0) >> 4);
		REG_EX_B = RF->read(REG_ID_IR & 0x000f);

        RF->setStatus((Byte) ((REG_ID_IR & 0x0f00) >> 8), true);
		return;
	}

	// here, opcode == OPC_NOT
	if(opcode == 6)	{
		addr1 = (REG_ID_IR & 0x00f0) >> 4;
        if(RF->giveStatus(addr1))
        {
        	stalls_data++;
            stalls+=1;
            EX_run = false;
            ID_run = true;
            return;
        }

		REG_EX_A = RF->read((REG_ID_IR & 0x00f0) >> 4);

        RF->setStatus((Byte) ((REG_ID_IR & 0x0f00) >> 8), true);
	}
}

//Execute Stage
void Processor::execute_Stage()
{
    if(!EX_run || stall_EX)
    {
        return;
    }

    int opcode = REG_EX_IR >> 12;

    switch(opcode)
    {
        case 0 :	REG_MM_AO = REG_EX_A + REG_EX_B;                //Addition
						instruction_count_arith++;	break;

		case 1 :	REG_MM_AO = REG_EX_A - REG_EX_B;
						instruction_count_arith++; break;      //Subtraction

		case 2 :	REG_MM_AO = REG_EX_A * REG_EX_B; 
						instruction_count_arith++; break;      //Multiplication

		case 3 :	REG_MM_AO = REG_EX_A + 1; 		 
						instruction_count_arith++; break;      //Increment

		case 4 :	REG_MM_AO = REG_EX_A & REG_EX_B; 
						instruction_count_logic++; break;      //Logical AND

		case 5 :	REG_MM_AO = REG_EX_A | REG_EX_B; 
						instruction_count_logic++; break;      //Logical OR

		case 6 :	REG_MM_AO = ~REG_EX_A;
						instruction_count_logic++; break;      //Logical Negation

		case 7 :	REG_MM_AO = REG_EX_A ^ REG_EX_B;
						instruction_count_logic++; break;      //Logical XOR

		case 8 :	REG_MM_AO = REG_EX_A + REG_EX_B;
						instruction_count_data++;	break;      //Load operation
						
		case 9 :	REG_MM_AO = REG_EX_A + REG_EX_B;
						instruction_count_data++;	break;      //Store Operation

		case 10:
			stall_EX = true;
			instruction_count_control++;	//counting the control instruction
			REG_MM_AO = REG_EX_PC + (Byte) ((usint) REG_EX_A << 1);
			break;

		case 11:
			stall_EX = true;
			instruction_count_control++;	//counting the control instruction
			if((int)REG_EX_A == 0)
			{
				REG_MM_AO =  REG_EX_PC +((usint) ( REG_EX_B << 1));
			}
			else
			{
				REG_MM_AO =  REG_EX_PC;
			}
			break;

		default:	break;
	}
	REG_MM_IR = REG_EX_IR;
	MM_run = true;	//marking that next instruction to be implemented is memmory 
	EX_run = false;	//setting that the stage is finished
} 
    
//Memory Management Stage
void Processor::memory_Stage(){
	if(!MM_run || stall_MM)
	{
		return;
	}
	MM_run = false;
	usint opCode = REG_MM_IR >> 12;
	Byte offset = (Byte) ((REG_MM_IR & 0x0f00) >> 8);
	if(opCode == 15)
	{
		WB_run = true;
		REG_WB_IR = REG_MM_IR;
		return;
	}
	else if((opCode == 10) || (opCode == 11))  {
		// if PC didn't change, carry on with decoding
		if(REG_IF_PC == REG_MM_AO)  {
			ID_run = false;
			stall_ID = false;
			stall_EX = false;
		// if it did, flush pipeline
		}  else  {
			REG_IF_PC = REG_MM_AO;
			flushPipeline();
		}
		WB_run = false;
		return;
	}
	
	else if(opCode == 9)
	{
		DCache->write(REG_MM_AO,RF->read((REG_MM_IR & 0x0f00) >> 8));
		MM_run = false;
        REG_WB_AO = 0u;
        REG_WB_IR = 0u;
        REG_WB_LMD = 0u;
        REG_WB_COND = false;
        WB_run = true;
		return;
	}
	else if(opCode == 8)
	{
		REG_WB_LMD = DCache->read(REG_MM_AO);
		REG_WB_IR = REG_MM_IR;
		REG_WB_AO = REG_MM_AO;
		WB_run = true;
	}  
	else  {
		MM_run = false;
        REG_WB_AO = REG_MM_AO;
        REG_WB_IR = REG_MM_IR;
        REG_WB_LMD = 0u;
        REG_WB_COND = REG_MM_COND;
        WB_run = true;
		return;
	}

	MM_run = false;
	return;
}

//Writeback Stage
void Processor::writeback_Stage(){
	if(!WB_run || stall_WB)
	{
		return;
	}
	usint opCode = REG_WB_IR >> 12;
	Byte offset = (Byte) ((REG_WB_IR & 0x0f00) >> 8);
	if(opCode == 15){
		WB_run = false;
		halt= true;
		return;
	}
	if(opCode == 8) 
	{
		RF->write(offset, REG_WB_LMD);
	}
	else 
	{
		RF->write(offset, REG_WB_AO);
	}
	RF->setStatus(offset, false);
	WB_run = false;
}

//Pipeline Flush
void Processor::flushPipeline()
{
    IF_run=true;
    ID_run=false;
    EX_run=false;
    MM_run=false;
    WB_run=false;

    REG_ID_IR = 0u;
	REG_ID_PC = 0u;
	REG_EX_A = 0u;
	REG_EX_B = 0u;
	REG_EX_IR = 0u;
	REG_EX_PC = 0u;
	REG_MM_AO = 0u;
	REG_MM_IR = 0u;
	REG_WB_AO = 0u;
	REG_WB_IR = 0u;
	REG_WB_COND = 0u;
	REG_WB_LMD = 0u;

    stall_IF = false;
	stall_ID = false;
	stall_EX = false;
	stall_MM = false;
	stall_WB = false;
}

//Store statistics of Processor in an output file
void Processor::store_data(string CacheFile,string OutputFile)
{
    DCache->store(CacheFile);

    ofstream outfile;

    outfile.open(OutputFile);

    outfile<<"Total Number of Instructions Executed: "<<instruction_count<<endl;

	outfile<<"Number of instructions in each class"<<endl;

    outfile<<"Arithmetic Instructions              : "<<instruction_count_arith<<endl;

    outfile<<"Logical Instructions                 : "<<instruction_count_logic<<endl;

    outfile<<"Data Instructions                    : "<<instruction_count_data<<endl;

    outfile<<"Control Instructions                 : "<<instruction_count_control<<endl;

    outfile<<"Halt Instructions                    : "<<instruction_count_halt<<endl;

	outfile<<"Cycles Per Instructions(CPI)         : "<<((double)cycles/instruction_count)<<endl;

    outfile<<"Total Number of Stalls               : "<<stalls<<endl;

    outfile<<"Data Stalls                          : "<<stalls_data<<endl;

    outfile<<"Control Stalls                       : "<<stalls_control<<endl;


    outfile.close();
}