#include "kernel/types.h"
#include "user/user.h"
#include "kernel/fcntl.h"

int
main(int argc, char *argv[])
{
    if(argc < 2){
        fprintf(2, "Usage: sixfive <file>\n");
        exit(1);
    }

    int fileDescriptor = open(argv[1], 0);
    if(fileDescriptor < 0){
        fprintf(2, "Error: cannot open file %s\n", argv[1]);
        exit(1);
    }

    char currentChar[1];
    char numberBuffer[32];
    int bufferIndex = 0;
    char *separators = " -\r\t\n./,";

    while(read(fileDescriptor, currentChar, 1) == 1){
        if(strchr(separators, currentChar[0]) == 0){
            if(bufferIndex < sizeof(numberBuffer)-1){
                numberBuffer[bufferIndex++] = currentChar[0];
            }
        } else {
            if(bufferIndex > 0){
                numberBuffer[bufferIndex] = 0;
                int number = atoi(numberBuffer);
                if(number % 5 == 0 || number % 6 == 0){
                    printf("%d\n", number);
                }
                bufferIndex = 0;
            }
        }
    }

    if(bufferIndex > 0){
        numberBuffer[bufferIndex] = 0;
        int number = atoi(numberBuffer);
        if(number % 5 == 0 || number % 6 == 0){
            printf("%d\n", number);
        }
    }

    close(fileDescriptor);
    exit(0);
}

