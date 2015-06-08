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
	
	
	if(*lastCoreElement>=size){
		if(*lastCore == 16){
			return 0;
		}
		(*lastCore)++;
		size = *(coreSize[*lastCore]);
		*lastCoreElement = 0;
	}else{
		(*lastCoreElement)++;
	}
	
	return (coreMatrix[*lastCore])[*lastCoreElement];
	
}

int main(void){
	
	
	unsigned char *ready;
	unsigned int *inputData;
	unsigned char *outputData;
	
	char lastCore;
	int lastElement;
	
	//set the pointer to their variables
	ready = (char*)(COMMADDRESS_READY);
	inputData = (int*)(COMMADDRESS_INPUT);
	outputData = (char*)(COMMADDRESS_OUTPUT);
	
	while(1){
		
		lastCore = -1;
		lastElement = -1;
		
		while(!(*ready));
		
		while(getNextElement(&lastCore,&lastElement));
		inputData[0] = lastCore;
		inputData[8] = lastElement;
		
		
		*ready = 0;
		
	}
	
	return EXIT_SUCCESS;
}