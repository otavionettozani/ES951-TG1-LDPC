#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <e-hal.h>
#include <math.h>
#include <sys/time.h>
#include "messages.h"

#define RAM_SIZE (0x8000)
#define VECTOR_SIZE (524288)

#define DATA_TO_SEND (32)


void clearMemory(){
	
	e_epiphany_t dev;
	char clear[0x6000] = {0};
	unsigned int i,j;
	
	for(i=0;i<4;i++){
		for(j=0;j<4; j++){
			e_open(&dev,i,j,1,1);
			e_reset_group(&dev);
			e_write(&dev,0,0,COMMADDRESS_READY,clear,0x6000*sizeof(char));
			e_close(&dev);
		}
	}
	usleep(10000);
	
}



int main(){
	
	//EPIPHANY VARIABLES
	e_platform_t platform;
	e_epiphany_t dev;
	
	//DEBUG VARIABLES
	unsigned read_buffer[RAM_SIZE/4];
	unsigned read_data;
	unsigned addr;
	int i,j,k;
	
	char filename[9] = "logs.txt";
	FILE* file = fopen(filename,"w");
	
	//TIME VARIABLEs
	struct timeval initTime, endTime;
	long int timediff;
	
	//LDPC VARIABLES
	FILE* sparseMatrix = fopen("../H.txt", "r");
	short *matrix = (short*) malloc (H_MATRIX_SIZE*sizeof(short));
	char data[DATA_TO_SEND][DATA_SIZE];
	char answer[DATA_TO_SEND][DATA_SIZE];
	
	//control variables
	char lastStates[4][4] = {{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0}};
	char dataInCores[4][4] = {{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0}};
	
	//read matrix H
	int line, col, lastLine = 1, ignore, read_scan;
	for(i=0; i<H_MATRIX_SIZE; i++){
		read_scan = fscanf(sparseMatrix, "%d %d %d",&col,&line, &ignore);
		if(read_scan != 3){
			matrix[i] = -1;
			continue;
		}
		if(line!=lastLine){
			matrix[i]=-1;
			i++;
			lastLine=line;
		}
		matrix[i]=col;
	}
	
	//initialize the cores
	e_init(NULL);
	e_get_platform_info(&platform);
	
	clearMemory();
	
	e_open(&dev, 0,0,4,4);
	
	e_reset_group(&dev);
	
	//copy the matrix to the cores memory and set its size
	int copyMatrixSize=H_MATRIX_SIZE/16;
	int offsetOfMatrix = 0;
	for(i = 0; i<4; i++){
		for(j=0; j<4; j++){
			if(i==3 && j==3){
				copyMatrixSize = H_MATRIX_SIZE - 15*copyMatrixSize;
			}
			e_write(&dev,i,j,COMMADDRESS_MATRIX,&matrix[offsetOfMatrix],copyMatrixSize*sizeof(short));
			e_write(&dev,i,j,COMMADDRESS_SIZE,&copyMatrixSize,sizeof(int));
			offsetOfMatrix+=copyMatrixSize;
		}
	}
	
	
	
	//start programs
	for(i=0; i<4; i++){
		for(j=0;j<4;j++){
			e_load("epiphanyProgram.srec",&dev,i,j,E_TRUE);
		}
	}
	
	
	i = 0;
	j = 0;
	char readReady;
	char bitOn = 1;
	char lastSentData =0;
	char lastReadData =0;
	char receivedData =0;
	//TIME MEASUREMENT HERE
	
	//loop to send data
	
	
	while(1){
		
		//see if needs to send new data
		e_read(&dev,i,j,COMMADDRESS_READY,&readReady,sizeof(char));
		
		if(readReady == 0 && lastStates[i][j] == 0 && lastSentData < DATA_TO_SEND){
			//send new data
			e_write(&dev,i,j,COMMADDRESS_INPUT,&data[lastSentData][0],DATA_SIZE*sizeof(char));
			//send start signal
			e_write(&dev,i,j,COMMADDRESS_READY,&bitOn,sizeof(char));
			
			//update control variables
			lastStates[i][j]=1;
			dataInCores[i][j] = lastSentData;
			lastSentData++;
			
			//see if needs to read data
		}else if(readReady == 0 && lastStates[i][j]==1){
			//read data
			int dataLocation = dataInCores[i][j];
			e_read(&dev,i,j,COMMADDRESS_OUTPUT,&answer[dataLocation][0],DATA_SIZE*sizeof(char));
			
			//update control variables
			lastStates[i][j]=0;
			receivedData++;
			
			if(receivedData == DATA_TO_SEND){
				break;
			}
		}
		
		printf("%d,%d->%d\n",i,j,receivedData);
		
		//update loop variables
		j = i==3? (j==3 ? 0 : j+1): j;
		i = i==3? 0: i+1;
		
	}
	
	
	//END OF TIME MEASUREMENT
	
	
	//-------------------------DUMP MEMORY -----------------------------
	//read all memory
	e_open(&dev, 0, 0, platform.rows, platform.cols);
	fprintf(file,"(ROW,COL)   ADDRESS   DATA\n");
	fprintf(file,"-----------------------------\n");
	for (i=0; i<(platform.rows); i++) {
		for (j=0; j<(platform.cols); j++) {
			for(k=0;k<RAM_SIZE/4;k++){
				addr=4*k;
				e_read(&dev, i, j, addr, &read_data, sizeof(int));
				read_buffer[k]=read_data;
			}
			for(k=0;k<RAM_SIZE/4;k++){
				fprintf(file,"(%2d,%2d)     0x%08x   0x%08x\n",i,j,k*4,read_buffer[k]);
			}
		}
	}
	
	
	fclose(file);
	e_close(&dev);
	e_finalize();
	
	
	free(matrix);
	return EXIT_SUCCESS;
	
}