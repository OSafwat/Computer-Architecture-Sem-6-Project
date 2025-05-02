#include <stdio.h>
#include <stdint.h>


short int instructionMem[1024]; //every instruction will be stored in decimal format
int dataMem[2048]; //all Data will also be stored in decimal format
int GPRS[64];
int SREG[8];
long Cycle = 0; //this determines what cycle we are in right now used for scheduling the different tasks.

void GetInstrucions(FILE *fptr){ //i believe this will only be called in the beggining to store instructions in MEM, but idk yet
  fptr = fopen("text.txt", "r");  
                                   //mb
  char stra[9999];
  while (fgets(stra, sizeof(stra), fptr) != NULL)
  {
    printf("%s", stra);
  }
  fclose(fptr);
  return 0;
}

char* instructionFetch(int PC){ 
  char* instruction;
  instruction = instructionMem[PC];
  return instruction;
} //ts kinda useless 💔💔💔

void instructionDecode(int instruction){
  int opcode;
  int R1;
  int R2_Im; //this can be either immediate or R2 according to instruction type since they both occupy up the same bits

  opcode = (instruction>>12) & 0b1111;
  R1     = (instruction>>6)  & 0b111111;
  R2_Im  = (instruction)     & 0b111111; 

}

void execute (){
  //
}
int main()
{
  
}
