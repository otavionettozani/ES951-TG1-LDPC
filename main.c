#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <e-hal.h>
#include <math.h>
#include <sys/time.h>
#include "messages.h"

#define RAM_SIZE (0x8000)

#define DATA_TO_SEND (16)


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
	FILE* sparseMatrix = fopen("../Ho.txt", "r");
	short *matrix = (short*) malloc (H_MATRIX_SIZE*sizeof(short));
	char data[DATA_TO_SEND][DATA_SIZE];
	char answer[DATA_TO_SEND][DATA_SIZE];
	FILE* entryData = fopen("../vector.txt", "r");
	unsigned char originalData[DATA_SIZE] = {0};
	int readOriginalData;
    for(i=0; i<DATA_SIZE;i++){
        fscanf(entryData,"%d",&readOriginalData);
        originalData[i] = (char)readOriginalData;
    }

    fclose(entryData);


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

	fclose(sparseMatrix);

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

	//create data
    for(i=0;i<DATA_TO_SEND;i++){
        for(j=0;j<DATA_SIZE;j++){
            data[i][j] = originalData[j];
        }
        data[i][0] ^= 0x01;
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
    gettimeofday(&initTime,NULL);
    timediff = initTime.tv_sec*1000000+initTime.tv_usec;
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
            //printf("Data Sent->%d Cores->%d,%d\n",lastSentData,i,j);
			//see if needs to read data
		}else if(readReady == 0 && lastStates[i][j]==1){
			//read data
			int dataLocation = dataInCores[i][j];
			e_read(&dev,i,j,COMMADDRESS_INPUT,&answer[dataLocation][0],DATA_SIZE*sizeof(char));

			//update control variables
			lastStates[i][j]=0;
			receivedData++;

            //printf("Data Received->%d Core->%d %d\n",dataLocation+1,i,j);
			if(receivedData >= DATA_TO_SEND-1){
				break;
			}
		}


		//update loop variables
		j = i==3? (j==3 ? 0 : j+1): j;
		i = i==3? 0: i+1;

	}

    //END OF TIME MEASUREMENT
    gettimeofday(&endTime,NULL);
    timediff = endTime.tv_sec*1000000+endTime.tv_usec - timediff;
    double timeInSeconds = timediff;
    timeInSeconds /=1000000;
    printf("time:%g\n",timeInSeconds);


    for(i=0; i<DATA_SIZE; i++){
        fprintf(file,"0x%04x,",answer[0][i]);
    }
    fprintf(file,"\n");
    for(i=0; i<DATA_SIZE; i++){
        fprintf(file,"0x%04x,",originalData[i]);
    }
    fprintf(file,"\n");

    char test;
    for(i=0;i<DATA_TO_SEND;i++){
        test = 0;
        for(j=0;j<DATA_SIZE;j++){
            if(answer[i][j]!=originalData[j]){
                printf("wrong byte: %d->",j+1);
                test=1;
                break;
            }
        }
        if(test){
            printf("error at core:%d\n",i+1);
        }else{
            printf("ok at:%d\n",i+1);
        }
    }




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
