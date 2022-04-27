# Pipelined-Processor-Simulator

This is a basic simulation of a 5-stage pipeline including IF,ID,EX,MEM,WB cycles.
It measures the number of instructions exuxuted, data stall cycles, control stall cycles and total CPI of the Instruction set.
Both DCache and ICache contain 256 blocks of 4B each and all Instructions are 16 bit instructions where the first 4 bits denote the operation
The register file contains 16 registers of 1 byte storage

To run the code:
1. Make sure all the header files and text files are in the same directory as the cpp file
2. The text files can be modified according to the necessary instructions that you want to run
3. Run the cpp file
4. The final processor stats will be present in the newly formed Output text file and the final DCache values will be present in the ODCache text file
5. A debugger may be used to see the exact details of stalls and instruction cycles
