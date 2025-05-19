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
long Cycle = 0;
char *instructionMap[12] = {"ADD", "SUB", "MUL", "LDI", "BEQZ", "AND", "OR", "JR", "SLC", "SRC", "LB", "SB"};
int counter = 1;
bool flushed = false;

short int instructionToBeDecoded = -1;
short int instructionToBeExecuted[3] = {-1, -1, -1};

short int binaryStringToShort(const char *binaryString)
{
  if (binaryString == NULL || *binaryString == '\0')
    return 0; // Handle null or empty string

  short int result = 0;
  for (int i = 0; i < 16; i++)
    if (binaryString[15 - i] == '1')
      result += (1 << i);

  return (short int)(result);
}

void logStage(const char *stage, const char *msg, short int rawInst, int v1, int v2, int result)
{
  printf("[Cycle %ld] %-8s | %-24s | IN:0x%04X V1:%d V2:%d  OUT:%d\n",
         Cycle, stage, msg, rawInst, v1, v2, result);

  if (!strcmp(stage, "DECODE"))
    printf("\n");
}

// register memory change tracker
void logChange(const char *stage, const char *what, int idx, int oldVal, int newVal, int opcode, int R1, int R2, int result)
{
  logStage("EXEC", instructionMap[opcode], opcode, R1, R2, result);
  if (oldVal != newVal)
    printf("          ↳ %-8s changed %-4s[%d] : %d → %d\n\n",
           stage, what, idx, oldVal, newVal);
  else
    printf("\n");
}

char *intToBinaryString(int num)
{
  char *binary = malloc(7);
  for (int i = 0; i < 6; i++)
  {
    binary[5 - i] = (num & (1 << i)) ? '1' : '0';
  }
  if (num < 0)
  {
    binary[0] = '1';
  }
  binary[6] = '\0';
  return binary;
}
char *RToBinaryString(char *RString)
{
  if (RString == NULL || RString[0] != 'R')
  {
    return NULL; // Handle invalid input
  }

  // Extract the number part of the string
  char *numberString = malloc(strlen(RString));
  strncpy(numberString, RString + 1, strlen(RString) - 1); // Pointer to the character after 'R'
  numberString[strlen(RString) - 1] = '\0';
  long number = strtol(numberString, NULL, 10); // Convert to long
  free(numberString);

  if (number == 0)
  {
    char *binaryString = malloc(7);
    if (binaryString == NULL)
      return NULL;
    strcpy(binaryString, "000000");
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
  char *binaryString = malloc(7); // +1 for the null terminator
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
  bool isImm = false;
  char *temp = strtok(inst, " ");
  char *ans;
  if (strcmp(temp, "ADD") == 0)
    ans = "0000";
  else if (strcmp(temp, "SUB") == 0)
    ans = "0001";
  else if (strcmp(temp, "MUL") == 0)
    ans = "0010";
  else if (strcmp(temp, "LDI") == 0)
  {
    ans = "0011";
    isImm = true;
  }
  else if (strcmp(temp, "BEQZ") == 0)
  {
    ans = "0100";
    isImm = true;
  }
  else if (strcmp(temp, "AND") == 0)
    ans = "0101";
  else if (strcmp(temp, "OR") == 0)
    ans = "0110";
  else if (strcmp(temp, "JR") == 0)
    ans = "0111";
  else if (strcmp(temp, "SLC") == 0)
  {
    ans = "1000";
    isImm = true;
  }
  else if (strcmp(temp, "SRC") == 0)
  {
    ans = "1001";
    isImm = true;
  }
  else if (strcmp(temp, "LB") == 0)
  {
    ans = "1010";
    isImm = true;
  }
  else if (strcmp(temp, "SB") == 0)
  {
    ans = "1011";
    isImm = true;
  }

  char *check = strtok(NULL, " ");
  char *R1 = RToBinaryString(check);
  check = strtok(NULL, " ");
  char *R2;
  if (isImm)
    R2 = intToBinaryString(atoi(check));
  else
    R2 = RToBinaryString(check);
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
  int R1 = instructionToBeExecuted[1];
  int R2, Imm;
  R2 = Imm = instructionToBeExecuted[2];
  short int result = 0;
  bool logicalOrArithmetic = true;
  int8_t newValue = 0;
  if (opcode == -1)
    return;
  bool logged = false;
  switch (opcode)
  {
  case 0b0:
  { // ADD

    result = GPRS[R1] + GPRS[R2];

    // this block is for carry flag
    long temp1 = GPRS[R1];
    long temp2 = GPRS[R2];
    temp1 &= 0x000000FF;
    temp2 &= 0x000000FF;
    long carryResult = (temp1 & 0x000000FF) + (temp2 & 0x000000FF);
    if (carryResult & (1 << 8))
      SREG[4] = 1;
    else
      SREG[4] = 0;

    newValue = result & ((1 << 8) - 1);

    // for overflow flag
    if (!(((GPRS[R1] ^ GPRS[R2]) & (1 << 7))) &&
        (((newValue ^ GPRS[R2]) & (1 << 7))))
      SREG[3] = 1;

    { // reg write & log
      int8_t old = GPRS[R1];
      GPRS[R1] = newValue;
      logged = true;
      logChange("EXEC", "R", R1, old, GPRS[R1], opcode, R1, R2, result);
    }
    break;
  }
  case 0b1:
  { // SUB
    result = GPRS[R1] - GPRS[R2];
    if (result & (1 << 8))
      SREG[4] = 1;
    newValue = result & ((1 << 8) - 1);
    if (((GPRS[R1] ^ GPRS[R2]) & (1 << 7)) &&
        !(((newValue ^ GPRS[R2]) & (1 << 7))))
      SREG[3] = 1;

    {
      int8_t old = GPRS[R1];
      GPRS[R1] = newValue;
      logged = true;
      logChange("EXEC", "R", R1, old, GPRS[R1], opcode, R1, R2, result);
    }
    break;
  }
  case 0b10:
  { // MUL
    result = GPRS[R1] * GPRS[R2];
    {
      int8_t old = GPRS[R1];
      GPRS[R1] = GPRS[R1] * GPRS[R2];
      logged = true;
      logChange("EXEC", "R", R1, old, GPRS[R1], opcode, R1, R2, result);
    }
    break;
  }
  case 0b11:
  { // LDI
    logicalOrArithmetic = false;
    {
      int8_t old = GPRS[R1];
      GPRS[R1] = Imm;
      logged = true;
      logChange("EXEC", "R", R1, old, GPRS[R1], opcode, R1, R2, result);
    }
    break;
  }
  case 0b100:
  { // BEQZ
    logicalOrArithmetic = false;
    if (GPRS[R1] == 0)
    {
      int oldPC = PC - 2;
      PC = oldPC + Imm;
      logged = true;
      logChange("EXEC", "PC", 0, oldPC, PC, opcode, R1, R2, result);
      flushed = true;
    }
    break;
  }
  case 0b101:
  { // AND
    result = GPRS[R1] & GPRS[R2];
    {
      int8_t old = GPRS[R1];
      GPRS[R1] &= GPRS[R2];
      logged = true;
      logChange("EXEC", "R", R1, old, GPRS[R1], opcode, R1, R2, result);
    }
    break;
  }
  case 0b110:
  { // OR
    result = GPRS[R1] | GPRS[R2];
    {
      int8_t old = GPRS[R1];
      GPRS[R1] |= GPRS[R2];
      logged = true;
      logChange("EXEC", "R", R1, old, GPRS[R1], opcode, R1, R2, result);
    }
    break;
  }
  case 0b111:
  { // JR
    {
      int oldPC = PC;
      PC = (GPRS[R1] << 8) | GPRS[R2];
      logicalOrArithmetic = false;
      logged = true;
      logChange("EXEC", "PC", 0, oldPC, PC, opcode, R1, R2, result);
      flushed = true;
    }
    break;
  }
  case 0b1000:
  { // SLC
    result = (GPRS[R1] << Imm) | (GPRS[R1] >> (8 - Imm));
    {
      int8_t old = GPRS[R1];
      GPRS[R1] = result;
      logged = true;
      logChange("EXEC", "R", R1, old, GPRS[R1], opcode, R1, R2, result);
    }
    break;
  }
  case 0b1001:
  { // SRC
    result = (GPRS[R1] >> Imm) | (GPRS[R1] << (8 - Imm));
    {
      int8_t old = GPRS[R1];
      GPRS[R1] = result;
      logged = true;
      logChange("EXEC", "R", R1, old, GPRS[R1], opcode, R1, R2, result);
    }
    break;
  }
  case 0b1010:
  { // LB
    logicalOrArithmetic = false;
    {
      int8_t old = GPRS[R1];
      GPRS[R1] = dataMem[Imm];
      logged = true;
      logChange("EXEC", "R", R1, old, GPRS[R1], opcode, R1, R2, result);
    }
    break;
  }
  case 0b1011:
  { // SB
    logicalOrArithmetic = false;
    {
      int8_t old = dataMem[Imm];
      dataMem[Imm] = GPRS[R1];
      logged = true;
      logChange("EXEC", "DM", Imm, old, dataMem[Imm], opcode, R1, R2, result);
    }
    break;
  }
  } // end switch

  if (logicalOrArithmetic)
  {
    if (result & (1 << 7))
      SREG[2] = 1;
    else
      SREG[2] = 0;

    SREG[0] = result == 0;

    if (opcode == 0 || opcode == 1)
      SREG[1] = SREG[3] ^ SREG[2];
  }

  instructionToBeExecuted[0] = instructionToBeExecuted[1] = instructionToBeExecuted[2] = -1;
  if (!logged)
    logStage("EXEC", instructionMap[opcode], opcode, R1, R2, result);
}

long binaryStringToNumber(const char *binaryString)
{
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
  opcode = (instruction >> 12) & 0b1111;
  R1 = (instruction >> 6) & 0b111111;
  R2_Im = (instruction) & 0b111111;
  bool isImm = opcode == 3 || opcode == 4 || opcode == 8 || opcode == 9 || opcode == 10 || opcode == 11;
  printf("[Cycle %ld] Decode Stage: Instruction %d    ", Cycle, counter - 1);
  if (!isImm)
    printf("Input Values: R%d R%d\n\n", binaryToInt(R1), binaryToInt(R2_Im));
  else
    printf("Input Values: R%d %d\n\n", binaryToInt(R1), binaryToInt(R2_Im));
  logStage("DECODE", instructionMap[opcode], instruction, R1, R2_Im, 0);
  execute();
  instructionToBeExecuted[0] = opcode;
  instructionToBeExecuted[1] = R1;
  instructionToBeExecuted[2] = R2_Im;

  if (flushed)
  {
    instructionToBeExecuted[0] = instructionToBeExecuted[1] = instructionToBeExecuted[2] = -1;
  }

  instructionToBeDecoded = -1;
}

void instructionFetch()
{

  Cycle++;

  short int instruction;
  if (PC >= size)
  {

    printf("[Cycle %ld] Fetch Stage: bubble\n\n", Cycle);
    instructionDecode();
    return;
  }
  printf("[Cycle %ld] Fetch Stage: Instruction %d    Input: PC=%d    Output: inst=0x%04X\n\n",
         Cycle, counter, PC, instruction);

  instructionDecode();

  instruction = instructionMem[PC++];

  counter++;
  instructionToBeDecoded = instruction;
  if (flushed)
  {
    instructionToBeDecoded = -1;
    flushed = false;
  }
}

// print final state
void dumpFinalState(void)
{
  puts("\n-----------  FINAL STATE   -------------");
  printf("PC  = %d\n", PC);
  printf("SREG= ");
  for (int i = 7; i >= 0; --i)
    printf("%d", SREG[i]);
  puts("\n\n-- General-purpose registers -----------------------------");
  for (int i = 0; i < 64; ++i)
    printf("R%-2d = %-4d%s", i, GPRS[i], (i % 8 == 7) ? "\n" : "  ");
  puts("\n-- Instruction Memory -----------------------------------");
  for (int i = 0; i < size; ++i)
    printf("IM[%d] = %d\n", i, instructionMem[i]);
  puts("\n-- Data Memory ------------------------------------------");
  for (int i = 0; i < 2048; ++i)
    if (dataMem[i]) /* only non-zero   */
      printf("DM[%d] = %d\n", i, dataMem[i]);
}

int main()
{
  PC = 0;
  const char *filename = "C:\\Users\\omars\\Desktop\\CA Project\\Computer-Architecture-Sem-6-Project\\input.txt";
  FILE *filePointer = fopen(filename, "r");

  if (filePointer == NULL)
    printf("\nError opening input.txt\n");
  GetInstructions(filePointer);
  while (PC < size)
    instructionFetch();
  execute();
  instructionDecode();
  execute();
  instructionDecode();
  execute();

  dumpFinalState();
}

//[Cycle 20] Decode Stage: Instruction 19    Input Values: R8 2

//[Cycle 20] DECODE   | SLC                      | IN:0xFFFF8202 V1:8 V2:2  OUT:0
//