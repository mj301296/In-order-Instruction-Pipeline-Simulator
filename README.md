# APEX Pipeline Simulator v2.0
A template for 5 Stage APEX In-order Pipeline

## Notes:

 - This code is a simple implementation template of a working 5-Stage APEX In-order Pipeline
 - Implementation is in `C` language
 - Stages: Fetch -> Decode -> Execute -> Memory -> Writeback
 - The execute stage is divided into three parellel processing stages: Integer, Multiplier and Load-Store
 - All the stages except Multiplier have latency of one cycle. Multiplier has a latency of 3 cycles
 - Logic to check data dependencies has not be included
 - Includes logic for `ADD`, `ADDL`, `SUB`, `SUBL`, `MUL`, `DIV`, `LOAD`, `STORE`, `LDR`,  `STR`, `CMP`, `NOP` and`HALT` instructions
 - On fetching `HALT` instruction, fetch stage stop fetching new instructions
 - When `HALT` instruction is in commit stage, simulation stops

## Files:

 - `Makefile`
 - `file_parser.c` - Functions to parse input file
 - `apex_cpu.h` - Data structures declarations
 - `apex_cpu.c` - Implementation of APEX cpu
 - `apex_macros.h` - Macros used in the implementation
 - `main.c` - Main function which calls APEX CPU interface
 - `input.asm` - Sample input file

## How to compile and run

 Go to terminal, `cd` into project directory and type:
```
 make
```
 Run as follows:
```
 ./apex_sim <input_file_name>
 
```
## Simulator functions:

 - `MakefileTo display final register and data memory values`<br>
 ./apex_sim <input_file.asm> simulate
 - `To display final register and data memory values along with each clock cycle information`<br>
 ./apex_sim <input_file.asm> display
 - 'To display final register and data memory values upto particular cycle number'<br>
 ./apex_sim <input_file.asm> simulate <no of cycles>
 - 'To perform processing of one cycle as a single step'<br>  
 ./apex_sim <input_file.asm> single_step
 - 'To display data stored at a particular location'<br>
 ./apex_sim <input_file.asm> show_mem <memory_position>

