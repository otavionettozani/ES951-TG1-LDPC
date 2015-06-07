#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "e_lib.h"

#include "messages.h"


int main(void){


	unsigned char *dataReceived;
	unsigned int *size;
	unsigned char *ack;
	unsigned char *busy;
	unsigned char *dataSent;
	unsigned char *armAck;
	unsigned int *data;

	//set the pointer to their variables
	size = (int*)(COMMADDRESS_SIZE);
	ack = (char*)(COMMADDRESS_EPIPHANY_ACK);
	dataReceived = (char*)(COMMADDRESS_DATA_TO_EPIPHANY);
	busy = (char*)(COMMADDRESS_BUSY);
	dataSent = (char*)(COMMADDRESS_DATA_TO_ARM);
	armAck = (char*)(COMMADDRESS_ARM_ACK);
	data = (int*)(COMMADDRESS_DATA+CORE_0_0_MEM);

	//temporary
	size[0] = e_get_coreid();
	size[2] = data;
	data[0] = e_get_coreid();

	return EXIT_SUCCESS;

	*dataSent = 0;
	//set the core as busy
	*busy = 1;
	//wait for signal that all data has been transfered
	while(!dataReceived[0]);
	//acknowledge receiving the data
	*ack = 1;
	//reset receive data bit
	*dataReceived = 0;

//--------------
//do all that matters


//----------------


	//up data sent flag
	*dataSent = 1;
	//down own ack

	//wait for arm to ack
	while(!armAck[0]);

	*ack = 0;

	//reset busy flag
	*busy = 0;

	*armAck = 0;


	return EXIT_SUCCESS;
}
