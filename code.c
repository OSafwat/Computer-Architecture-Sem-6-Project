#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

short int instructionMem[1024]; // every instruction will be stored in decimal format
int8_t dataMem[2048];           // all Data will also be stored in decimal formats
int8_t GPRS[64];
int8_t SREG[8];
short int PC;
int size;
long Cycle = 0; // determine cycle for scheduling the different tasks.
char *instructionMap[12] = {"ADD", "SUB", "MUL", "LDI", "BEQZ", "AND", "OR", "JR", "SLC", "SRC", "LB", "SB"};
int counter = 1;

short int instructionToBeDecoded = -1;
short int instructionToBeExecuted[3] = {-1, -1, -1};

short int binaryStringToShort(const char *binaryString)
{ // ts Also AI, also didnt check its correctness
  if (binaryString == NULL || *binaryString == '\0')
    return 0; // Handle null or empty string

  long int result = 0;
  int sign = 1;
  int i = 0;

  if (binaryString[0] == '-')
  {
    sign = -1;
    i++;
  }

  while (binaryString[i] != '\0')
  {
    if (binaryString[i] == '0' || binaryString[i] == '1')
    {
      result = (result << 1) + (binaryString[i] - '0');
      // Check for overflow.  This is tricky without SHRT_MAX.
      if (sign == 1 && result > 32767)
      { // 2^15 - 1
        return 32767;
      }
      else if (sign == -1 && result > 32768)
        return -32768; // 2^15
    }
    else
      return 0; // Handle invalid characters
    i++;
  }
  return (short int)(result * sign);
}

char *intToBinaryString(int num)
{ // ts AI, i didnt check its correctness
  if (num == 0)
  {
    char *binaryString = malloc(2);
    strcpy(binaryString, "0");
    return binaryString;
  }

  int bits = 0;
  unsigned int temp = (num < 0) ? -num : num;
  while (temp)
  {
    temp >>= 1;
    bits++;
  }

  char *binaryString = malloc(bits + 1);
  if (!binaryString)
    return NULL;

  binaryString[bits] = '\0';
  for (int i = bits - 1; i >= 0; i--)
  {
    binaryString[i] = (num & 1) + '0';
    num >>= 1;
  }
  return binaryString;
}
char *RToBinaryString(const char *RString)
{
  if (RString == NULL || RString[0] != 'R')
  {
    return NULL; // Handle invalid input
  }

  // Extract the number part of the string
  char *sub = (char *)malloc(strlen(RString));
  char *numberString = strncpy(sub, RString + 1, strlen(RString) - 1); // Pointer to the character after 'R'
  long number = strtol(numberString, NULL, 10);                        // Convert to long

  if (number == 0)
  {
    char *binaryString = malloc(4);
    if (binaryString == NULL)
      return NULL;
    strcpy(binaryString, "0");
    return binaryString;
  }

  if (number < 0)
  {
    return NULL;
  }
  // Calculate the number of bits needed (optimized)
  int bits = 0;
  unsigned long temp = number;
  while (temp)
  {
    temp >>= 1;
    bits++;
  }

  // Handle the case where the number is 0
  if (bits == 0)
  {
    bits = 1; // Need at least one bit for "0"
  }

  // Allocate memory for the binary string
  char *binaryString = malloc(bits + 1); // +1 for the null terminator
  if (binaryString == NULL)
  {
    return NULL; // Handle allocation failure
  }

  binaryString[6] = '\0'; // Null-terminate the string

  // Convert the number to binary (most efficient)
  int l = 0;
  for (int i = 5; i >= 0; i--)
    binaryString[5 - i] = (number & (1 << i)) ? '1' : '0';
  return binaryString;
}

short int encode(char *inst)
{ // instructions must be encoded (as bin, dec, or String) before it can be used

  char *temp = strtok(inst, " ");
  printf("\n%s\n", temp);
  char *ans;
  ans[0] = '0';
  if (strcmp(temp, "ADD") == 0)
    ans = "0000";
  else if (strcmp(temp, "SUB") == 0)
    ans = "0001";
  else if (strcmp(temp, "MUL") == 0)
    ans = "0010";
  else if (strcmp(temp, "LDI") == 0)
    ans = "0011";
  else if (strcmp(temp, "BEZQ") == 0)
    ans = "0100";
  else if (strcmp(temp, "AND") == 0)
    ans = "0101";
  else if (strcmp(temp, "OR") == 0)
    ans = "0110";
  else if (strcmp(temp, "JR") == 0)
    ans = "0111";
  else if (strcmp(temp, "SLC") == 0)
    ans = "1000";
  else if (strcmp(temp, "SRC") == 0)
    ans = "1001";
  else if (strcmp(temp, "LB") == 0)
    ans = "1010";
  else if (strcmp(temp, "SB") == 0)
    ans = "1011";

  char *check = strtok(NULL, " ");
  printf("%s\n", check);
  char *R1 = RToBinaryString(check);
  check = strtok(NULL, " ");
  printf("%s\n", check);
  char *R2 = RToBinaryString(check);
  printf("%s\n", R1);
  printf("%s\n", R2);
  printf("%s\n", ans);
  char finalAns[17];
  strcpy(finalAns, ans);
  strcat(finalAns, R1);
  strcat(finalAns, R2);
  // For R1
  // For R2/Imm
  // now we have the full instruction encoded into binary in String format
  return (binaryStringToShort(finalAns));
}

void GetInstructions(FILE *fptr)
{ // i believe this will only be called in the beginning to store instructions in MEM, but idk yet
  rewind(fptr);

  char stra[9999];
  int index = 0;
  while (fgets(stra, sizeof(stra), fptr) != NULL)
    instructionMem[index++] = encode(stra);
  size = index;
  fclose(fptr);
}


void execute()
{ 
    int opcode = instructionToBeExecuted[0];
    int R1     = instructionToBeExecuted[1];
    int R2, Imm;
    R2 = Imm = instructionToBeExecuted[2];
    short int result = 0;
    bool logicalOrArithmetic = true;
    int8_t newValue = 0;

    if (opcode == -1)
        return;

    switch (opcode)
    {
    case 0b0: {  // ADD
        result   = GPRS[R1] + GPRS[R2];
        if (result & (1 << 8))     SREG[4] = 1;
        newValue = result & ((1 << 8) - 1);
        if (!(((GPRS[R1] ^ GPRS[R2]) & (1 << 7))) &&
             (((newValue   ^ GPRS[R2]) & (1 << 7))))
            SREG[3] = 1;

        {   // reg write & log
            int8_t old   = GPRS[R1];
            GPRS[R1]     = newValue;
            logChange("EXEC", "R", R1, old, GPRS[R1]);
        }
        break;
    }
    case 0b1: {  // SUB
        result   = GPRS[R1] - GPRS[R2];
        if (result & (1 << 8))     SREG[4] = 1;
        newValue = result & ((1 << 8) - 1);
        if ( ((GPRS[R1] ^ GPRS[R2]) & (1 << 7)) &&
            !(((newValue  ^ GPRS[R2]) & (1 << 7))) )
            SREG[3] = 1;

        {
            int8_t old = GPRS[R1];
            GPRS[R1]   = newValue;
            logChange("EXEC", "R", R1, old, GPRS[R1]);
        }
        break;
    }
    case 0b10: { // MUL
        result = GPRS[R1] * GPRS[R2];
        {
            int8_t old = GPRS[R1];
            GPRS[R1]   = GPRS[R1] * GPRS[R2];
            logChange("EXEC", "R", R1, old, GPRS[R1]);
        }
        break;
    }
    case 0b11: { // LDI
        logicalOrArithmetic = false;
        {
            int8_t old = GPRS[R1];
            GPRS[R1]   = Imm;
            logChange("EXEC", "R", R1, old, GPRS[R1]);
        }
        break;
    }
    case 0b100: { // BEQZ
        logicalOrArithmetic = false;
        if (GPRS[R1] == 0) {
            int oldPC = PC;
            PC += 1 + Imm;
            logChange("EXEC", "PC", 0, oldPC, PC);
        }
        break;
    }
    case 0b101: { // AND
        result = GPRS[R1] & GPRS[R2];
        {
            int8_t old = GPRS[R1];
            GPRS[R1]   &= GPRS[R2];
            logChange("EXEC", "R", R1, old, GPRS[R1]);
        }
        break;
    }
    case 0b110: { // OR
        result = GPRS[R1] | GPRS[R2];
        {
            int8_t old = GPRS[R1];
            GPRS[R1]   |= GPRS[R2];
            logChange("EXEC", "R", R1, old, GPRS[R1]);
        }
        break;
    }
    case 0b111: { // JR
        {
            int oldPC = PC;
            PC = (GPRS[R1] << 8) | GPRS[R2];
            logicalOrArithmetic = false;
            logChange("EXEC", "PC", 0, oldPC, PC);
        }
        break;
    }
    case 0b1000: { // SLC
        result   = (GPRS[R1] << Imm) | (GPRS[R1] >> (8 - Imm));
        {
            int8_t old = GPRS[R1];
            GPRS[R1]   = result;
            logChange("EXEC", "R", R1, old, GPRS[R1]);
        }
        break;
    }
    case 0b1001: { // SRC
        result   = (GPRS[R1] >> Imm) | (GPRS[R1] << (8 - Imm));
        {
            int8_t old = GPRS[R1];
            GPRS[R1]   = result;
            logChange("EXEC", "R", R1, old, GPRS[R1]);
        }
        break;
    }
    case 0b1010: { // LB
        logicalOrArithmetic = false;
        {
            int8_t old = GPRS[R1];
            GPRS[R1]   = dataMem[Imm];
            logChange("EXEC", "R", R1, old, GPRS[R1]);
        }
        break;
    }
    case 0b1011: { // SB
        logicalOrArithmetic = false;
        {
            int8_t old = dataMem[Imm];
            dataMem[Imm] = R1;
            logChange("EXEC", "DM", Imm, old, dataMem[Imm]);
        }
        break;
    }
    } // end switch

    if (logicalOrArithmetic)
    {
        if (result & (1 << 8))       SREG[2] = 1;
        else                         SREG[2] = 0;

        if (!(result & ((1 << 8) - 1))) SREG[0] = 1;
        else                             SREG[0] = 0;

        SREG[1] = SREG[3] ^ SREG[2];
    }

    /* remove old printf; add unified log of EXEC stage */
    logStage("EXEC", instructionMap[opcode], opcode, R1, R2, result);
}


void printinfo()
{
}
// For each clock cycle, you need to print which instruction is in each stage, as well as, the values
// that entered the stage, and the output of this stage.

//  Moreover, if you changed the value of a location in the memory or the register file, you
// need to print that this location or register (including R0) value has changed alongside the
// new value (and in which stage did the value change).

//  At the end of your program, you need to print the values of all registers (general and
// special purpose including the PC and SREG), and the full instruction and data memory
// locations.
long binaryStringToNumber(const char *binaryString)
{ //AI
  if (binaryString == NULL || binaryString[0] == '\0')
  {
    return 0; // Handle null or empty string
  }

  long number = 0;
  size_t length = strlen(binaryString);

  for (size_t i = 0; i < length; i++)
  {
    if (binaryString[i] != '0' && binaryString[i] != '1')
    {
      return -1; // Handle invalid characters
    }
    number = (number << 1) | (binaryString[i] - '0');
  }
  return number;
}

int binaryToInt(int x)
{
  int num = 0;
  for (int i = 16; i >= 0; i--)
    num += (1 << i) * ((x >> (i)) & 1);
  return num;
}

void instructionDecode()
{
  short int instruction = instructionToBeDecoded;
  int opcode;
  int R1;
  int R2_Im; // this can be either immediate or R2 according to instruction type since they both occupy up the same bits
  if (instruction == -1)
    return;
  if (PC == size + 1 || PC == size + 2)
  {
    printf("                               ");
    execute();
    return;
  }
  opcode = (instruction >> 12) & 0b1111;
  R1 = (instruction >> 6) & 0b111111;
  R2_Im = (instruction) & 0b111111;
  printf("Decode Stage: Instruction %d    ", counter - 1);
  printf("\nInstruction: %s R%d R%d\n", instructionMap[opcode], binaryToInt(R1), binaryToInt(R2_Im));
  printf("Output: NULL\n");
  printf("Input Values: R%d R%d\n", binaryToInt(R1), binaryToInt(R2_Im));
  if (counter < 3)
    printf("\n");
  execute();
  instructionToBeExecuted[0] = opcode;
  instructionToBeExecuted[1] = R1;
  instructionToBeExecuted[2] = R2_Im;

  logStage("DECODE", instructionMap[opcode], instruction, R1, R2_Im, 0);

}


void instructionFetch()
{
    
    Cycle++;

    short int instruction;
    if (PC == size || PC == size + 1)
    {
        
        printf("[Cycle %ld] Fetch Stage: bubble\n", Cycle);
        instructionDecode();
        return;
    }

    
    instruction = instructionMem[PC++];

    
    printf("[Cycle %ld] Fetch Stage: Instruction %d    Input: PC=%d    Output: inst=0x%04X\n",
           Cycle, counter, PC-1, instruction);

    if (counter < 2)
        printf("\n");

    instructionDecode();
    counter++;
    instructionToBeDecoded = instruction;
}
 // ts kinda useless 💔💔💔




void logStage(const char *stage, const char *msg, short int  rawInst, int v1, int v2, int result)
{
    printf("\n[Cycle %ld] %-8s | %-24s | IN:0x%04X V1:%d V2:%d  OUT:%d\n",
           Cycle, stage, msg, rawInst, v1, v2, result);
}

// register  memory change tracker
void logChange(const char *stage, const char *what, int idx, int oldVal, int newVal)
{
    if (oldVal != newVal)
        printf("          ↳ %-8s changed %-4s[%d] : %d → %d\n",
               stage, what, idx, oldVal, newVal);
}

// print final state
void dumpFinalState(void)
{
    puts("\n-----------  FINAL STATE   -------------");
    printf("PC  = %d\n", PC);
    printf("SREG= ");
    for (int i = 7; i >= 0; --i) printf("%d", SREG[i]);
    puts("\n\n-- General-purpose registers -----------------------------");
    for (int i = 0; i < 64; ++i)
        printf("R%-2d = %-4d%s", i, GPRS[i], (i % 8 == 7) ? "\n" : "  ");
    puts("\n-- Instruction Memory -----------------------------------");
    for (int i = 0; i < size; ++i)
        printf("IM[%3d] = %d\n", i, instructionMem[i]);
    puts("-- Data Memory ------------------------------------------");
    for (int i = 0; i < 2048; ++i)
        if (dataMem[i])                                 /* only non-zero   */
            printf("DM[%4d] = %d\n", i, dataMem[i]);
}


int main()
{
  printf("testing");
  PC = 0;
  const char *filename = "C:\\Users\\youss\\OneDrive\\Documents\\Computer-Architecture-Sem-6-Project\\input.txt";
  FILE *filePointer = fopen(filename, "r");

  if (filePointer == NULL)
    printf("\nError opening input.txt\n");
  GetInstructions(filePointer);
  for (int i = 0; i < size; i++)
    instructionFetch();

  instructionFetch();
  PC++;
  instructionFetch();

  dumpFinalState();

}
