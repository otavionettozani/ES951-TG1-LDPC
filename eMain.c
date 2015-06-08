#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "e_lib.h"

#include "messages.h"

short getNextElement (char* lastCore, int* lastCoreElement){
	int coreMatrixS[16] = {CORE_0_0_MEM+COMMADDRESS_MATRIX,
		CORE_0_1_MEM+COMMADDRESS_MATRIX,
		CORE_0_2_MEM+COMMADDRESS_MATRIX,
		CORE_0_3_MEM+COMMADDRESS_MATRIX,
		CORE_1_0_MEM+COMMADDRESS_MATRIX,
		CORE_1_1_MEM+COMMADDRESS_MATRIX,
		CORE_1_2_MEM+COMMADDRESS_MATRIX,
		CORE_1_3_MEM+COMMADDRESS_MATRIX,
		CORE_2_0_MEM+COMMADDRESS_MATRIX,
		CORE_2_1_MEM+COMMADDRESS_MATRIX,
		CORE_2_2_MEM+COMMADDRESS_MATRIX,
		CORE_2_3_MEM+COMMADDRESS_MATRIX,
		CORE_3_0_MEM+COMMADDRESS_MATRIX,
		CORE_3_1_MEM+COMMADDRESS_MATRIX,
		CORE_3_2_MEM+COMMADDRESS_MATRIX,
		CORE_3_3_MEM+COMMADDRESS_MATRIX};

	int coreSizeS[16] = {CORE_0_0_MEM+COMMADDRESS_SIZE,
		CORE_0_1_MEM+COMMADDRESS_SIZE,
		CORE_0_2_MEM+COMMADDRESS_SIZE,
		CORE_0_3_MEM+COMMADDRESS_SIZE,
		CORE_1_0_MEM+COMMADDRESS_SIZE,
		CORE_1_1_MEM+COMMADDRESS_SIZE,
		CORE_1_2_MEM+COMMADDRESS_SIZE,
		CORE_1_3_MEM+COMMADDRESS_SIZE,
		CORE_2_0_MEM+COMMADDRESS_SIZE,
		CORE_2_1_MEM+COMMADDRESS_SIZE,
		CORE_2_2_MEM+COMMADDRESS_SIZE,
		CORE_2_3_MEM+COMMADDRESS_SIZE,
		CORE_3_0_MEM+COMMADDRESS_SIZE,
		CORE_3_1_MEM+COMMADDRESS_SIZE,
		CORE_3_2_MEM+COMMADDRESS_SIZE,
		CORE_3_3_MEM+COMMADDRESS_SIZE};

	short *coreMatrix[16];
	int *coreSize[16];

	int i;

	for(i=0;i<16;i++){
		coreMatrix[i] = (short*)coreMatrixS[i];
		coreSize[i] = (int*)coreSizeS[i];
	}


	if(*lastCore==(char)-1 && *lastCoreElement==-1){
		*lastCore = 0;
		*lastCoreElement = 0;

		return (coreMatrix[0])[0x1230];
	}


	int size = *(coreSize[*lastCore]);


	if(*lastCoreElement==size-1){
		if(*lastCore == 15){
			return 0;
		}
		(*lastCore)++;
		size = *(coreSize[*lastCore]);
		*lastCoreElement = 0;
	}else{
		(*lastCoreElement)++;
	}

    short test = (coreMatrix[*lastCore])[*lastCoreElement];
	return test?test:-2;

}

int main(void){


	unsigned char *ready;
	unsigned char *inputData;
	unsigned char *outputData;
	unsigned char *dataCounter;

	char lastCore;
	int lastElement;

	//set the pointer to their variables
	ready = (char*)(COMMADDRESS_READY);
	inputData = (char*)(COMMADDRESS_INPUT);
	outputData = (char*)(COMMADDRESS_OUTPUT);
	dataCounter = (char*)(COMMADDRESS_COUNTER);

	int i;



	while(1){


		lastCore = -1;
		lastElement = -1;

		while(!(*ready));

        short element = -1;
        char parityCheck = 0;
        unsigned char bitPosition;
        unsigned int bytePosition;
        unsigned char selectedBit;
        while(1){
            element = getNextElement(&lastCore,&lastElement);
            if(element == 0){
                break;
            }
            bitPosition = element%8;
            bytePosition = element/8;
            selectedBit = 1<<bitPosition;

            parityCheck ^= (selectedBit&inputData[bytePosition])>>bitPosition;


        }

		*ready = 0;

	}

	return EXIT_SUCCESS;
}
