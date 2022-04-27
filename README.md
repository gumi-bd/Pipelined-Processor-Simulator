# Pipelined-Processor-Simulator

This is a basic simulation of a 5-stage pipeline including IF,ID,EX,MEM,WB cycles.
It measures the number of instructions exuxuted, data stall cycles, control stall cycles and total CPI of the Instruction set.
Both DCache and ICache contains 256B with 4B block sizes and all Instructions are 16 bit instructions where the first 4 bits denote the operation that needs to be conducted
The register file contains 16 registers of 1 byte storage

To run the code:
1. Make sure all the header files and text files are in the same directory as the cpp file
2. The text files can be modified according to the necessary instructions and data that you want to run. Instructions go in ICache.txt and data goes in DCache.txt. Each entry is taken in hexadecimal value constisting of 1B. RF.txt contains details about the register file. The last ICache instuction must be f0 followed by all 00 entries to denote a halt
3. Run the cpp file
4. The final processor stats will be present in the newly formed Output text file and the final DCache values will be present in the ODCache text file
5. A debugger may be used to see the exact details of stalls and instruction cycles
