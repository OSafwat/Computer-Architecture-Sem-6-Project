#include <stdio.h>
#include <time.h>


int main(){
    FILE *fptr = fopen("text.txt" , "r");
    
    char stra[9999];
    while (fgets(stra, sizeof(stra), fptr) != NULL) {
        printf("%s", stra);
      }
    fclose(fptr);
return 0;
}
