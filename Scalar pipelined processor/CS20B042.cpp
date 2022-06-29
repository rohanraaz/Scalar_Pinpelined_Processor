#include<bits/stdc++.h>
#include<iostream>
#include<fstream>

#include "conversions.h"

using namespace std;

enum opcode {ADD,SUB,MUL,INC,AND,OR,NOT,XOR,LOAD,STORE,JMP,BEQZ,FOO1,FOO2,FOO3,HLT};

int cycle = 0,arithmetic=0,logical=0,data_transfer=0,control_transfer=0,instruction_count=0,halt_inst_count = 0;
int datastalls = 0;


int halt = 0; //change halt to 1 when you encounter a halt instruction

//IR store the instruction in decimal form
int IR=-1;

//PC stores the line in which next instrution to be executed is present in ICache.txt file
int PC=0;


//special registers
//output of ALU is stored here in every cycle
int ALUoutput = -1;

//if the instruction is a branch instruction this instruction gives the offset that has to be added to current PC value
int L = -1;


//signals

//this is the pipeline register between decode and execute
int ALU_or_branch = -1; //0 inplies ALU operation 1 implies branch instruction
int source1;
int source2;
int destination_reg;
int source1_reg,source2_reg;
int takefromofsource1 = 0,takefromofsource2 = 0;
opcode operation;



//this is the pipeline register between execute and mem access stages
opcode operation_memaccess;
int writeback_data,writeback_reg;
int mem_access = -1; //1 implies next intruction is a memory access 0 implies next intruction is not memory access
int MAR; //memory adress register
int MBR; //memory buffer/data register
bool read_or_write; //0 implies read 1 implies write



//this is the pipeline register between mem access atage and wb stage
opcode operation_wb;
int wb = -1; //1 implies next instruction includes write back stage 0 implies no write back stage
int wb_data,wb_reg; //wb_location stores the destination register number



string rf[16];
string of_bus[16];
int rf_validity[16];

string icache[256];
string dcache[256];


void fetch()
{
    string instruction_part1,instruction_part2;

    instruction_part1 = icache[PC];
    instruction_part2 = icache[PC];

    IR = hex_to_int(instruction_part1);

    IR*=256;

    IR += hex_to_int(instruction_part2);

    PC+=2;
}


void initialization()
{
    ifstream RF_readport;
    RF_readport.open("RF.txt");

    for(int i=0;i<16;i++)
    {
        RF_readport >> rf[i];
        //cout << "from initialization : " << endl;
        //cout << "for i = " << i << " r[i] = " << rf[i] << endl;
        rf_validity[i] = 1;
        of_bus[i] = rf[i];
    }

    //cout << "\n\n\n\n";

    ifstream icache_readport;

    icache_readport.open("ICache.txt");

    for(int i=0;i<256;i++)
    {
        icache_readport >> icache[i];
    }

    ifstream dcache_readport;

    dcache_readport.open("DCache.txt");

    for(int i=0;i<256;i++)
    {
        dcache_readport >> dcache[i];
    }
}

void destructor()
{
    ofstream icache_writeport;
    ofstream dcache_writeport;
    ofstream output;

    dcache_writeport.open("ODCache.txt");
    output.open("Output.txt");

    for(int i=0;i<256;i++)
    {
        dcache_writeport << dcache[i];
        dcache_writeport << endl;
    }

    //output << "Total number of cycles took : " << cycle+datastalls << endl;
    output << "Total number of instructions executed : " << instruction_count << endl;
    output << "Number of instructions in each class" << endl;
    output << "Total number of arithmetic instructions : " << arithmetic << endl;
    output << "Total number of logical instructions : " << logical << endl;
    output << "Total number of data_transfer instructions : " << data_transfer << endl;
    output << "Total number of control_transfer instructions : " << control_transfer << endl;
    output << "Total number of halt instructions : " << halt_inst_count << endl;
    output << "Number of cycles per instruction : " << (double)(cycle+datastalls)/instruction_count << endl;
    output << "Total number of stalls : " << datastalls << endl;

}


int main()
{
    initialization();


    while(halt!=1)
    {
        
        //write back stage
        if(wb!=-1)
        {
            //you are supposed to write it as a hexadecimal string in 1's complement form

            if(operation_wb!=HLT)
            {
                //updated here
                string data = twos_complement_int_to_hex(wb_data);
                
                rf[wb_reg] = data;

                //operand forwarding
                rf_validity[wb_reg] = 1;
            }
            else
            {
                halt = 1;
            }
        }




        //memory access stage
        if(mem_access!=-1)
        {
            operation_wb = operation_memaccess;
            if(mem_access==1)
            {
                //load instruction
                if(read_or_write==0)
                {
                    MBR = twos_complement_hex_to_int(dcache[MAR]);

                    wb = 1;
                    wb_data = MBR;
                    wb_reg = writeback_reg;
                }

                //store instruction
                else if(read_or_write==1)
                {
                    //assume that no overflow is there
                    wb = -1;
                    dcache[MAR] = twos_complement_int_to_hex(MBR);
                }
            }
            else if(mem_access==0)
            {
                operation_wb = operation_memaccess;
                wb = 1;
                wb_data = writeback_data;
                wb_reg = writeback_reg;
            }
        }




        //execute stage
        if(ALU_or_branch!=-1)
        {
            operation_memaccess = operation;
            //ALU operation
            if(ALU_or_branch==0)
            {
                //operand forwarding
                if(takefromofsource1 && operation!=LOAD && operation!=STORE)
                    source1 = twos_complement_hex_to_int(of_bus[source1_reg]);
                if(takefromofsource2 && operation!=LOAD && operation!=STORE)
                    source2 = twos_complement_hex_to_int(of_bus[source2_reg]);
                

                switch (operation)
                {
                    case ADD : ALUoutput = source1+source2; mem_access=0; break;
                    case SUB : ALUoutput = source1-source2; mem_access=0; break;
                    case MUL : ALUoutput = source1*source2; mem_access=0; break;
                    case INC : ALUoutput = source1+1; mem_access=0; break;
                    case AND : ALUoutput = source1 & source2; mem_access=0; break;
                    case OR : ALUoutput = source1 | source2; mem_access=0; break;
                    case NOT : ALUoutput = -1*source1; mem_access=0; break;
                    case XOR : ALUoutput = source1^source2; mem_access=0; break;

                    //if the operation if effective adress calculation
                    case LOAD : ALUoutput = source1+source2; MAR = ALUoutput; MBR = twos_complement_hex_to_int(rf[destination_reg]); mem_access=1; read_or_write=0; writeback_reg=destination_reg; break;
                    case STORE : ALUoutput = source1+source2; MAR = ALUoutput; MBR = twos_complement_hex_to_int(rf[destination_reg]); if(rf_validity[destination_reg]==0) MBR = twos_complement_hex_to_int(of_bus[destination_reg]); mem_access=1; read_or_write=1; break;
                    case HLT : mem_access=0;               
                    default : break;
                }

                //operand forwarding
                if(rf_validity[destination_reg]==0 && operation!=LOAD && operation!=STORE)
                {
                    of_bus[destination_reg] = twos_complement_int_to_hex(ALUoutput);
                }

                if(mem_access==0)
                {
                    writeback_data = ALUoutput;
                    writeback_reg = destination_reg;
                }
            }

            /*---------------------- still have to deal with branch instruction execution part -------------------------*/
        }




        //decode stage
        if(IR!=-1)
        {
            int operation_code = IR/4096;
            int rem = IR%4096;

            
            switch (operation_code)
            {
                case 0 : operation = ADD; ALU_or_branch = 0; arithmetic++; break;
                case 1 : operation = SUB; ALU_or_branch = 0; arithmetic++; break;
                case 2 : operation = MUL; ALU_or_branch = 0; arithmetic++; break;
                case 3 : operation = INC; ALU_or_branch = 0; arithmetic++; break;
                case 4 : operation = AND; ALU_or_branch = 0; logical++; break;
                case 5 : operation = OR; ALU_or_branch = 0; logical++; break;
                case 6 : operation = NOT; ALU_or_branch = 0; logical++; break;
                case 7 : operation = XOR; ALU_or_branch = 0; logical++; break;
                case 8 : operation = LOAD; ALU_or_branch = 0; data_transfer++; break;
                case 9 : operation = STORE; ALU_or_branch = 0; data_transfer++; break;
                case 10 : operation = JMP; ALU_or_branch = 1; control_transfer++; break;
                case 11 : operation = BEQZ; ALU_or_branch = 1; control_transfer++; break;
                case 15 : operation = HLT; halt=1; halt_inst_count++; break;
                
                default : break;
            }

            int s1_reg,s2_reg;

            destination_reg = rem/256;

            
            rem = rem%256;

            s1_reg = rem/16;
            rem%=16;

            s2_reg = rem;

            source1 = twos_complement_hex_to_int(rf[s1_reg]);
            source2 = twos_complement_hex_to_int(rf[s2_reg]);


            //operand forwarding
            rf_validity[destination_reg] = 0;

            if(rf_validity[s1_reg]==0)
                takefromofsource1 = 1;
            else 
                takefromofsource1=0;
            if(rf_validity[s2_reg]==0)
                takefromofsource2 = 1;
            else
                takefromofsource2 = 0;
            
            source1_reg = s1_reg;
            source2_reg = s2_reg;

            if(operation==LOAD || operation==STORE)
            {
                source2 = s2_reg;
            }

            if(operation==LOAD)
            {
                datastalls+=2;
                rf[destination_reg] = dcache[source1+source2];
                of_bus[destination_reg] = dcache[source1+source2];
            }
        }




        //fetch stage
        if(1)
        {
            string instruction_part1,instruction_part2;
    
            instruction_part1 = icache[PC];
            PC++;
            instruction_part2 = icache[PC];

            IR = hex_to_int(instruction_part1);

            IR*=256;

            IR += hex_to_int(instruction_part2);

            PC++;
            if(IR!=0)
                instruction_count++;
        }

        cycle++;
    }
    cycle++;


    destructor();

    return 0;
}