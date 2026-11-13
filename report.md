
# Resources:
- Computer Architecture: A Quantitative Approach 6th Edition
- The RISC-V Instruction Set Manual Volume I: User-Level ISA Document Version 2.2
- Online RISC-V simulator: https://riscv-simulator-five.vercel.app/


# Emulator implementation

## Not working
The pipeline for level6 (comp.bin) does not work. It prints some extra garbled charaters, I do not understand why.

## Implementation choices
- Dataforwarding is done using a global matrix N x M. N decides what a previous instruction type was, this can be either a normal operation (data can be collected from the previous command's execute stage) or a load operation (data has to be collected from the previous command's memory stage). M declares from where the data should be collected, either from the previous execute or memory stage. This configuration is computed in the decoder stage (InstructionDecodeStage::dataforward_detect), which basically remembers the type and the rd register of two previous registers and uses that information to define the configuration. When a dataforward has to come from a load, then a NOP is inserted. This has to be done because otherwise the register value is avaible yet when the later instruction is in the EX stage.
- I've used a mux to set the program counter from somewhere futher along in the pipeline. Also I''ve used a mux to flush the pipeline when a jump is performed.

# Testing
I have written a couple of configuration files to test some instructions (test/instruction/*.conf) and validated that the output matches the online RISCV-simulator. Furthermore I've written a simple script (scripts/test_application.sh) that runs the test on a level in (test/application/testdata/*.test). What I mostly did is run each level, look at what failed, maybe compare sequential and pipelined runs, and fix whatever went wrong. I did not write programms for all instructions, because would take me too much time.

