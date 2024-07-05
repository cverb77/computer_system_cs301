#ifndef __PROJECT1_CPP__
#define __PROJECT1_CPP__

#include "project1.h"
#include <vector>
#include <string>
#include <unordered_map>
#include <iostream>
#include <sstream>
#include <fstream>

int main(int argc, char* argv[]) {
    if (argc < 4) // Checks that at least 3 arguments are given in command line
    {
        std::cerr << "Expected Usage:\n ./assemble infile1.asm infile2.asm ... infilek.asm staticmem_outfile.bin instructions_outfile.bin\n" << std::endl;
        exit(1);
    }
    //Prepare output files
    std::ofstream inst_outfile, static_outfile;
    static_outfile.open(argv[argc - 2], std::ios::binary);
    inst_outfile.open(argv[argc - 1], std::ios::binary);
    std::vector<std::string> instructions;

    /**
     * Phase 1:
     * Read all instructions, clean them of comments and whitespace DONE
     * TODO: Determine the numbers for all static memory labels
     * (measured in bytes starting at 0)
     * TODO: Determine the line numbers of all instruction line labels
     * (measured in instructions) starting at 0
    */
    // Initialize the label containers
     
    std::unordered_map<std::string, int> staticMemoryLabels;
    std::unordered_map<std::string, int> instructionLabels;
    int staticMemoryAddress = 0; // Starting address for static memory labels
    int instructionLineCount = 0; // Counter for instruction lines
    bool processStaticMemory = false; // Start with processing static memory

    std::vector<std::string> StaticValue; // Vector to store all values from .word directives
    //For each input file:
    for (int i = 1; i < argc - 2; i++) 
    {
        std::ifstream infile(argv[i]); //  open the input file for reading
        if (!infile) 
        { // if file can't be opened, need to let the user know
            std::cerr << "Error: could not open file: " << argv[i] << std::endl;
            exit(1);
        }

        std::string str;
        while (getline(infile, str))
        {
            str = clean(str); // remove comments, leading and trailing whitespace
            if (str == "") 
            { //Ignore empty lines
                continue;
            }
            instructions.push_back(str); // TODO This will need to change for labels
            if (str[0] == '.') 
            {
                continue; // Skip any line starts with "."
            }
            
            size_t wordPos = str.find(".word");
            if (wordPos != std::string::npos) 
            {
                processStaticMemory = true; // Switch to processing static memory when ".word" has been found 
                
            }
            
            if (str[0] != '.' && wordPos == std::string::npos) 
            {
                processStaticMemory = false; // Switch to processing instruction
            }

            if (!processStaticMemory) 
            {   
                // Process instruction labels and instructions
                size_t colonPos = str.find(':');
                if (colonPos != std::string::npos) {
                    std::string label = str.substr(0, colonPos);
                    instructionLabels[label] = instructionLineCount;
                    instructionLineCount--;
                }
                instructionLineCount++;
                continue;
            }
            else
            {
                // Process static memory labels
                size_t colonPos = str.find(':');
                if (colonPos != std::string::npos) 
                {
                    std::string label = str.substr(0, colonPos);
                    std::string valuesStr = str.substr(wordPos + 6); // +6 to skip ":.word" 
                    std::vector<std::string> values = split(valuesStr, " ");
                    // staticMemoryLabels[label] = staticMemoryAddress;
                    // Append the _END_OF_STATIC_MEMORY_ label
                    staticMemoryLabels["_END_OF_STATIC_MEMORY_"] = staticMemoryAddress;
                    // Check each value in values and append each value to the staticValue vector
                    for (const std::string& value : values) 
                    {
                        // Check if the value exists as a key in instructionLabels
                        auto it = instructionLabels.find(value);
                        if (it != instructionLabels.end()) 
                        {
                            // If found, use the associated value, multiply by 4, and add to the static memory address
                            staticMemoryAddress += it->second * 4;
                            
                        } else 
                        {
                            // If not found, treat it as an integer
                            staticMemoryAddress += 4; // each integer value occupies 4 bytes
            
                        }
                        StaticValue.push_back(value); 
                    }
                    continue; 
                }  
                 
            }
            infile.close();
            }
    }

        /** Phase 2
         * Process all static memory, output to static memory file
         * TODO: All of this
         */     
        for(std::string values : StaticValue)
            {
                auto found = instructionLabels.find(values); 

                if(found != instructionLabels.end()) //if we find the label in the static memory table, calculate the correct address and write to binary
                {
                    int write = found->second * 4;
                    write_binary(write, static_outfile);
                
                }
                else //else, assume int and write to memory
                {
                    int write = std::stoi(values);
                    write_binary(write, static_outfile);
                }   
            }

    /** Phase 3 Almost Done!
     * Process all instructions, output to instruction memory file
     * TODO: Almost all of this, it only works for adds
     */
    int count = 0;
    
    for(std::string inst : instructions) {
        std::vector<std::string> terms = split(inst, WHITESPACE+",()");
        std::string inst_type = terms[0];
        if (inst_type == "add") { //code to write the add instructions to memory in binary
            int result = encode_Rtype(0,registers[terms[2]], registers[terms[3]], registers[terms[1]], 0, 32);
            write_binary(encode_Rtype(0,registers[terms[2]], registers[terms[3]], registers[terms[1]], 0, 32),inst_outfile);
        }
        if (inst_type == "sub"){ //code to write the sub instructions to memory in binary
            int result = encode_Rtype(0,registers[terms[2]], registers[terms[3]], registers[terms[1]], 0, 34);
            write_binary(encode_Rtype(0,registers[terms[2]], registers[terms[3]], registers[terms[1]], 0, 34),inst_outfile);
        }
        if (inst_type == "div"){ //code to write the div instructions to memory in binary
            int result = encode_Rtype(0,registers[terms[1]],registers[terms[2]],0,0,26);
            write_binary(encode_Rtype(0,registers[terms[1]],registers[terms[2]],0,0,26),inst_outfile);
        }
        if (inst_type == "mult"){ //code to write the mult instructions to memory in binary
            int result = encode_Rtype(0,registers[terms[1]],registers[terms[2]],0,0,24);
            write_binary(encode_Rtype(0,registers[terms[1]],registers[terms[2]],0,0,24),inst_outfile);     
        }
        if(inst_type == "mflo"){ //code to write the mflo instructions to memory in binary
            int result = encode_Rtype(0,0,0,registers[terms[1]],0,18);
            write_binary(encode_Rtype(0,0,0,registers[terms[1]],0,18),inst_outfile);
        }
        if(inst_type == "mfhi"){ //code to write the mfhi instructions to memory in binary
            int result = encode_Rtype(0,0,0,registers[terms[1]],0,16);
            write_binary(encode_Rtype(0,0,0,registers[terms[1]],0,16),inst_outfile);
        }
        if(inst_type == "sll"){ //code to write the sll instructions to memory in binary
            int result = encode_Rtype(0,0,registers[terms[2]],registers[terms[1]], std::stoi(terms[3]),0);
            write_binary(encode_Rtype(0,0,registers[terms[2]],registers[terms[1]], std::stoi(terms[3]),0),inst_outfile);
        }
        if(inst_type == "srl"){ //code to write the srl instructions to memory in binary
            int result = encode_Rtype(0,0,registers[terms[2]],registers[terms[1]], std::stoi(terms[3]),2);
            write_binary(encode_Rtype(0,0,registers[terms[2]],registers[terms[1]], std::stoi(terms[3]),2),inst_outfile);
        }
        if(inst_type == "slt"){ //code to write the slt instructions to memory in binary
            int result = encode_Rtype(0,registers[terms[2]],registers[terms[3]],registers[terms[1]],0,42);
            write_binary(encode_Rtype(0,registers[terms[2]],registers[terms[3]],registers[terms[1]],0,42),inst_outfile);  
        }
        if(inst_type == "syscall"){ //code to write the syscall instructions to memory in binary
            int result = encode_Rtype(0,0,0,26,0,12);
            write_binary(encode_Rtype(0,0,0,26,0,12),inst_outfile);
        }
        if(inst_type == "addi"){ //code to write the addi instructions to memory in binary
            int result = encode_Itype(8,registers[terms[2]],registers[terms[1]],std::stoi(terms[3]));
            write_binary(encode_Itype(8,registers[terms[2]],registers[terms[1]],std::stoi(terms[3])),inst_outfile);
        }
        if(inst_type == "sw"){ //code to write the sw instructions to memory in binary
            int result = encode_Itype(43,registers[terms[3]], registers[terms[1]], std::stoi(terms[2]));
            write_binary(encode_Itype(43,registers[terms[3]], registers[terms[1]], std::stoi(terms[2])),inst_outfile);
        }
        if(inst_type == "lw"){ //code to write the lw instructions to memory in binary
            int result = encode_Itype(35,registers[terms[3]],registers[terms[1]], std::stoi(terms[2]));
            write_binary(encode_Itype(35,registers[terms[3]],registers[terms[1]], std::stoi(terms[2])),inst_outfile);
        }
        if(inst_type == "beq"){ //code to write the beq instructions to memory in binary
            //term is adjusted in accordance with the formula on the guidlines sheet. 
            std::string constant = (terms[3]); 
            int term = labelExist(constant, instructionLabels);
            term = count + 1 - term;
            term = term * -1;

            int result = encode_Itype(4,registers[terms[1]],registers[terms[2]],term);
            write_binary(encode_Itype(4,registers[terms[1]],registers[terms[2]], term),inst_outfile);
         }
        if(inst_type == "bne"){ //code to write the bne instructions to memory in binary
            //term is adjusted in accordance with the formula on the guidlines sheet. 
            std::string constant = (terms[3]); 
            int term = labelExist(constant, instructionLabels);
            term = count + 1 - term;
            term = term * -1; 
            int result = encode_Itype(5,registers[terms[1]],registers[terms[2]], term);
            write_binary(encode_Itype(5,registers[terms[1]],registers[terms[2]], term),inst_outfile);
        }
        if(inst_type == "j"){ //code to write the j instructions to memory in binary
            std::string constant = (terms[1]); 
            int term = labelExist(constant, instructionLabels);
            int result = encode_Jtype(2, term);
            write_binary(encode_Jtype(2, term), inst_outfile);
        }
        if(inst_type == "jal"){ //code to write the jal instructions to memory in binary
            std::string constant = (terms[1]); 
            int term = labelExist(constant, instructionLabels);
            
            int result = encode_Jtype(3, term);
            write_binary(encode_Jtype(3, term), inst_outfile);
        }
        if(inst_type == "jr"){ //code to write the jr instructions to memory in binary
            int result = encode_Rtype(0,registers[terms[1]],0,0,0,8);
            write_binary(encode_Rtype(0,registers[terms[1]],0,0,0,8), inst_outfile);
        }
        if(inst_type == "la"){ //code to write the la instructions to memory in binary
            std::string constant = (terms[2]); 
            int term = labelExist(constant, staticMemoryLabels);
            int result = encode_Itype(8,0,registers[terms[1]], term);
            write_binary(encode_Itype(8,0,registers[terms[1]], term),inst_outfile);
        }
        if(inst_type == "jalr"){ //code to write the jalr instructions to memory in binary
            if(terms.size() == 2){
            int result = encode_Rtype(0,registers[terms[1]],0,31,0,9);
            write_binary(encode_Rtype(0,registers[terms[1]],0,31,0,9),inst_outfile);
            }
            else{
            int result = encode_Rtype(0,registers[terms[1]],0,registers[terms[2]],0,9);
            write_binary(encode_Rtype(0,registers[terms[1]],0,registers[terms[2]],0,9),inst_outfile);                
            }
        }
        if(inst_type == "li"){ //code to write the li instructions to memory in binary
            int result = encode_Itype(8,0,registers[terms[1]],std::stoi(terms[2]));
            write_binary(encode_Itype(8,0,registers[terms[1]],std::stoi(terms[2])),inst_outfile); 
        }
        if(inst_type == "mov"){ //code to write the mov instructions to memory in binary
            int result = encode_Rtype(0,registers[terms[2]],registers["$0"],registers[terms[1]],0,32);
            write_binary(encode_Rtype(0,registers[terms[2]],registers["$0"],registers[terms[1]],0,32),inst_outfile);
        }
        if(inst_type == "bge"){ //code to write the bge instructions to memory in binary
            int result = encode_Rtype(0,registers[terms[1]],registers[terms[2]],registers["$at"],0,42);
            write_binary(encode_Rtype(0,registers[terms[1]],registers[terms[2]],registers["$at"],0,42),inst_outfile);
            
            std::string constant = (terms[3]); 
            int term = labelExist(constant, instructionLabels);
            term = count + 1 - term;
            term = term * -1;
            result = encode_Itype(4,registers["$at"],registers["$0"],term);
            write_binary(encode_Itype(4,registers["$at"],registers["$0"],term),inst_outfile);
        }
        if(inst_type == "bgt"){ //code to write the bgt instructions to memory in binary
            int result = encode_Rtype(0,registers[terms[2]],registers[terms[1]],registers["$at"],0,42);
            write_binary(encode_Rtype(0,registers[terms[2]],registers[terms[1]],registers["$at"],0,42),inst_outfile);
            
            std::string constant = (terms[3]); 
            int term = labelExist(constant, instructionLabels);
            term = count + 1 - term;
            term = term * -1;
            result = encode_Itype(5,registers["$at"],registers["$0"],term);
            write_binary(encode_Itype(5,registers["$at"],registers["$0"],term),inst_outfile);
        }
        if(inst_type == "ble"){ //code to write the ble instructions to memory in binary
            int result = encode_Rtype(0,registers[terms[2]],registers[terms[1]],registers["$at"],0,42);
            write_binary(encode_Rtype(0,registers[terms[2]],registers[terms[1]],registers["$at"],0,42),inst_outfile);
            
            std::string constant = (terms[3]); 
            int term = labelExist(constant, instructionLabels);
            term = count + 1 - term;
            term = term * -1;

            result = encode_Itype(4,registers["$at"],registers["$0"],term);
            write_binary(encode_Itype(4,registers["$at"],registers["$0"],term),inst_outfile);
        }
        if(inst_type == "blt"){//code to write the blt instructions to memory in binary
            int result = encode_Rtype(0,registers[terms[1]],registers[terms[2]],registers["$at"],0,42);
            write_binary(encode_Rtype(0,registers[terms[1]],registers[terms[2]],registers["$at"],0,42),inst_outfile);
            
            std::string constant = (terms[3]); 
            int term = labelExist(constant, instructionLabels);
            term = count + 1 - term;
            term = term * -1;

            result = encode_Itype(5,registers["$at"],registers["$0"],term);
            write_binary(encode_Itype(5,registers["$at"],registers["$0"],term),inst_outfile);
        }
        if(inst_type == "and"){ //code to write the and instructions to memory in binary
            int result = encode_Rtype(0,registers[terms[2]],registers[terms[3]],registers[terms[1]],0,36);
            write_binary(encode_Rtype(0,registers[terms[2]],registers[terms[3]],registers[terms[1]],0,36),inst_outfile);
        }
        if(inst_type == "or"){ //code to write the or instructions to memory in binary
            int result = encode_Rtype(0,registers[terms[2]],registers[terms[3]],registers[terms[1]],0,37);
            write_binary(encode_Rtype(0,registers[terms[2]],registers[terms[3]],registers[terms[1]],0,37),inst_outfile);
        }
        if(inst_type == "nor"){//code to write the nor instructions to memory in binary
            int result = encode_Rtype(0,registers[terms[2]],registers[terms[3]],registers[terms[1]],0,39);
            write_binary(encode_Rtype(0,registers[terms[2]],registers[terms[3]],registers[terms[1]],0,39),inst_outfile);
        }
        if(inst_type == "xor"){ //code to write the xor instructions to memory in binary
            int result = encode_Rtype(0,registers[terms[2]],registers[terms[3]],registers[terms[1]],0,38);
            write_binary(encode_Rtype(0,registers[terms[2]],registers[terms[3]],registers[terms[1]],0,38),inst_outfile);
        }
        if(inst_type == "andi"){ //code to write the andi instructions to memory in binary
            int result = encode_Itype(12,registers[terms[2]],registers[terms[1]],std::stoi(terms[3]));
            write_binary(encode_Itype(12,registers[terms[2]],registers[terms[1]],std::stoi(terms[3])),inst_outfile);
        }
        if(inst_type == "ori"){//code to write the ori instructions to memory in binary
            int result = encode_Itype(13,registers[terms[2]],registers[terms[1]],std::stoi(terms[3]));
            write_binary(encode_Itype(13,registers[terms[2]],registers[terms[1]],std::stoi(terms[3])),inst_outfile);
        }
        if(inst_type == "xori"){//code to write the xori instructions to memory in binary
            int result = encode_Itype(14,registers[terms[2]],registers[terms[1]],std::stoi(terms[3]));
            write_binary(encode_Itype(14,registers[terms[2]],registers[terms[1]],std::stoi(terms[3])),inst_outfile);
        }
        if(inst_type == "lui"){ //code to write the lui instructions to memory in binary
            int result = encode_Itype(15,0,registers[terms[1]],std::stoi(terms[2]));
            write_binary(encode_Itype(15,0,registers[terms[1]],std::stoi(terms[2])),inst_outfile);
        } 
        //ensures that count increments correctly (only on instructions)
        if(inst.find(":") == std::string::npos && inst.find(".") == std::string::npos){
            count++;
        }
    }
}
#endif
