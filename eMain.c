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

		return (coreMatrix[0])[0];
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
	unsigned char *dataCounterSignal;
	unsigned char *dataCounter;

	char lastCore;
	int lastElement;

	char lastCoreInternal;
	int lastElementInternal;

	//set the pointer to their variables
	ready = (char*)(COMMADDRESS_READY);
	inputData = (char*)(COMMADDRESS_INPUT);
	dataCounterSignal = (char*)(COMMADDRESS_COUNTER_SIGNAL);
	dataCounter = (char*)(COMMADDRESS_COUNTER);

	int i;
    short element = -1;
    short internalElement=-1;
    char parityCheck = 0;
    unsigned char bitPosition;
    unsigned int bytePosition;
    unsigned char selectedBit;
    unsigned char bitValue;

    char algorithmEnd = 0;

	while(1){


		lastCore = -1;
		lastElement = -1;
		element = -1;
		parityCheck = 0;

		lastCoreInternal = -1;
		lastElementInternal = -1;
		internalElement = -1;

		while(!(*ready));

        while(1) {
            while(1){


                element = getNextElement(&lastCore,&lastElement);

                if(element == 0){
                    //go to correction code
                    break;
                }else if(element == -1){
                    //finish line parity check
                    internalElement = getNextElement(&lastCoreInternal,&lastElementInternal);
                    while(internalElement!= (short)-1){
                        bitPosition = (internalElement-1)%8;
                        bytePosition = (internalElement-1)/8;
                        selectedBit = 1<<bitPosition;
                        bitValue = (selectedBit&inputData[bytePosition])>>bitPosition;

                        dataCounter[internalElement-1] += parityCheck?(!bitValue - bitValue) : (bitValue - !bitValue);
                        internalElement = getNextElement(&lastCoreInternal,&lastElementInternal);

                    }
                    parityCheck = 0;

                }else{
                    //line parity check
                    bitPosition = (element-1)%8;
                    bytePosition = (element-1)/8;
                    selectedBit = 1<<bitPosition;

                    parityCheck ^= (selectedBit&inputData[bytePosition])>>bitPosition;

                    if(parityCheck){
                        algorithmEnd = 1;
                    }

                }

            }


            //if all parity is right break the decoding loop
            if(!algorithmEnd){
                for(i=0; i<COUNTER_SIZE; i++){
                    dataCounter[i] = 0x00;
                }
                break;
            }

            algorithmEnd = 0;
            //create the new message to redo the algorithm.

            for(i=0; i<COUNTER_SIZE; i++){
                bytePosition= i/8;
                bitPosition = i%8;
                selectedBit = 1<<bitPosition;

                if(dataCounter[i] == (char)0){
                    inputData[bytePosition] &= selectedBit;
                }else if(dataCounter[i]&0x80){
                    inputData[bytePosition] &= (~selectedBit);
                }else{
                    inputData[bytePosition] |= selectedBit;
                }
                dataCounter[i] = 0;
            }
            lastCore = -1;
            lastElement = -1;
            lastCoreInternal = -1;
            lastElementInternal = -1;

        }

        //end of decoding
		*ready = 0;

	}

	return EXIT_SUCCESS;
}
