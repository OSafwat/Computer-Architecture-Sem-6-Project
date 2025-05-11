#include <stdio.h>
#include <stdint.h>
#include <string.h>

short int instructionMem[1024]; // every instruction will be stored in decimal format
int8_t dataMem[2048];           // all Data will also be stored in decimal formats
int8_t GPRS[64];
int8_t SREG[8];
short int PC;
long Cycle = 0; // this determines what cycle we are in right now used for scheduling the different tasks.

void GetInstrucions(FILE *fptr)
{ // i believe this will only be called in the beginning to store instructions in MEM, but idk yet
  fptr = fopen("text.txt", "r");
  char stra[9999];
  while (fgets(stra, sizeof(stra), fptr) != NULL)
    printf("%s", stra);
  fclose(fptr);
  return 0;
}

short int encode(char *inst)
{ // instructions must be encoded (as bin, dec, or String) before it can be used
  char *temp = strtok(inst, " ");
  char *ans = "";
  if (strcmp(temp, "ADD") == 0)
    strcat(ans, "0000");
  else if (strcmp(temp, "SUB") == 0)
    strcat(ans, "0001");
  else if (strcmp(temp, "MUL") == 0)
    strcat(ans, "0010");
  else if (strcmp(temp, "LDI") == 0)
    strcat(ans, "0011");
  else if (strcmp(temp, "BEZQ") == 0)
    strcat(ans, "0100");
  else if (strcmp(temp, "AND") == 0)
    strcat(ans, "0101");
  else if (strcmp(temp, "OR") == 0)
    strcat(ans, "0110");
  else if (strcmp(temp, "JR") == 0)
    strcat(ans, "0111");
  else if (strcmp(temp, "SLC") == 0)
    strcat(ans, "1000");
  else if (strcmp(temp, "SRC") == 0)
    strcat(ans, "1001");
  else if (strcmp(temp, "LB") == 0)
    strcat(ans, "1010");
  else if (strcmp(temp, "SB") == 0)
    strcat(ans, "1011");

  strcat(ans, intToBinaryString(strtok(NULL, " "))); // For R1
  strcat(ans, intToBinaryString(strtok(NULL, " "))); // For R2/Imm
  // now we have the full instruction encoded into binary in String format
  return (binaryStringToShort(ans));
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

short int instructionFetch(int PC)
{
  short int instruction;
  instruction = instructionMem[PC];
  return instruction;
} // ts kinda useless 💔💔💔

void instructionDecode(short int instruction)
{
  int opcode;
  int R1;
  int R2_Im; // this can be either immediate or R2 according to instruction type since they both occupy up the same bits

  opcode = (instruction >> 12) & 0b1111;
  R1 = (instruction >> 6) & 0b111111;
  R2_Im = (instruction) & 0b111111;
}

void execute(int instruction)
{ // Didnt implement SREG Flags!!!
  int opcode;
  int R1;
  int R2;
  int Imm;
  switch (opcode)
  {
  case (0b0):
    R1 += R2;
    break;
  case (0b1):
    R1 = R1 - R2;
    break;
  case (0b10):
    R1 *= R2;
    break;
  case (0b11):
    R1 = Imm;
    break;
  case (0b100):
    if (R1 == 0)
      PC += 1 + Imm;
    break;
  case (0b101):
    R1 = R1 & R2;
    break;
  case (0b110):
    R1 = R1 | R2;
    break;
  case (0b111):
    // PC = concat between R1 and R2
    break;
  case (0b1000):
    R1 = R1 << Imm | R1 >> 8 - Imm;
    break;
  case (0b1001):
    R1 = R1 >> Imm | R1 << 8 - Imm;
    break;
  case (0b1010):
    R1 = dataMem[Imm];
    break;
  case (0b1011):
    dataMem[Imm] = R1;
    break;
  }
}

int main()
{
  // if there is something in the description i didnt implement/realize was there, tell me 🙏🙏🙏
}
