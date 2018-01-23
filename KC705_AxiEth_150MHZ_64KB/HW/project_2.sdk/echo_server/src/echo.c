/******************************************************************************
*
* Copyright (C) 2016 Xilinx, Inc.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* Use of the Software is limited solely to applications:
* (a) running on a Xilinx device, or
* (b) that interact with a Xilinx device through a bus or interconnect.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
******************************************************************************/

#include <stdio.h>
#include <string.h>

#include "lwip/sockets.h"
#include "netif/xadapter.h"
#include "lwipopts.h"
#include "xil_printf.h"
#include "FreeRTOS.h"
#include "task.h"
#include "xparameters.h"
#include "xgpio_l.h"
#include "xil_printf.h"
#include "xstatus.h"
#include "xllfifo.h"
#include "xstreamer.h"
#include <xiic.h>
#include "register_map_vfat3.h"
//#include<cstdlib.h>
#define THREAD_STACKSIZE 1024

#define HDLC_FS                	0X7E
#define HDLC_ADDRESS		   	0X00
#define HDLC_CONTROL           	0x03

#define SC0           			0x96
#define SC1           			0x99


    #define TRANSACTION_ID 		    0x05

    #define TRANS_TYPE_RD  		0x0	// Read
	#define TRANS_TYPE_WR  		0x1		// Write
	#define TRANS_TYPE_RDN  	0x2		// Non-incrementing Read
	#define TRANS_TYPE_WRN  	0x3		// Non-incrementing Write
	#define TRANS_TYPE_RMWB  	0x4		// Read/Modify/Write Bits
	#define TRANS_TYPE_RMWS  	0x5		// Read/Modify/Write Sum

	#define TRANS_TYPE_RSVD  	0xe		// Get Reserved Address Information
	#define TRANS_TYPE_INFO  	0xf		// Byte-order/Idle

	// IPBUS Info Codes
	#define INFO_CODE_OK  		0x0		// Request handled successfully by host
	#define INFO_CODE_BAD_HDR  	0x1		// Bad header
	#define INFO_CODE_RBE  		0x2		// Bus error on read
	#define INFO_CODE_WBE  		0x3		// Bus error on write
	#define INFO_CODE_RTO  		0x4		// Bus timeout on read
	#define INFO_CODE_WTO  		0x5		// Bus timeout on write
	#define INFO_CODE_OVERFLOW 	0x6		// TX Buffer overflow
	#define INFO_CODE_UNDERFLOW 0x7		// RX Buffer underflow
	#define INFO_CODE_REQ 	 	0xf		// Outbound request

	// IPBUS Current Header version
	#define HEADER_VERSION 		0x2
#define DEPTH				    0X1
u16_t echo_port = 7;
#define READ 0X00
#define FIFO_DEV_ID	   	XPAR_AXI_FIFO_0_DEVICE_ID
//#define FIFO_DEV_ID_rx	   	XPAR_AXI_FIFO_0_DEVICE_ID

#define WORD_SIZE 4			/* Size of words in bytes */

#define MAX_PACKET_LEN 4

#define NO_OF_PACKETS 16000//16250

#define MAX_DATA_BUFFER_SIZE NO_OF_PACKETS*MAX_PACKET_LEN
//#define FIFO_DEV_ID	   	XPAR_AXI_FIFO_0_DEVICE_ID // receiver fifo
//#define FIFO_DEV_ID	   	XPAR_AXI_FIFO_1_DEVICE_ID // transmitter fifo

/************************** Function Prototypes ******************************/
#ifdef XPAR_UARTNS550_0_BASEADDR
static void Uart550_Setup(void);
#endif
/************************** Variable Definitions *****************************/
/*
 * Device instance definitions
 */
XLlFifo FifoInstance;
XIic Iic;
/*****************************************************************************/
void check_fifo(u32  *SourceAddr,u32 data_len,u8 verbose_mode);
int XLlFifoPollingExample(XLlFifo *InstancePtr, u16 DeviceId,u32  *SourceAddr,u32 data_len,u8 verbose_mode);
int TxSend(XLlFifo *InstancePtr, u32 *SourceAddr,u32 data_len,u8 verbose_mode);
u32 RxReceive(XLlFifo *InstancePtr, u32 *DestinationAddr,u8 verbose_mode);
int send_sync_command(XLlFifo *InstancePtr, u16 DeviceId,u32  *SourceAddr,int sd,u8 verbose_mode);
int Initialize_fifo(XLlFifo *InstancePtr, u16 DeviceId);
void Initialize_everything();
int decode_receive_packet(u8 *recv_buf,u32 *SourceAddr,int sd);
unsigned int crc16(char *data_p, unsigned short length);
void HDLC_Tx(XLlFifo *InstancePtr,u32 register_address, u32 register_data,u8 mode,u32* SourceAddr,u8 verbose_mode);
u32 HDLC_Rx(XLlFifo *InstancePtr,u32* Destination_Addr,u32 rcv_len,u8 mode,u8 verbose_mode);
int CalibrateADC(u32 *SourceAddr,u32 *DestinationBuffer,u8 calibration_mode);
int CalibrateCAL_DAC(u32 *SourceAddr,u32 *DestinationBuffer,u8 calibration_mode);
signed configureADC(u8 channel);
//signed short ReadADC();
signed short ReadADC();
u32 AdjustIref(u32 *SourceAddr,u32 *DestinationBuffer);
int send_Calpulse_LV1As(XLlFifo *InstancePtr, u16 DeviceId,u32  *SourceAddr,u16 data_len,u8 verbose_mode);//
int Scurve(u32 *SourceAddr,u32 *DestinationBuffer,u16 Latency, u16 num_of_triggers,u8 calibration_mode);
int DecodeDataPacket(XLlFifo *InstancePtr, u16 DeviceId,u16 Latency,u8 verbose_mode);
int Transmit_fast_command(XLlFifo *InstancePtr, u16 DeviceId,u32  *SourceAddr,u16 data_len,u8 verbose_mode);
void SendReply(int sd,u32 *Buffer, u32 length);
/***************** Macros (Inline Functions) Definitions *********************/
//int send_sync_command(XLlFifo *InstancePtr_tx);

u32 SourceBuffer[MAX_DATA_BUFFER_SIZE * WORD_SIZE];
u32 DestinationBuffer[MAX_DATA_BUFFER_SIZE * WORD_SIZE];
u32 ReceiveLength;
u32 MON_SEL_BITS	=	0;
u32 MON_GAIN_BITS	=	0;
u32 VREF_ADC_BITS	=	0;
u32 CAL_DAC;
u8 datapacket[27];
u8 Error_Dpkt;
u32 Hit_array[128][256]={0};
u8 mask_array[8] = {0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80};
u32 DATA_GBL_CFG_CAL_0;
//XLlFifo *InstancePtr;
XLlFifo_Config *Config;
char TYPE_ID;


//int sd;
#define RECV_BUF_SIZE  2048
	u8 recv_buf[RECV_BUF_SIZE];
	int n, nwrote;


//unsigned short DEPTH=0X01;
union{
			unsigned short crc;

			struct{
			u8 crc_lsb;
			u8 crc_msb;
				  }st1;
			}ut1;

//IP_BUS_HEADER

			//u8 TxMsgPtr []={0x01,0x81,0x00};//default data rate
			u8 TxMsgPtr []={0x01,0xd4,0xE3};//max data rate   byte 1 = 0xc4 ch0, 0xd4 ch1
				u8 TxMsgPtr_conversion []={0x00};
				u8 TxMsgPtr_config[]={0x01};
				u8 RcvBuffer [2]={0x00,0x00};
				unsigned Num_Bytes_received;
				u16 config_register;
				//XIic * 	InstancePtr;

				int ByteCount=sizeof(TxMsgPtr);

int     crc_ok = 0x470F;

u8 reverse(u8 b) {
   b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
   b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
   b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
   return b;
}
u8 flag=0;
int send_sync_command(XLlFifo *InstancePtr, u16 DeviceId,u32  *SourceAddr,int sd,u8 verbose_mode)// sends 03 sync commands to the chip hard sync

{
	int Status;

	u32 i;
	u8 success;
	u32 Rcv_len=0;
	u32 data_len=30;
//u8 verbose_mode


#ifdef 	XPAR_REVERSE_RXD_RX_REVERSE_BASEADDR
	#ifdef	XPAR_INVERT_RXD_RX_INVERSE_BASEADDR
		#ifdef XPAR_INVERT_TXD_TX_INVERT_BASEADDR
			#ifdef XPAR_REVERSE_TXD_TX_REVERSE_BASEADDR
		XGpio_WriteReg((XPAR_REVERSE_RXD_RX_REVERSE_BASEADDR),4, 0);
		XGpio_WriteReg((XPAR_REVERSE_RXD_RX_REVERSE_BASEADDR),0, 1);

		XGpio_WriteReg((XPAR_INVERT_RXD_RX_INVERSE_BASEADDR),4, 0);
		XGpio_WriteReg((XPAR_INVERT_RXD_RX_INVERSE_BASEADDR),0, 1);

		XGpio_WriteReg((XPAR_INVERT_TXD_TX_INVERT_BASEADDR),4, 0);
		XGpio_WriteReg((XPAR_INVERT_TXD_TX_INVERT_BASEADDR),0, 1);

		XGpio_WriteReg((XPAR_REVERSE_TXD_TX_REVERSE_BASEADDR),4, 0);
		XGpio_WriteReg((XPAR_REVERSE_TXD_TX_REVERSE_BASEADDR),0, 1);

		//xil_printf("\r\n transceiver direction reversed and inverted \r\n");
			#endif
		#endif
	#endif
#endif
//////////////////////end setting vbrad board reqirements////////////////////////



		xil_printf("\n\r--- Entering send_sync_command() ---\n\r");
		//xil_printf("\n\r sourceBuffer  address = %p \n\r",(void*)SourceBuffer);
		//xil_printf("\n\r source_address = %p \n\r",(void*)SourceAddr);
		for (i=0;i<=data_len;i++)
				{
					 *(SourceAddr + i) = 0x17;
					//else if(i==(MAX_DATA_BUFFER_SIZE-1))*(SourceAddr + i) = 0xe8;

			//if(i>=0 && i< 3){*(SourceAddr + i) = 0x17;}//sync
			 //if (i>=500)
		//	 {*(SourceAddr + i) = 0xe8;}// syncack
			//else {*(SourceAddr + i) = 0xe8;}
				}
		//xil_printf("\n\r source address = %p \n\r",(void *)SourceAddr);
		//XLlFifo_Config *Config;

			//int Status;
		//	int i;
			int Error;
			Status = XST_SUCCESS;

			/* Initial setup for Uart16550 */
/*
		#ifdef XPAR_UARTNS550_0_BASEADDR

			Uart550_Setup();

		#endif
*/


#ifdef 		XPAR_BIT_SLIP_BASEADDR
			#ifdef 		XPAR_SUCCESS_BASEADDR
				//ch=!ch;
			//xil_printf("\r\n Aligning bits . ");
			do{


			XGpio_WriteReg((XPAR_BIT_SLIP_BASEADDR),	4, 0);//set as OUTPUT
			XGpio_WriteReg((XPAR_SUCCESS_BASEADDR),	4, 1);//set as INPUT

				//if(recv_buf[1]==0xaa)//bit slip pulse
				{
				XGpio_WriteReg((XPAR_BIT_SLIP_BASEADDR),0, 1);//rising edge of bitslip signal from microblaze
				XGpio_WriteReg((XPAR_BIT_SLIP_BASEADDR),0, 0);//

				xil_printf(".");
				}
						//xil_printf("Hi Henri i receive ur message\r\n %x,%x %x",recv_buf[0],recv_buf[1],recv_buf[2],recv_buf[3]);
				usleep(1000);usleep(1000);usleep(1000);
				success = XGpio_ReadReg((XPAR_SUCCESS_BASEADDR),0);
				XLlFifo_Reset(&FifoInstance);
			}while(success!=1);

		#endif
	#endif
			xil_printf("\n\r--- Initializing fifo ---\n\r");

					Status = Initialize_fifo(&FifoInstance, FIFO_DEV_ID);
					if (Status != XST_SUCCESS) {
						xil_printf("Axi Streaming TX FIFO Initialization failed\n\r");
						xil_printf("--- Exiting main() ---\n\r");
						return XST_FAILURE;
					}

					xil_printf("Successfully Initialized Axi Streaming FIFO \n\r");
					//xil_printf("--- Exiting check_fifo() ---\n\r");

					//return XST_SUCCESS;
			/* Initialize the Device Configuration Interface driver */
			Config = XLlFfio_LookupConfig(DeviceId);
			if (!Config) {
				xil_printf("No config found for %d\r\n", DeviceId);
				return XST_FAILURE;
			}



			/*
			 * This is where the virtual address would be used, this example
			 * uses physical address.
			 */
			Status = XLlFifo_CfgInitialize(&FifoInstance, Config, Config->BaseAddress);
			if (Status != XST_SUCCESS) {
				xil_printf("Initialization failed\n\r");
				return Status;
			}




			/* Check for the Reset value */
			Status = XLlFifo_Status(&FifoInstance);
			XLlFifo_IntClear(&FifoInstance,0xffffffff);
			Status = XLlFifo_Status(&FifoInstance);
			if(Status != 0x0) {
				xil_printf("\n ERROR : Reset value of ISR0 : 0x%x\t"
					    "Expected : 0x0\n\r",
					    XLlFifo_Status(&FifoInstance));
				return XST_FAILURE;
			}


			XLlFifo_Reset(&FifoInstance);

			xil_printf("vacancy= %d\r\n",XLlFifo_iTxVacancy(&FifoInstance));
			/* Transmit the Data Stream */
			Status = TxSend(&FifoInstance, SourceAddr,data_len,verbose_mode);
			if (Status != XST_SUCCESS){
				xil_printf("Transmisson of Data failed\n\r");
				return XST_FAILURE;
			}





			/* Revceive the Data Stream */
			Rcv_len = RxReceive(&FifoInstance, DestinationBuffer,verbose_mode);
			if (Status != XST_SUCCESS){
				xil_printf("Receiving data failed");
				return XST_FAILURE;
			}

			Error = 0;
			u16 sync_ok=0;
			if(Rcv_len<MAX_DATA_BUFFER_SIZE)
			{
			for(i=0;i<Rcv_len;i++)
				if((u8)*(DestinationBuffer+i)== 0x3a ) sync_ok=1; else sync_ok=0;
			if(sync_ok==1)
			xil_printf("Sync Ok\r\n ");
			else
			xil_printf("NO Sync \r\n ");
			}
			else
			{
				xil_printf("No sync\r\n");
				XLlFifo_Reset(&FifoInstance);
			}

			/* handle request
					if ((nwrote = write(sd, DestinationBuffer, Rcv_len)) < 0) {
						xil_printf("%s: ERROR responding to client echo request. received = %d, written = %d\r\n",
								__FUNCTION__, n, nwrote);
						xil_printf("Closing socket %d\r\n", sd);
					}*/
			/* Compare the data send with the data received */
			//xil_printf(" destination buffer[0] = %x \n\r",DestinationBuffer[0]);
			//for( i=0 ; i<MAX_DATA_BUFFER_SIZE ; i++ ){
			//	if ( *(SourceBuffer + i) != *(DestinationBuffer + i) ){
			//		Error = 1;
			//		break;
			//	}
			//return Status;
		/*	///////////////////ALIGN LVDS RECEIVER BLOCK/////////////////

		#ifdef 		XPAR_BIT_SLIP_BASEADDR
			#ifdef 		XPAR_SUCCESS_BASEADDR
				//ch=!ch;
			//xil_printf("\r\n Aligning bits . ");
			do{


			XGpio_WriteReg((XPAR_BIT_SLIP_BASEADDR),	4, 0);//set as OUTPUT
			XGpio_WriteReg((XPAR_SUCCESS_BASEADDR),	4, 1);//set as INPUT

				//if(recv_buf[1]==0xaa)//bit slip pulse
				{
				XGpio_WriteReg((XPAR_BIT_SLIP_BASEADDR),0, 1);//rising edge of bitslip signal from microblaze
				XGpio_WriteReg((XPAR_BIT_SLIP_BASEADDR),0, 0);//

				xil_printf(".");
				}
						//xil_printf("Hi Henri i receive ur message\r\n %x,%x %x",recv_buf[0],recv_buf[1],recv_buf[2],recv_buf[3]);
				usleep(1000);usleep(1000);usleep(1000);
				success = XGpio_ReadReg((XPAR_SUCCESS_BASEADDR),0);

			}while(success!=1);
			xil_printf("\r\n Aligning  done successfully\r\n");
			#endif
		#endif
		/////////////////END BIT ALIGNMENT BLOCK////////////////////*/


			XLlFifo_Reset(&FifoInstance);
			configureADC(0);
			SendReply(sd,DestinationBuffer,Rcv_len);
		return Rcv_len;

}

int Transmit_fast_command(XLlFifo *InstancePtr, u16 DeviceId,u32  *SourceAddr,u16 data_len,u8 verbose_mode)
{
	u8 Status;
	/* Transmit the Data Stream */
				Status = TxSend(&FifoInstance, SourceAddr,data_len,verbose_mode);
				if (Status != XST_SUCCESS){
					xil_printf("Transmisson of Data failed\n\r");
					return XST_FAILURE;
				}


return 0;
}



int send_Calpulse_LV1As(XLlFifo *InstancePtr, u16 DeviceId,u32  *SourceAddr,u16 data_len,u8 verbose_mode)//

{
	int Status;
	//XLlFifo_Config *Config;
	u32 i;
//	u8 success;
//	u32 Rcv_len=0;
	//u32 data_len;
//u8 verbose_mode

u32 j;



			//int Error;
			Status = XST_SUCCESS;





			////////////fill the tx buffer with cal pulse and lv1as
			/*for(i=0;i<num_of_triggers;i++)
			{
			for(j=0;j<=(Latency+1);j++)
							{
								//	if(j==0)
								//	*(SourceAddr + j) = RUN_MODE;//FAST COMMAND
								 if(j==0)
									*((SourceAddr + j) + i*(Latency+2)) = CAL_PULSE;//FAST COMMAND

								else if(j==(Latency+1))
									*((SourceAddr + j) + i*(Latency+2)) = LV1A;

								else if (j % 2==0)
									*((SourceAddr + j) + i*(Latency+2)) = 0x00;

								else
									*((SourceAddr + j) + i*(Latency+2)) = 0xff;

							//xil_printf("index=%d SourceBuffer=%x\r\n",(j + i*(Latency+2)),*((SourceAddr + j) + i*(Latency+2)) );

							}
			}*/

			//for(i=0;i<num_of_triggers;i++)
				//		{
						for(j=0;j<data_len;j++)
										{
											//	if(j==0)
											//	*(SourceAddr + j) = RUN_MODE;//FAST COMMAND
											 if(j==0)
												*(SourceAddr + j)  = CAL_PULSE;//FAST COMMAND

											else if(j==1)
												*(SourceAddr + j) = LV1A;

											else if (j % 2==0)
												*(SourceAddr + j) = 0x00;

											else
												*(SourceAddr + j)  = 0xff;

										//xil_printf("index=%d SourceBuffer=%x\r\n",(j + i*(Latency+2)),*((SourceAddr + j) + i*(Latency+2)) );

										}
						//}







			//data_len = ( (Latency+2));//*num_of_triggers);
		//	xil_printf("data tx length = %d \r\n",data_len);
			/* Transmit the Data Stream */
			Status = TxSend(&FifoInstance, SourceAddr,data_len,verbose_mode);
			if (Status != XST_SUCCESS){
				xil_printf("Transmisson of Data failed\n\r");
				return XST_FAILURE;
			}




		return XST_SUCCESS;

}

void Initialize_everything()

{
	//int i=0;
	int Status;
	u8 success;
flag=1;

		xil_printf("\n\r--- Initializing fifo ---\n\r");

		Status = Initialize_fifo(&FifoInstance, FIFO_DEV_ID);
		if (Status != XST_SUCCESS) {
			xil_printf("Axi Streaming TX FIFO Initialization failed\n\r");
			xil_printf("--- Exiting main() ---\n\r");
			return XST_FAILURE;
		}

		xil_printf("Successfully Initialized Axi Streaming FIFO \n\r");
		//xil_printf("--- Exiting check_fifo() ---\n\r");

		//return XST_SUCCESS;







/*
	/////////////////////////
#ifdef 		XPAR_TRANSMITTER_LOGIC_OUT_MUX_CONTROL_BASEADDR
		//ch=!ch;
		XGpio_WriteReg((XPAR_TRANSMITTER_LOGIC_OUT_MUX_CONTROL_BASEADDR),
				4, 0);

		XGpio_WriteReg((XPAR_TRANSMITTER_LOGIC_OUT_MUX_CONTROL_BASEADDR),
						0, 0);
		//xil_printf("\n\r receive packet %x %x %x %x\n \r",recv_buf[0],recv_buf[1],recv_buf[2],recv_buf[3],recv_buf[4],recv_buf[5]);
#endif
*/

////////////////////////////////////////////////////////////////////////

//Initialize_TX_fifo(XLlFifo *InstancePtr_tx, u16 DeviceId_tx);
//Initialize_RX_fifo();
///////////////// set transceivers according to vbrad board////////////////////////////

#ifdef 	XPAR_REVERSE_RXD_RX_REVERSE_BASEADDR
	#ifdef	XPAR_INVERT_RXD_RX_INVERSE_BASEADDR
		#ifdef XPAR_INVERT_TXD_TX_INVERT_BASEADDR
			#ifdef XPAR_REVERSE_TXD_TX_REVERSE_BASEADDR
		XGpio_WriteReg((XPAR_REVERSE_RXD_RX_REVERSE_BASEADDR),4, 0);
		XGpio_WriteReg((XPAR_REVERSE_RXD_RX_REVERSE_BASEADDR),0, 1);

		XGpio_WriteReg((XPAR_INVERT_RXD_RX_INVERSE_BASEADDR),4, 0);
		XGpio_WriteReg((XPAR_INVERT_RXD_RX_INVERSE_BASEADDR),0, 1);

		XGpio_WriteReg((XPAR_INVERT_TXD_TX_INVERT_BASEADDR),4, 0);
		XGpio_WriteReg((XPAR_INVERT_TXD_TX_INVERT_BASEADDR),0, 1);

		XGpio_WriteReg((XPAR_REVERSE_TXD_TX_REVERSE_BASEADDR),4, 0);
		XGpio_WriteReg((XPAR_REVERSE_TXD_TX_REVERSE_BASEADDR),0, 1);

		xil_printf("\r\n transceiver direction reversed and inverted \r\n");
			#endif
		#endif
	#endif
#endif
//////////////////////end setting vbrad board reqirements////////////////////////

	///////////////////ALIGN LVDS RECEIVER BLOCK/////////////////

#ifdef 		XPAR_BIT_SLIP_BASEADDR
	#ifdef 		XPAR_SUCCESS_BASEADDR
		//ch=!ch;
	xil_printf("\r\n Aligning bits . ");
	do{


	XGpio_WriteReg((XPAR_BIT_SLIP_BASEADDR),	4, 0);//set as OUTPUT
	XGpio_WriteReg((XPAR_SUCCESS_BASEADDR),	4, 1);//set as INPUT

		//if(recv_buf[1]==0xaa)//bit slip pulse
		{
		XGpio_WriteReg((XPAR_BIT_SLIP_BASEADDR),0, 1);//rising edge of bitslip signal from microblaze
		XGpio_WriteReg((XPAR_BIT_SLIP_BASEADDR),0, 0);//

		xil_printf(".");
		}
				//xil_printf("Hi Henri i receive ur message\r\n %x,%x %x",recv_buf[0],recv_buf[1],recv_buf[2],recv_buf[3]);
		usleep(1000);usleep(1000);usleep(1000);
		success = XGpio_ReadReg((XPAR_SUCCESS_BASEADDR),0);

	}while(success!=1);
	xil_printf("\r\n Aligning  done successfully\r\n");
	#endif
#endif
/////////////////END BIT ALIGNMENT BLOCK////////////////////


	return XST_SUCCESS;


}
unsigned short swapBytes(unsigned short x)
{
    return ( (x & 0x00ff)<<8 | (x & 0xff00)>>8 );
}


int decode_receive_packet(u8 *recv_buf,u32  *SourceAddr,int sd)
{
	int Status;int i;
	u8 verbose_mode=0;
	Status= XST_SUCCESS;
	char data_crc[]={0x00,0x03,0x1f,0x06,0x01,0x20,0x81,0x00,0x00,0x00,0x07,0x20,0x00,0x00};
	//char data_crc[]={0x00,0x03,0x0f,0x05,0x01,0x20,0x81,0x00,0x00,0x00};
		unsigned int crc;
		signed short RESULT;
		u16 Latency=6,num_of_triggers=100;
		u32 Read_data;
		u32 *RD ;
	//int RECV_BUF_SIZE = 2048;
	//u8 *recv_buf;
xil_printf("Inside decode packet\r\n");
xil_printf("%x , %x , %x, %x ", *recv_buf ,*(recv_buf+1),*(recv_buf+2),*(recv_buf+3));
//if(flag==0)
//{send_sync_command(&FifoInstance, FIFO_DEV_ID,SourceAddr);
//send_sync_command(&FifoInstance, FIFO_DEV_ID,SourceAddr);
//send_sync_command(&FifoInstance, FIFO_DEV_ID,SourceAddr);
//flag=1;


//}
if(*recv_buf==0xca){
	if (*(recv_buf+1)==0x00 && *(recv_buf+2)==0x00)
		xil_printf("In Fast command block \r\n");
	else if (*(recv_buf+1)==0x00 && *(recv_buf+2)==0x01){
		xil_printf("In Slow control mode\r\n");

		u32 register_address = *(recv_buf+7) | (*(recv_buf+6)<<8) | (*(recv_buf+5)<<16) | (*(recv_buf+4)<<24);
		u32 register_data = *(recv_buf+11) | (*(recv_buf+10)<<8) | (*(recv_buf+9)<<16) | (*(recv_buf+8)<<24);
		u8 mode = *(recv_buf+3);
		xil_printf("register_address=%x\r\n",register_address);
		xil_printf("register_data=%x\r\n",register_data);
		xil_printf("mode =%x\r\n",mode);
		HDLC_Tx(&FifoInstance,register_address , register_data,mode,SourceAddr,verbose_mode);
		Read_data = HDLC_Rx(&FifoInstance,DestinationBuffer,ReceiveLength,mode,verbose_mode);

		RD = &Read_data;
		xil_printf("HDLC_ read_data= %x\r\n" ,Read_data),
		SendReply(sd, RD,1 * 4);
	}


	else if (*(recv_buf+1)==0x00 && *(recv_buf+2)==0x02){
			xil_printf("\n\rHard reset vfat3\r\n");
			send_sync_command(&FifoInstance, FIFO_DEV_ID,SourceAddr,sd,verbose_mode);
			//send_sync_command(&FifoInstance, FIFO_DEV_ID,SourceAddr,verbose_mode);
			//send_sync_command(&FifoInstance, FIFO_DEV_ID,SourceAddr,verbose_mode);


	}
	else if (*(recv_buf+1)==0x00 && *(recv_buf+2)==0x03)
		{
			xil_printf("\n\rIn external ADC Read mode\r\n");
			i=0;
			do{
			RESULT=ReadADC();
			Read_data = RESULT;
			RD = &Read_data;
			SendReply(sd, RD,1 * 4);
				printf("Result = %f mv\r\n",(float)(RESULT*0.0625));
				i++;
			}while(i<1);
		}

	else if (*(recv_buf+1)==0x00 && *(recv_buf+2)==0x04){
				xil_printf("\n\rEntering crc checking function\r\n");

	crc=crc16(data_crc,sizeof(data_crc));
	crc=swapBytes((unsigned short)crc);
	xil_printf("crc=%04x, size= %d",crc,sizeof(data_crc));
	}

	else if (*(recv_buf+1)==0x00 && *(recv_buf+2)==0x05){
					xil_printf("\n\rEntering in Iref adjustment mode function\r\n");
					Read_data = AdjustIref(SourceAddr,DestinationBuffer);
					RD = &Read_data;
					SendReply(sd, RD,1 * 4);


		}

	else if (*(recv_buf+1)==0x00 && *(recv_buf+2)==0x06){
						xil_printf("%c[2J",27);//clear terminal
						//clrscr();
						xil_printf("\n\rEntering in INTERNAL ADC calibration routine\r\n");
						CalibrateADC(SourceAddr,DestinationBuffer,0);


			}
	else if (*(recv_buf+1)==0x00 && *(recv_buf+2)==0x07){
							xil_printf("%c[2J",27);//clear terminal
							//clrscr();
							xil_printf("\n\rEntering in cal_dac calibration routine\r\n");
							CalibrateCAL_DAC(SourceAddr,DestinationBuffer,0);


				}



	else if (*(recv_buf+1)==0x00 && *(recv_buf+2)==0x08)
				{
							xil_printf("%c[2J",27);//clear terminal
							//clrscr();
							xil_printf("\n\rEntering in scurve routine\r\n");

							Latency = ( *(recv_buf+3) <<8) | *(recv_buf+4);
							num_of_triggers = ( *(recv_buf+5) <<8) | *(recv_buf+6);
							xil_printf("Latency= %d, Num_of_triggers LV1As = %d\r\n",Latency, num_of_triggers );
							Scurve(SourceAddr,DestinationBuffer,Latency,num_of_triggers,0);


				}


	else {
		xil_printf("no match\r\n");
		xil_printf("%c\r\n",0x0c);
		xil_printf("data = %x %x %x %x\r\n",*recv_buf,*(recv_buf+1),*(recv_buf+2),*(recv_buf+3));
	}

}
return Status;
}
int Initialize_fifo(XLlFifo *InstancePtr, u16 DeviceId)
{
	int Status;
	//XLlFifo_Config *Config;
	//	XLlFifo_Config *Config_rx;
		//int Status;
		int i;
		int Error;
		Status = XST_SUCCESS;

	/* Initialize the Device Configuration Interface driver */
		Config = XLlFfio_LookupConfig(DeviceId);
		if (!Config) {
			xil_printf("No config found for %d\r\n", DeviceId);
			return XST_FAILURE;
		}




		/*
		 * This is where the virtual address would be used, this example
		 * uses physical address.
		 */
		Status = XLlFifo_CfgInitialize(&FifoInstance, Config, Config->BaseAddress);
		if (Status != XST_SUCCESS) {
			xil_printf("Initialization failed\n\r");
			return Status;
		}
		/* Check for the Reset value */
			Status = XLlFifo_Status(&FifoInstance);
			XLlFifo_IntClear(&FifoInstance,0xffffffff);
			Status = XLlFifo_Status(&FifoInstance);
			if(Status != 0x0) {
				xil_printf("\n ERROR : Reset value of ISR0 : 0x%x\t"
					    "Expected : 0x0\n\r",
					    XLlFifo_Status(&FifoInstance));
				return XST_FAILURE;
			}
			return XST_SUCCESS;
}
u32 RxReceive (XLlFifo *InstancePtr, u32* DestinationAddr,u8 verbose_mode)
{

	int i;
	int Status;
	u32 RxWord;
	u32 ReceiveLength;
	//u8 verbose_mode=1;
	//static u32 frame_len;
	//if(verbose_mode)xil_printf(" Receiving data ....\n\r");

	/* while (XLlFifo_RxOccupancy(&InstancePtr)) {
	                frame_len = XLlFifo_RxGetLen(&RxInstance);
	                while (frame_len) {
	                        unsigned bytes = min(sizeof(buffer), frame_len);
	                        XLlFifo_Read(&InstancePtr, buffer, bytes);
	                        // ********
	                        // do something with buffer here
	                        xil_printf(" Received data = %x \n\r",);
	                        // ********
	                        frame_len -= bytes;
	                }
	        }*/


	unsigned length=0;

	//usleep(1000);usleep(1000);usleep(1000);
	//usleep(1000);usleep(1000);usleep(1000);
	//usleep(1000);usleep(1000);usleep(1000);
	//usleep(1000);usleep(1000);usleep(1000);
/*
	                ReceiveLength = (XLlFifo_iRxGetLen(InstancePtr))/WORD_SIZE;
	                xil_printf("receive length =%d \r\n",ReceiveLength);
	                length=ReceiveLength;
	                unsigned bytes=ReceiveLength;

	                //while (ReceiveLength) {
	                        //unsigned bytes = min(sizeof(DestinationAddr), ReceiveLength);
	                        XLlFifo_Read(InstancePtr, DestinationAddr, bytes);
	                        // ********
	                        // do something with buffer here
	                        // ********
	               //         ReceiveLength -= bytes;
	               // }
	       // }

for(i=0;i<length;i++)
xil_printf("data %d = %x \r\n ",i,*(DestinationAddr+i));


*/



//	 Read Recieve Length
	ReceiveLength = (XLlFifo_iRxGetLen(&FifoInstance))/WORD_SIZE;
	length        = ReceiveLength;
//	if(verbose_mode)xil_printf("receive length =%d \r\n",ReceiveLength);
	 //Start Receiving
	for ( i=0; i < ReceiveLength; i++){
		RxWord = 0;
		RxWord = XLlFifo_RxGetWord(&FifoInstance);
		//xil_printf("rxword1  =%x \r\n",RxWord);
		if(XLlFifo_iRxOccupancy(&FifoInstance)){
			RxWord = XLlFifo_RxGetWord(&FifoInstance);
		//	xil_printf("rxword 2 =%x \r\n",RxWord);
		}
		*(DestinationAddr+i) = RxWord;
	}

///	if(verbose_mode){
//	for(i=0;i<length;i++)
		//if(i==(length-1))
//	xil_printf("ReceiveBuffer(%d) = %x \r\n ",i,*(DestinationAddr+i));
//	}
	//Status = XLlFifo_IsRxDone(InstancePtr);
	//if(Status != TRUE){
	//	xil_printf("Failing in receive complete ... \r\n");
	//return XST_FAILURE;
//	}


	return ReceiveLength;
}
int XLlFifoPollingExample(XLlFifo *InstancePtr, u16 DeviceId,u32  *SourceAddr,u32 data_len,u8 verbose_mode)
{
	XLlFifo_Config *Config;

	int Status;
	int i;
	int Error;
	Status = XST_SUCCESS;

	/* Initial setup for Uart16550 */
/*
#ifdef XPAR_UARTNS550_0_BASEADDR

	Uart550_Setup();

#endif
*/

/*
	 Initialize the Device Configuration Interface driver
	Config = XLlFfio_LookupConfig(DeviceId);
	if (!Config) {
		xil_printf("No config found for %d\r\n", DeviceId);
		return XST_FAILURE;
	}




	 * This is where the virtual address would be used, this example
	 * uses physical address.

	Status = XLlFifo_CfgInitialize(InstancePtr, Config, Config->BaseAddress);
	if (Status != XST_SUCCESS) {
		xil_printf("Initialization failed\n\r");
		return Status;
	}




	 Check for the Reset value
	Status = XLlFifo_Status(InstancePtr);
	XLlFifo_IntClear(InstancePtr,0xffffffff);
	Status = XLlFifo_Status(InstancePtr);
	if(Status != 0x0) {
		xil_printf("\n ERROR : Reset value of ISR0 : 0x%x\t"
			    "Expected : 0x0\n\r",
			    XLlFifo_Status(InstancePtr));
		return XST_FAILURE;
	}
*/



	/* Transmit the Data Stream */
	Status = TxSend(&FifoInstance, SourceAddr,data_len,verbose_mode);
	if (Status != XST_SUCCESS){
		xil_printf("Transmisson of Data failed\n\r");
		return XST_FAILURE;
	}





	/* Revceive the Data Stream */
		ReceiveLength = RxReceive(&FifoInstance, DestinationBuffer,verbose_mode);
	//if (Status != XST_SUCCESS){
	//	xil_printf("Receiving data failed");
	//	return XST_FAILURE;
//	}

	Error = 0;

	/* Compare the data send with the data received */
	//xil_printf(" destination buffer[0] = %x \n\r",DestinationBuffer[0]);
	//for( i=0 ; i<MAX_DATA_BUFFER_SIZE ; i++ ){
	//	if ( *(SourceBuffer + i) != *(DestinationBuffer + i) ){
	//		Error = 1;
	//		break;
	//	}
	return Status;
	}

	//if (Error != 0){
	//	return XST_FAILURE;
	//}

//	return Status;
//}


/*****************************************************************************/
/**
*
* TxSend routine, It will send the requested amount of data at the
* specified addr.
*
* @param	InstancePtr is a pointer to the instance of the
*		XLlFifo component.
*
* @param	SourceAddr is the address where the FIFO stars writing
*
* @return
*		-XST_SUCCESS to indicate success
*		-XST_FAILURE to indicate failure
*
* @note		None
*
******************************************************************************/
int TxSend(XLlFifo *InstancePtr, u32  *SourceAddr,u32 data_len,u8 verbose_mode)
{

	u32 i;
	u32 j;
	//if(verbose_mode)xil_printf(" Transmitting Data in txsend... \r\n");

	// Filling the buffer with data
//	for (i=0;i<MAX_DATA_BUFFER_SIZE;i++)
//	{
//		if(i>=0 && i< 3){*(SourceAddr + i) = 0x17;}
//		else if(i==(MAX_DATA_BUFFER_SIZE-1))*(SourceAddr + i) = 0xe8;
//		else if( i % 2 == 0 )
//		*(SourceAddr + i) = 0x00;//

//		else
//			*(SourceAddr + i) = 0xff;//

//	}
	/*for(i=0 ; i < data_len ; i++){

			 Writing into the FIFO Transmit Port Buffer
			if( XLlFifo_iTxVacancy(&FifoInstance) ){
					//if(i<10)xil_printf("%x\n ", (u32)*(SourceAddr+(i*MAX_PACKET_LEN)+j));
					XLlFifo_TxPutWord(&FifoInstance,*(SourceAddr+i));
				}


		}*/

	if( XLlFifo_iTxVacancy(&FifoInstance)> data_len)
	XLlFifo_Write
		(
			&FifoInstance,
			SourceAddr,
			data_len *WORD_SIZE
		);



	/* Start Transmission by writing transmission length into the TLR */
	XLlFifo_iTxSetLen(&FifoInstance, data_len * WORD_SIZE);

	/* Check for Transmission completion */
	while( !(XLlFifo_IsTxDone(&FifoInstance)) ){

	}

	/* Transmission Complete */
	//if(verbose_mode)xil_printf(" End Transmition.. \r\n");
	return XST_SUCCESS;
}



/*****************************************************************************/
/**
*
* RxReceive routine.It will receive the data from the FIFO.
*
* @param	InstancePtr is a pointer to the instance of the
*		XLlFifo instance.
*
* @param	DestinationAddr is the address where to copy the received data.
*
* @return
*		-XST_SUCCESS to indicate success
*		-XST_FAILURE to indicate failure
*
* @note		None
*
******************************************************************************/
void check_fifo(u32  *SourceAddr,u32 data_len,u8 verbose_mode)
{
int Status;
u32 i;
//u8 verbose_mode=0;
//if(verbose_mode)xil_printf("\n\r--- Entering check_fifo() ---\n\r");
//if(verbose_mode)	xil_printf("\n\r source address_static = %p \n\r",(void*)SourceBuffer);
//if(verbose_mode)	xil_printf("\n\r source address = %p \n\r",(void*)SourceAddr);
	/*for (i=0;i<MAX_DATA_BUFFER_SIZE;i++)
			{
				if(i>=0 && i< 3)*(SourceAddr + i) = 0x17;
				else if(i==(MAX_DATA_BUFFER_SIZE-1))*(SourceAddr + i) = 0xe8;
				else if( i % 2 == 0 )
				*(SourceAddr + i) = 0x00;//

			else
					*(SourceAddr + i) = 0xff;//
		//if(i>=0 && i< 3){*(SourceAddr + i) = 0x17;}//sync
		 //if (i>=500)
	//	 {*(SourceAddr + i) = 0xe8;}// syncack
		//else {*(SourceAddr + i) = 0xe8;}
			}*/
	//xil_printf("\n\r source address = %p \n\r",(void *)SourceAddr);
	Status = XLlFifoPollingExample(&FifoInstance, FIFO_DEV_ID,SourceAddr,data_len,verbose_mode);
	if (Status != XST_SUCCESS) {
		xil_printf("Axi Streaming FIFO Polling Example Test Failed\n\r");
		xil_printf("--- Exiting main() ---\n\r");
		return XST_FAILURE;
	}

	//xil_printf("Successfully ran Axi Streaming FIFO Polling Example\n\r");
	//xil_printf("--- Exiting check_fifo() ---\n\r");

	return XST_SUCCESS;
}



/**************************************************************************
//
// crc16.c - generate a ccitt 16 bit cyclic redundancy check (crc)
//
//      The code in this module generates the crc for a block of data.
//
**************************************************************************/

/*
//                                16  12  5
// The CCITT CRC 16 polynomial is X + X + X + 1.
// In binary, this is the bit pattern 1 0001 0000 0010 0001, and in hex it
//  is 0x11021.
// A 17 bit register is simulated by testing the MSB before shifting
//  the data, which affords us the luxury of specifiy the polynomial as a
//  16 bit value, 0x1021.
// Due to the way in which we process the CRC, the bits of the polynomial
//  are stored in reverse order. This makes the polynomial 0x8408.
*/
#define POLY 0x8408

/*
// note: when the crc is included in the message, the valid crc is:
//      0xF0B8, before the compliment and byte swap,
//      0x0F47, after compliment, before the byte swap,
//      0x470F, after the compliment and the byte swap.
*/

extern  crc_ok;
/**************************************************************************
//
// crc16() - generate a 16 bit crc
//
//
// PURPOSE
//      This routine generates the 16 bit remainder of a block of
//      data using the ccitt polynomial generator.
//
// CALLING SEQUENCE
//      crc = crc16(data, len);
//
// PARAMETERS
//      data    <-- address of start of data block
//      len     <-- length of data block
//
// RETURNED VALUE
//      crc16 value. data is calcuated using the 16 bit ccitt polynomial.
//
// NOTES
//      The CRC is preset to all 1's to detect errors involving a loss
//        of leading zero's.
//      The CRC (a 16 bit value) is generated in LSB MSB order.
//      Two ways to verify the integrity of a received message
//        or block of data:
//        1) Calculate the crc on the data, and compare it to the crc
//           calculated previously. The location of the saved crc must be
//           known.
/         2) Append the calculated crc to the end of the data. Now calculate
//           the crc of the data and its crc. If the new crc equals the
//           value in "crc_ok", the data is valid.
//
// PSEUDO CODE:
//      initialize crc (-1)
//      DO WHILE count NE zero
//        DO FOR each bit in the data byte, from LSB to MSB
//          IF (LSB of crc) EOR (LSB of data)
//            crc := (crc / 2) EOR polynomial
//          ELSE
//            crc := (crc / 2)
//          FI
//        OD
//      OD
//      1's compliment and swap bytes in crc
//      RETURN crc
//
**************************************************************************/
unsigned int crc16(char *data_p, unsigned short length)
//char *data_p;
//unsigned short length;
{
       unsigned char i;
       unsigned int data;
       unsigned int crc;

       crc = 0xffff;

       if (length == 0)
              return (~crc);

       do {
              for (i = 0, data = (unsigned int)0xff & *data_p++;
                  i < 8;
                  i++, data >>= 1) {
                    if ((crc & 0x0001) ^ (data & 0x0001))
                           crc = (crc >> 1) ^ POLY;
                    else
                           crc >>= 1;
              }
       } while (--length);

       crc = ~crc;

       data = crc;
       crc = (crc << 8) | (data >> 8 & 0xFF);
       crc=swapBytes(crc);
       return (crc);
}


u32 HDLC_Rx(XLlFifo *InstancePtr,u32* DestinationAddr,u32 rcv_len,u8 mode,u8 verbose_mode)
{

	u8 fcs          = 0xff;
	u8 hdlc_address = 0xff;
	u8 hdlc_control = 0xff;
/*
	union{
			u32 ipbus_header;
		struct
		{
			u8 ipbus_header_0;
			u8 ipbus_header_1;
			u8 ipbus_header_2;
			u8 ipbus_header_3;


		}st;
	}un;

	union{
				u16 crc;
			struct
			{
				u8 crc_lsb;
				u8 crc_msb;

			}st1;
		}un1;

		union{
						u32 read_data;
					struct
					{
						u8 read_data_0;
						u8 read_data_1;
						u8 read_data_2;
						u8 read_data_3;

					}st2;
				}un2;

*/



u8 type_id=0xaa;
	u8 NUM_OF_ONES=0;
	u8 tmp;
	u8 rcv_8;
	u32 i=0;
	u32 j=0;
	u8 rcv_arr[15];
	u16 crc_calculated;
	u16 crc_received;


	ReceiveLength = RxReceive(&FifoInstance, DestinationBuffer,verbose_mode);
	//u8 verbose_mode=1;
//if(verbose_mode)	xil_printf("rcv_len=%d\r\n",rcv_len);
	do{
		rcv_8 = *(DestinationAddr+i);
		tmp=(rcv_8==SC0)?0:1;
		fcs <<=1;
		fcs|=tmp;
	//	if(verbose_mode)xil_printf("i=%d    rcv_8= %x  ,,     fcs=%x\r\n",i,rcv_8,fcs);
		i++;

}while(!(fcs==0x7e || i>rcv_len));
	//if(verbose_mode)xil_printf("HDLC_FCS RECEIVED= %02x\r\n",fcs);
	if(i<rcv_len)
	{
	///////////////HDLC ADDRESS DECODING///////////////////
		j=0;
		int k=0;
		do{

				rcv_8 = *(DestinationAddr+i);
				if(rcv_8 == SC0 || rcv_8== SC1)
				{


					if(rcv_8 == SC1 ){NUM_OF_ONES++;}else if(NUM_OF_ONES<5 && rcv_8==SC0) NUM_OF_ONES=0;//ZERO REMOVAL

					if(NUM_OF_ONES == 5 && rcv_8 == SC0){
						NUM_OF_ONES=0;
						i++;
					//	if(verbose_mode)xil_printf("discarded extra 0\r\n");
					}//discard extra 0
					else if(NUM_OF_ONES>5 && rcv_8 == SC1)
					{
						NUM_OF_ONES=0;
						i++;
						xil_printf("frame error 6 ones received in packet\r\n");
					}//error handling
					else
					{
					tmp=(rcv_8==SC0)?0:1;
					rcv_arr[k] <<=1;
					rcv_arr[k]|=tmp;
					//if(verbose_mode)	xil_printf("i=%d    rcv_8= %x  ,,     hdlc_address=%x\r\n",i,rcv_8,rcv_arr[k]);
					i++;j++;
					}
				//	if(rcv_8==SC1){NUM_OF_ONES++;}else NUM_OF_ONES=0;//ZERO REMOVAL
				}
				else{xil_printf("frame error 1 rcv_8 = %x\r\n",rcv_8);}
				//j++;
		}while(j<8);
		rcv_arr[k]=reverse(rcv_arr[k]);

	//	if(verbose_mode)xil_printf("HDLC_ADDRESS RECEIVED= %02x\r\n",rcv_arr[k]);
//////////////////////////////////////////////////////////////////////////
///////////////HDLC CONTROL DECODING//////////////////////////
		j=0;k++;//NUM_OF_ONES=0;
				do{
						rcv_8 = *(DestinationAddr+i);
						if(rcv_8 == SC0 || rcv_8== SC1)
						{
							if(rcv_8 == SC1 ){
								NUM_OF_ONES++;}else if(NUM_OF_ONES<5 && rcv_8==SC0) NUM_OF_ONES=0;//ZERO REMOVAL
							if(NUM_OF_ONES == 5 && rcv_8 == SC0)
							{
								NUM_OF_ONES=0;
								i++;
							//	if(verbose_mode)xil_printf("discarded extra 0\r\n");
							}//discard extra 0
							else if(NUM_OF_ONES>5 && rcv_8 == SC1)
							{
								NUM_OF_ONES=0;
								i++;
								//xil_printf("frame error 6 ones received in packet\r\n");
								}//error handling
							else{
								tmp=(rcv_8==SC0)?0:1;
							rcv_arr[k] <<= 1;
							rcv_arr[k]|= tmp;
							//if(verbose_mode)xil_printf("i=%d    rcv_8= %x  ,,     hdlc_control=%x\r\n",i,rcv_8,rcv_arr[k]);
							i++;j++;
							}
							//if(rcv_8==SC1){NUM_OF_ONES++;}else NUM_OF_ONES=0;//ZERO REMOVAL
						}
						else{xil_printf("frame error 2 rcv_8 = %x\r\n",rcv_8);}
						//j++;
				}while(j<8);
				rcv_arr[k]=reverse(rcv_arr[k]);
			//	if(verbose_mode)xil_printf("HDLC_CONTROL RECEIVED= %02x\r\n",rcv_arr[k]);
///////////////////////////////////////////////////////////////////////////////////////
				///////////////IPBUS HEADER DECODING//////////////////////////
						j=0;k++;//NUM_OF_ONES=0;
								do{
										rcv_8 = *(DestinationAddr+i);
										if(rcv_8 == SC0 || rcv_8== SC1)
										{
											if(rcv_8 == SC1 ){NUM_OF_ONES++;}else if(NUM_OF_ONES<5 && rcv_8==SC0) NUM_OF_ONES=0;//ZERO REMOVAL
											if(NUM_OF_ONES == 5 && rcv_8 == SC0)
											{
												NUM_OF_ONES=0;
												i++;
											//	if(verbose_mode)xil_printf("discarded extra 0\r\n");
											}//discard extra 0
											else if(NUM_OF_ONES>5 && rcv_8 == SC1)
											{
												NUM_OF_ONES=0;
												i++;
												xil_printf("frame error 6 ones received in packet\r\n");}//error handling
											else{
											tmp=(rcv_8==SC0)?0:1;
											rcv_arr[k] <<=1;
											rcv_arr[k]|=tmp;
											//if(verbose_mode)xil_printf("i=%d    rcv_8= %x  ,,     hdlc_control=%x\r\n",i,rcv_8,rcv_arr[k]);
											i++;j++;
											}
											//if(rcv_8==SC1){NUM_OF_ONES++;}else NUM_OF_ONES=0;//ZERO REMOVAL
										}
										else{xil_printf("frame error 3 rcv_8 = %x\r\n",rcv_8);}
										//j++;
								}while(j<8);
								rcv_arr[k]=reverse(rcv_arr[k]);
							//	if(verbose_mode)	xil_printf("IP bus header_0 RECEIVED= %02x\r\n",rcv_arr[k]);
				///////////////////////////////////////////////////////////////////////////////////////
								j=0;k++;//NUM_OF_ONES=0;
									do{
										rcv_8 = *(DestinationAddr+i);
										if(rcv_8 == SC0 || rcv_8== SC1)
										{
											if(rcv_8 == SC1 ){NUM_OF_ONES++;}else if(NUM_OF_ONES<5 && rcv_8==SC0) NUM_OF_ONES=0;//ZERO REMOVAL
											if(NUM_OF_ONES == 5 && rcv_8 == SC0)
											{NUM_OF_ONES=0;
											i++;
											//if(verbose_mode)xil_printf("discarded extra 0\r\n");
											}//discard extra 0
											else if(NUM_OF_ONES>5 && rcv_8 == SC1)
											{
												NUM_OF_ONES=0;
												i++;
												xil_printf("frame error 6 ones received in packet\r\n");
											}//error handling
											else{
											tmp=(rcv_8==SC0)?0:1;
											rcv_arr[k] <<=1;
											rcv_arr[k]|=tmp;
											//if(verbose_mode)xil_printf("i=%d    rcv_8= %x  ,,     ipbus_header_1=%x\r\n",i,rcv_8,rcv_arr[k]);
											i++;j++;
												}
										//	if(rcv_8==SC1){NUM_OF_ONES++;}else NUM_OF_ONES=0;//ZERO REMOVAL
										}
										else{xil_printf("frame error  4 rcv_8 = %x\r\n",rcv_8);}
										//j++;
										}while(j<8);
									rcv_arr[k]=reverse(rcv_arr[k]);
										//if(verbose_mode)	xil_printf("IP bus header_1 RECEIVED= %02x\r\n",rcv_arr[k]);

										///////////////////////////////////////////////////////////////////////////////////////
							j=0;k++;//NUM_OF_ONES=0;
								do{
										rcv_8 = *(DestinationAddr+i);
										if(rcv_8 == SC0 || rcv_8== SC1)
										{

											if(rcv_8 == SC1 ){NUM_OF_ONES++;}else if(NUM_OF_ONES<5 && rcv_8==SC0) NUM_OF_ONES=0;//ZERO REMOVAL
											if(NUM_OF_ONES == 5 && rcv_8 == SC0)
											{
												NUM_OF_ONES=0;
												i++;
											//	if(verbose_mode)xil_printf("discarded extra 0\r\n");
											}//discard extra 0
											else if(NUM_OF_ONES>5 && rcv_8 == SC1){NUM_OF_ONES=0;i++;xil_printf("frame error 6 ones received in packet\r\n");}//error handling
											else{
										tmp=(rcv_8==SC0)?0:1;
										rcv_arr[k] <<=1;
										rcv_arr[k]|=tmp;
										//if(verbose_mode)xil_printf("i=%d    rcv_8= %x  ,,     ipbus_header_2=%x\r\n",i,rcv_8,rcv_arr[k]);
										i++;j++;
											}
										//if(rcv_8==SC1){NUM_OF_ONES++;}else NUM_OF_ONES=0;//ZERO REMOVAL
										}
										else{xil_printf("frame error 5 rcv_8 = %x\r\n",rcv_8);}
										//j++;
								}while(j<8);
								rcv_arr[k]=reverse(rcv_arr[k]);
								//if(verbose_mode)	xil_printf("IP bus header_2 RECEIVED= %02x\r\n",rcv_arr[k]);
//////////////////////////////////////////////////////////////////////////////////////////
								j=0;k++;//NUM_OF_ONES=0;
								do{
									rcv_8 = *(DestinationAddr+i);
									if(rcv_8 == SC0 || rcv_8== SC1)
									{
										if(rcv_8 == SC1 ){NUM_OF_ONES++;}else if(NUM_OF_ONES<5 && rcv_8==SC0) NUM_OF_ONES=0;//ZERO REMOVAL
										if(NUM_OF_ONES == 5 && rcv_8 == SC0){
											NUM_OF_ONES=0;
											i++;
										//	if(verbose_mode)xil_printf("discarded extra 0\r\n");
										}//discard extra 0
										else if(NUM_OF_ONES>5 && rcv_8 == SC1)
										{
											NUM_OF_ONES=0;
											i++;
											xil_printf("frame error 6 ones received in packet\r\n");
										}//error handling
										else{
										tmp=(rcv_8==SC0)?0:1;
										rcv_arr[k] <<=1;
										rcv_arr[k]|=tmp;
										//if(verbose_mode)	xil_printf("i=%d    rcv_8= %x  ,,     ipbus_header_3=%x\r\n",i,rcv_8,rcv_arr[k]);
										i++;j++;
										}
									//if(rcv_8==SC1){NUM_OF_ONES++;}else NUM_OF_ONES=0;//ZERO REMOVAL
									}
								else{xil_printf("frame error 6 rcv_8 = %x\r\n",rcv_8);}
									//j++;
								}while(j<8);
								rcv_arr[k]=reverse(rcv_arr[k]);
							//	if(verbose_mode)xil_printf("IP bus header_3 RECEIVED= %02x\r\n",rcv_arr[k]);


////////////////////////////////////////////////////////////////////////////////////////////////////////////
								/////////////////////data//////////////////////////////////////////////

										if(mode == READ)//read transaction read data
										{
																	j=0;k++;//NUM_OF_ONES=0;
																do{
																rcv_8 = *(DestinationAddr+i);
																if(rcv_8 == SC0 || rcv_8== SC1)
																{

																if(rcv_8 == SC1 ){NUM_OF_ONES++;}else if(NUM_OF_ONES<5 && rcv_8==SC0) {NUM_OF_ONES=0;}//ZERO REMOVAL
																if(NUM_OF_ONES == 5 && rcv_8 == SC0)
																{NUM_OF_ONES=0;
																i++;
																//if(verbose_mode)xil_printf("discarded extra 0\r\n");
																}//discard extra 0
																else if(NUM_OF_ONES>5 && rcv_8 == SC1){NUM_OF_ONES=0;i++;xil_printf("frame error 6 ones received in packet\r\n");}//error handling
																else{
																	//xil_printf("last else\t");
																tmp=(rcv_8 == SC0)?0:1;
																rcv_arr[k] <<=1;
																rcv_arr[k]|=tmp;
																//if(verbose_mode)	xil_printf("i=%d    rcv_8= %x  ,,     read_data_0=%x\r\n",i,rcv_8,rcv_arr[k]);
																i++;j++;
																}
																//xil_printf("num_of_ones= %d ,rcv_8 = %x ,i= %d\r\n",NUM_OF_ONES,rcv_8,i-1);
																}
																else{xil_printf("frame error  7 rcv_8 = %x\r\n",rcv_8);}

															}while(j<8);
																rcv_arr[k]=reverse(rcv_arr[k]);
													//		if(verbose_mode)xil_printf("read_data_0 RECEIVED= %02x\r\n",rcv_arr[k]);

////////////////////////////////////////////////data_1////////////////////////
															j=0;k++;//NUM_OF_ONES=0;
														do{
														rcv_8 = *(DestinationAddr+i);
														if(rcv_8 == SC0 || rcv_8== SC1)
														{
															if(rcv_8 == SC1 ){NUM_OF_ONES++;}else if(NUM_OF_ONES<5 && rcv_8==SC0) NUM_OF_ONES=0;//ZERO REMOVAL
															if(NUM_OF_ONES == 5 && rcv_8 == SC0){NUM_OF_ONES=0;i++;if(verbose_mode)xil_printf("discarded extra 0\r\n");}//discard extra 0
															else if(NUM_OF_ONES>5 && rcv_8 == SC1){NUM_OF_ONES=0;i++;xil_printf("frame error 6 ones received in packet\r\n");}//error handling
															else{
																tmp=(rcv_8==SC0)?0:1;
																rcv_arr[k] <<=1;
																rcv_arr[k]|=tmp;
																//if(verbose_mode)	xil_printf("i=%d    rcv_8= %x  ,,     read_data_1=%x\r\n",i,rcv_8,rcv_arr[k]);
																i++;j++;
																}

																}
																else{xil_printf("frame error 8 rcv_8 = %x\r\n",rcv_8);}
														//j++;
															}while(j<8);
														rcv_arr[k]=reverse(rcv_arr[k]);
																//if(verbose_mode)xil_printf("read_data_1 RECEIVED= %02x\r\n",rcv_arr[k]);


																////////////////////////////////////////////////data_2////////////////////////
																															j=0;k++;//NUM_OF_ONES=0;
																														do{
																														rcv_8 = *(DestinationAddr+i);
																														if(rcv_8 == SC0 || rcv_8== SC1)
																														{
																															if(rcv_8 == SC1 ){NUM_OF_ONES++;}else if(NUM_OF_ONES<5 && rcv_8==SC0) NUM_OF_ONES=0;//ZERO REMOVAL
																																															if(NUM_OF_ONES == 5 && rcv_8 == SC0){NUM_OF_ONES=0;i++;if(verbose_mode)xil_printf("discarded extra 0\r\n");}//discard extra 0
																																															else if(NUM_OF_ONES>5 && rcv_8 == SC1){NUM_OF_ONES=0;i++;xil_printf("frame error 6 ones received in packet\r\n");}//error handling
																																															else{
																																tmp=(rcv_8==SC0)?0:1;
																																rcv_arr[k] <<=1;
																																rcv_arr[k]|=tmp;
																																//if(verbose_mode)	xil_printf("i=%d    rcv_8= %x  ,,     read_data_2=%x\r\n",i,rcv_8,rcv_arr[k]);
																																i++;j++;
																																}
																																//if(rcv_8==SC1){NUM_OF_ONES++;}else NUM_OF_ONES=0;//ZERO REMOVAL
																																}
																																else{xil_printf("frame error  9 rcv_8 = %x\r\n",rcv_8);}
																														//j++;
																															}while(j<8);
																														rcv_arr[k]=reverse(rcv_arr[k]);
																																//if(verbose_mode)xil_printf("read_data_2 RECEIVED= %02x\r\n",rcv_arr[k]);

																																////////////////////////////////////////////////data_3////////////////////////
																														j=0;k++;//NUM_OF_ONES=0;
																														do{
																														rcv_8 = *(DestinationAddr+i);
																														if(rcv_8 == SC0 || rcv_8== SC1)
																														{
																															if(rcv_8 == SC1 ){NUM_OF_ONES++;}else if(NUM_OF_ONES<5 && rcv_8==SC0) NUM_OF_ONES=0;//ZERO REMOVAL
																																															if(NUM_OF_ONES == 5 && rcv_8 == SC0){NUM_OF_ONES=0;i++;if(verbose_mode)xil_printf("discarded extra 0\r\n");}//discard extra 0
																																															else if(NUM_OF_ONES>5 && rcv_8 == SC1){NUM_OF_ONES=0;i++;xil_printf("frame error 6 ones received in packet\r\n");}//error handling
																																															else{
																														tmp=(rcv_8==SC0)?0:1;
																														rcv_arr[k]<<=1;
																														rcv_arr[k]|=tmp;
																														//if(verbose_mode)	xil_printf("i=%d    rcv_8= %x  ,,     read_data_3=%x\r\n",i,rcv_8,rcv_arr[k]);
																														i++;j++;
																														}
																														//if(rcv_8==SC1){NUM_OF_ONES++;}else NUM_OF_ONES=0;//ZERO REMOVAL
																														}
																														else{xil_printf("frame error 10 rcv_8 = %x\r\n",rcv_8);}
																														//j++;
																														}while(j<8);
																														rcv_arr[k]=reverse(rcv_arr[k]);
																													//	if(verbose_mode)xil_printf("read_data_3 RECEIVED= %02x\r\n",rcv_arr[k]);


																}//END MODE = READ
										////////////////////////////crc calculation/////////////////


																	j=0;//NUM_OF_ONES=0;
																									do{
																										rcv_8 = *(DestinationAddr+i);
																										if(rcv_8 == SC0 || rcv_8== SC1)
																										{
																											if(rcv_8 == SC1 ){NUM_OF_ONES++;}else if(NUM_OF_ONES<5 && rcv_8==SC0) NUM_OF_ONES=0;//ZERO REMOVAL
																																											if(NUM_OF_ONES == 5 && rcv_8 == SC0){NUM_OF_ONES=0;i++;if(verbose_mode)xil_printf("discarded extra 0\r\n");}//discard extra 0
																																											else if(NUM_OF_ONES>5 && rcv_8 == SC1){NUM_OF_ONES=0;i++;xil_printf("frame error 6 ones received in packet\r\n");}//error handling
																																											else{
																											tmp=(rcv_8==SC0)?0:1;
																											rcv_arr[k+1] <<=1;
																											rcv_arr[k+1]|=tmp;
																											//if(verbose_mode)	xil_printf("i=%d    rcv_8= %x  ,,     crc_lsb=%x\r\n",i,rcv_8,rcv_arr[k+1]);
																											i++;j++;
																											}
																										//if(rcv_8==SC1){NUM_OF_ONES++;}else NUM_OF_ONES=0;//ZERO REMOVAL
																										}
																									else{xil_printf("frame error 11 rcv_8 = %x\r\n",rcv_8);}
																										//j++;
																									}while(j<8);
																									rcv_arr[k+1]=reverse(rcv_arr[k+1]);
																								//	if(verbose_mode)xil_printf("crc_lsb RECEIVED= %02x\r\n",rcv_arr[k+1]);

																	////////////////////crc msb/////////////////////////////////////////
																									j=0;//k++;//NUM_OF_ONES=0;
																									do{
																										rcv_8 = *(DestinationAddr+i);
																										if(rcv_8 == SC0 || rcv_8== SC1)
																										{
																											if(rcv_8 == SC1 ){NUM_OF_ONES++;}else if(NUM_OF_ONES<5 && rcv_8==SC0) NUM_OF_ONES=0;//ZERO REMOVAL
																																											if(NUM_OF_ONES == 5 && rcv_8 == SC0){NUM_OF_ONES=0;i++;if(verbose_mode)xil_printf("discarded extra 0\r\n");}//discard extra 0
																																											else if(NUM_OF_ONES>5 && rcv_8 == SC1){NUM_OF_ONES=0;i++;xil_printf("frame error 6 ones received in packet\r\n");}//error handling
																																											else{
																										tmp=(rcv_8==SC0)?0:1;
																										rcv_arr[k+2] <<=1;
																										rcv_arr[k+2]|=tmp;
																										//if(verbose_mode)	xil_printf("i=%d    rcv_8= %x  ,,     crc_msb=%x\r\n",i,rcv_8,rcv_arr[k+2]);
																										i++;j++;
																											}
																										//if(rcv_8==SC1){NUM_OF_ONES++;}else NUM_OF_ONES=0;//ZERO REMOVAL
																											}
																										else{xil_printf("frame error 12  rcv_8 = %x\r\n",rcv_8);}
																										//j++;
																									}while(j<8);
																									rcv_arr[k+2]=reverse(rcv_arr[k+2]);
																								//	if(verbose_mode)xil_printf("crc_msb RECEIVED= %02x\r\n",rcv_arr[k+2]);

																									//type_id=(un.st.ipbus_header_0 >> 4) & 0x0f;


																									///////////////////FCS :TERMINATING FRAME

																									j=0;//k++;//NUM_OF_ONES=0;
																									do{
																										rcv_8 = *(DestinationAddr+i);
																										if(rcv_8 == SC0 || rcv_8== SC1)
																										{
																										//if(NUM_OF_ONES==5 && rcv_8 == SC0){NUM_OF_ONES=0;i++;xil_printf("discarded extra 0\r\n");}//discard extra 0
																										//else if(NUM_OF_ONES==5 && rcv_8 == SC1){NUM_OF_ONES=0;i++;xil_printf("frame error 6 ones received in packet\r\n");}//error handling
																										//else{
																											tmp=(rcv_8==SC0)?0:1;
																											rcv_arr[k+3] <<=1;
																											rcv_arr[k+3]|=tmp;
																										//	if(verbose_mode)	xil_printf("i=%d    rcv_8= %x  ,,     fcs_end=%x\r\n",i,rcv_8,rcv_arr[k+3]);
																											i++;j++;
																											//}
																											//if(rcv_8==SC1){NUM_OF_ONES++;}else NUM_OF_ONES=0;//ZERO REMOVAL
																											}
																										else{xil_printf("frame error 13 rcv_8 = %x\r\n",rcv_8);}
																										//j++;
																										}while(j<8);
																										//rcv_arr[k]=reverse(rcv_arr[k]);
																										//if(verbose_mode)xil_printf("FCS_END  RECEIVED= %02x\r\n",rcv_arr[k+3]);
																																													//type_id=(un.st.ipbus_header_0 >> 4) & 0x0f;
																									////////////////////////////////////////////////////////////////////////


																										if(mode==READ)crc_received=(rcv_arr[11]<<8)|rcv_arr[10];
																										else crc_received=(rcv_arr[7]<<8)|rcv_arr[6];
																										//crc_calculated= crc16(rcv_arr,k+1);




																										//	if(verbose_mode)
																										//	{xil_printf("k=%d\r\n",k);
																									//	xil_printf("crc_calculated = %x :: crc_received = %x\r\n", crc_calculated, crc_received);
																									//		}
																										//u8 loop=0;do{xil_printf("RCV_ARR[loop]=%02x\r\n",rcv_arr[loop++]);}while(loop<(k-1));




//for(i = 0,i<)
																			//rcv_arr[0] 		= 	hdlc_address;
																			//rcv_arr[1] 		=    hdlc_control;
																			//rcv_arr[2]	=   un.st.ipbus_header_0;
																			//rcv_arr[3]	=   un.st.ipbus_header_1;
																			//rcv_arr[4]	=   un.st.ipbus_header_2;
																			//rcv_arr[5]	=   un.st.ipbus_header_3;



			//	if(verbose_mode){xil_printf("IP bus header RECEIVED= %08x\r\n",(u32)(rcv_arr[5]<<24)|(rcv_arr[4]<<16)|(rcv_arr[3]<<8)|rcv_arr[2]);
			//					xil_printf("Info code= %01x\r\n",(rcv_arr[2] & 0x0f));
			//					xil_printf("Type_id= %01x\r\n",((rcv_arr[2] >> 4) & 0x0f));
			//					xil_printf("Transaction id= %02x\r\n",rcv_arr[3]);
			//					xil_printf("Depth = %03x\r\n",(u16)(( (rcv_arr[5] &0x0f)<<8)|rcv_arr[4]));
			//					xil_printf("Header version = %01x\r\n",( (un.st.ipbus_header_3 >>4)  & 0x0f) );
			///////					//xil_printf("Header version = %01x\r\n",( (un.st.ipbus_header_3 >>4)  & 0x0f) );

						//		if(mode==READ)xil_printf("READ_DATA= %04x\r\n", (u32)(rcv_arr[9]<<24)|(rcv_arr[8]<<16)|(rcv_arr[7]<<8)|rcv_arr[6] );
						//		xil_printf("CRC = %04x\r\n", crc_received );
					//			//for()
						//		xil_printf("i= %d, length =%d\r\n",i,rcv_len);
				//}
	///////////////////crc calculation//////////////////
									}//end if rcv_len
if(mode==READ)
	return (u32)(rcv_arr[9]<<24)|(rcv_arr[8]<<16)|(rcv_arr[7]<<8)|rcv_arr[6];
else
	return  (u32)(rcv_arr[9]<<24)|(rcv_arr[8]<<16)|(rcv_arr[7]<<8)|rcv_arr[6];


}
void HDLC_Tx(XLlFifo *InstancePtr,u32 register_address, u32 register_data,u8 mode,u32* SourceAddr,u8 verbose_mode)
{
	// IPBUS Transaction types

	//	u8 verbose_mode=0;
		//u8* p;

		int i;


u8 Status;
		unsigned short length;

		if (mode == READ)
			 TYPE_ID = TRANS_TYPE_RD;
		else TYPE_ID = TRANS_TYPE_WR;
		u32 IP_BUS_HEADER  = (u32) ((HEADER_VERSION<<28) | (DEPTH<<16) | (TRANSACTION_ID<<8)|(TYPE_ID<<4) | INFO_CODE_REQ);

//if(verbose_mode)xil_printf("header= %08x\r\n",IP_BUS_HEADER);


	u8 hdlc_tx[] =  	{
								//HDLC_FS,
								HDLC_ADDRESS,
								HDLC_CONTROL,
								(u8)(IP_BUS_HEADER),// & 0X000000FF),
								(u8)((IP_BUS_HEADER >>8) ),// & 0X000000FF),
								(u8)((IP_BUS_HEADER >>16) ),//& 0X000000FF),
								(u8)((IP_BUS_HEADER >>24)),// & 0X000000FF),

								(u8)((register_address )),//& 0X000000FF)),
								(u8)((register_address>>8)),//&0X000000FF),
								(u8)((register_address>>16)),//&0X000000FF),
								(u8)((register_address>>24)),// &0X000000FF),


								(u8)((register_data )),//& 0X000000FF)),
								(u8)((register_data>>8)),//&0X000000FF),
								(u8)((register_data>>16)),//&0X000000FF),
								(u8)((register_data>>24))//&0X000000FF)

								//(u8)ut1.st1.crc_lsb,
								//(u8)ut1.st1.crc_msb
								};





				if(mode ==READ)	length= sizeof(hdlc_tx) - 4;//exclude register_data in crc calculation
				else 			length= sizeof(hdlc_tx);//write mode ,include register _data in string


		ut1.crc=crc16(hdlc_tx, length);

		//xil_printf("        FCS = %x\r\n",HDLC_FS);

u8 bits[(length+4)*8];// (+4 means fcs+crc+crc+fcs)
u8 tmp;
char j;
		for(int i=0;i<(length+4);i++){
			if(i==0 || (i== (length+3))){
						//if(verbose_mode)xil_printf("        FCS = %x\r\n",HDLC_FS);
						tmp = HDLC_FS;
						for(j=0;j<8;j++){
							bits[8*i+j]= ((tmp & 1)==1)? SC1:SC0;
							tmp>>=1;
							//xil_printf("%d\t",bits[i][j]);
						}
						/*if(i==length+3)
						{
							tmp = 0x00;
													for(j=0;j<8;j++){
														bits[8*i+j]= ((tmp & 1)==1)? 0x00:0x00;
														tmp>>=1;
														//xil_printf("%d\t",bits[i][j]);
													}


						}
*/

					//	if(verbose_mode)for(j=7;j>=0;j--){
					//		xil_printf("%02X\t",bits[8*i+j]);
					//									}
					//	if(verbose_mode)xil_printf("\r\n");
					}
			else if(i>0 && i<=length)
			{
			//	if(verbose_mode)xil_printf("hdlc_tx(%02d) = %02x\r\n",i-1,hdlc_tx[i-1]);

		tmp = hdlc_tx[i-1];
		for(j=0;j<8;j++){
			bits[8*i+j]= ((tmp & 1)==1)? SC1:SC0;
			tmp>>=1;
			//xil_printf("%d\t",bits[i][j]);
		}
		//if(verbose_mode)for(j=7;j>=0;j--){
		//	xil_printf("%02x\t",bits[8*i+j]);
		//		}


		//if(verbose_mode)xil_printf("\r\n");
		}

			else if(i>length && i<=length+2)//crc
						{
				if(i==(length+1)){
					//if(verbose_mode)xil_printf("crc_lsb = %02x\r\n",ut1.st1.crc_lsb);

					tmp = ut1.st1.crc_lsb;
					for(j=0;j<8;j++){
						bits[8*i+j]= ((tmp & 1)==1)? SC1:SC0;
						tmp>>=1;
						//xil_printf("%d\t",bits[i][j]);
					}
					//if(verbose_mode)for(j=7;j>=0;j--){
					//	xil_printf("%02X\t",bits[8*i+j]);
					//		}


					//if(verbose_mode)xil_printf("\r\n");
					}
				else{
			//		if(verbose_mode)xil_printf("crc_msb = %02x\r\n",ut1.st1.crc_msb);
						tmp = (ut1.st1.crc_msb);
						for(j=0;j<8;j++){
						bits[8*i+j]= ((tmp & 1)==1)? SC1:SC0;
						tmp>>=1;
						//xil_printf("%d\t",bits[i][j]);
						}
				//	for(j=7;j>=0;j--){
				//		if(verbose_mode)xil_printf("%02X\t",bits[8*i+j]);
				//		}


				//	if(verbose_mode)xil_printf("\r\n");
					}

	}


}
//	j=0;for(i=0;i<sizeof(bits);i++)
//	{

//		if(verbose_mode)xil_printf("%02x\t",bits[i]);
//		if(j==7){
//			if(verbose_mode)xil_printf("\r\n");
//			j=0;}
//		else j++;
//	}


	/////////bit stuffing determination/////////////////
	///////////findind penta 5 1s///////////////
		j=0;
		u8 ZeroInsertion_location[50];
		u8 NUM_OF_ONES=0;
		u8 NUM_OF_ZEROS=0;
	for(i=8;i<(sizeof(bits)-8);i++)
		{

			if(bits[i]==SC1)
			{NUM_OF_ONES++;

			}
			else
			NUM_OF_ONES=0;

		//	ZeroInsertion_location[j]=i+1;


		if(NUM_OF_ONES==5)
		{
			ZeroInsertion_location[j]=i+1;
			j++;
			NUM_OF_ONES=0;
		}

		}
	NUM_OF_ZEROS=j;
//	if(verbose_mode){
//	xil_printf("NUM_OF_ZEROS=%d\r\n",NUM_OF_ZEROS);
//	for(i=0;i<NUM_OF_ZEROS;i++)xil_printf("ADDRESS OF_ZEROS (%d)=%d\r\n",i,ZeroInsertion_location[i]);
//	}
	/////////////////////////////////////////////////
	//stuffing zeros
	//u8 bits_stuffed[sizeof(bits)+NUM_OF_ZEROS];
	u32 data_len = sizeof(bits)+NUM_OF_ZEROS;//sizeof(bits_stuffed);
	u8 k=0;
	u32 addr;
	for(addr=0;addr<sizeof(bits);addr++)
	{
		if((addr==ZeroInsertion_location[k]) && (NUM_OF_ZEROS>0) && (k< NUM_OF_ZEROS))
		{
			*(SourceAddr+addr+k)=SC0;
			*(SourceAddr+addr+k+1)=bits[addr];//SC0;
			k++;

		}
		else *(SourceAddr+addr+k)=bits[addr];
	}


	//if(verbose_mode)xil_printf("BITS STUFFED ARRAY\r\n");
//	j=0;for(i=0;i<sizeof(bits_stuffed);i++)
//		{

	//	if(verbose_mode)xil_printf("%02x\t",bits_stuffed[i]);
	//		if(j==7){
	//			if(verbose_mode)	xil_printf("\r\n");
	//			j=0;}
	//		else j++;
	//	}



//u32 data_len = sizeof(bits_stuffed);
//for (i=0;i<data_len;i++){
//	*(SourceAddr+i)=bits_stuffed[i];

//	if(verbose_mode)xil_printf("\r\nSourceBuffer(%d)=%x  bits_stuffed(%d)=%x\r\n",i,*(SourceAddr+i),i,*(bits_stuffed+i));
//}
	/* Transmit the Data Stream */
		Status = TxSend(&FifoInstance, SourceAddr,data_len,verbose_mode);
		if (Status != XST_SUCCESS){
			xil_printf("Transmisson of Data failed\n\r");
			return XST_FAILURE;
		}

	//check_fifo( SourceAddr,data_len,verbose_mode);//bits_stuffed);


	/*if(mode==READ){


				p = calloc(10, sizeof(u8) );  // Make u8 array of 10 elements


			}

			else {


						p = calloc(14, sizeof(u8) );  // Make u8 array of 14 elements


					}
*/







//	xil_printf("\r\nsizeof(bits)=%d\r\n",sizeof(bits));
//	xil_printf("\r\nsizeof(bits_stuffed)=%d\r\n",sizeof(bits_stuffed));
}


signed configureADC(u8 channel)
{
	unsigned  status;
	ByteCount=3;//sizeof(TxMsgPtr);
if (channel == 0) TxMsgPtr[1] = 0xc4;//ch0
else              TxMsgPtr[1] = 0xd4;//ch1
		////////write to config register//////////////////
		status=  XIic_Send	(	XPAR_AXI_IIC_0_BASEADDR,
				0x49,
				TxMsgPtr,
				ByteCount,
				0
				);
		return status;
}
signed short ReadADC()
{
	unsigned  status;


		//u8 RcvBuffer [2]={0x00,0x00};

		//			u16 config_register;
					//XIic * 	InstancePtr;



	////////write to config register//////////////////
	/*ByteCount=3;//sizeof(TxMsgPtr);
	status=  XIic_Send	(	XPAR_AXI_IIC_0_BASEADDR,
			0x49,
			TxMsgPtr,
			ByteCount,
			0
			);
*/



	//////////////////////write to adress pointer register to point config register////////////////

	/*do{
		 config_register=0;
	ByteCount=1;//sizeof(TxMsgPtr_config);
	status=  XIic_Send	(	XPAR_AXI_IIC_0_BASEADDR,
					0x49,
					TxMsgPtr_config,
					ByteCount,
					0
					);

	//////////////read config register//////////////////


	Num_Bytes_received=XIic_Recv(
									XPAR_AXI_IIC_0_BASEADDR,
									0x49,
									RcvBuffer,
									2,//sizeof(RcvBuffer),
									0
									);
	config_register=(RcvBuffer[0] <<8)|RcvBuffer[1];
	config_register=config_register & 0x8000;
	//xil_printf("config_register=%04x\r\n ",config_register);
	}while(	config_register!=0x8000);*/
//////////////////////write to adress pointer register to point conversion register////////////////
		ByteCount=1;//sizeof(TxMsgPtr_conversion);
		status=  XIic_Send	(	XPAR_AXI_IIC_0_BASEADDR,
				0x49,
				TxMsgPtr_conversion,
				ByteCount,
				0
				);

	//usleep(2500);usleep(2500);usleep(2500);usleep(2500);
	//usleep(2500);usleep(2500);usleep(2500);usleep(2500);
	//usleep(2500);usleep(2500);usleep(2500);usleep(2500);


	/////////////////////////read conversion register//////////////////
	Num_Bytes_received=XIic_Recv(
								XPAR_AXI_IIC_0_BASEADDR,
								0x49,
								RcvBuffer,
								2,//sizeof(RcvBuffer),
								0
								);


	//xil_printf(" MSB=%x :   LSB=  %x\t", RcvBuffer[0], RcvBuffer[1]);
	//xil_printf(" %x\r\n",(RcvBuffer[0]<<8 |RcvBuffer[1]));
	//printf(" %f\r\n",(float)((signed short)(RcvBuffer[0]<<8 |RcvBuffer[1])*0.0000625));
return (signed short)(RcvBuffer[0]<<8 |RcvBuffer[1]);
}



u32 AdjustIref(u32 *SourceAddr,u32 *DestinationBuffer)
{

	u32 register_address;
	u32 register_data;
	u8 verbose_mode=0;
	u8 mode=0;
	u32 read_data;
for(u32 i=0;i<(MAX_DATA_BUFFER_SIZE * WORD_SIZE);i++)
	{
	*(SourceAddr+i)=0;
	*(DestinationBuffer+i)=0;

	}
//u32* SourceAddr2 = calloc()
	/////////////write to GBL_CFG_RUN register for RUN bit=1////////////
	     register_address = GBL_CFG_RUN;
	     register_data    = 0X00000001;//SLEEP/RUN BIT=1;
	     mode             = 1;//write transaction on hdlc slow control
	     HDLC_Tx(&FifoInstance,register_address , register_data,mode,SourceAddr,verbose_mode);//transmit of sc data
	     HDLC_Rx(&FifoInstance,DestinationBuffer,ReceiveLength,mode,verbose_mode);//receive acknowledge of sc data


	     mode             = 0;//write transaction on hdlc slow control
	     HDLC_Tx(&FifoInstance,register_address , register_data,mode,SourceAddr,verbose_mode);//transmit of sc data
	     read_data=HDLC_Rx(&FifoInstance,DestinationBuffer,ReceiveLength,mode,verbose_mode);//receive acknowledge of sc data
	     xil_printf("GBL_CFG_RUN= %x,\r\n",read_data);
	////////////////////////////////////////////////////////////////////
	     do{
	/////////////read  GBL_CFG_CTR_4 register ////////////
	     register_address = GBL_CFG_CTR_4;//imon
	     //register_data    = 0X00000003;//MON SEL=0(imon) MON GAIN=0 VREF_ADC=3
	     mode             = 0;//read transaction on hdlc slow control
	     HDLC_Tx(&FifoInstance,register_address , register_data,mode,SourceAddr,verbose_mode);//transmit of sc data
	     read_data = HDLC_Rx(&FifoInstance,DestinationBuffer,ReceiveLength,mode,verbose_mode);//receive acknowledge of sc data
	     xil_printf("\r\nInitial value of gbl_cfg_ctr_4 =%x\r\n",read_data);
	     //////////////////write gbl_cfg_4 register//////////////////////////////////////////////////
		 MON_GAIN_BITS = MON_GAIN_1;
	     VREF_ADC_BITS =  VREF_ADC_3;
	     MON_SEL_BITS  = IREF;
	     register_data    = (u32)((read_data & 0xfffffc40) | VREF_ADC_BITS | MON_GAIN_BITS | MON_SEL_BITS);//MON SEL=0(imon) MON GAIN=0 VREF_ADC=3
		 //register_data    = ((read_data & 0xfffffc40) | 0X00000300);//MON SEL=0(imon) MON GAIN=0 VREF_ADC=3
		 mode             = 1;//write transaction on hdlc slow control
		 HDLC_Tx(&FifoInstance,register_address , register_data,mode,SourceAddr,verbose_mode);//transmit of sc data
		 HDLC_Rx(&FifoInstance,DestinationBuffer,ReceiveLength,mode,verbose_mode);//receive acknowledge of sc data
		///////////////////re read to verify write /////////////////////////////////////////////////
		//register_address = GBL_CFG_CTR_4;//imon
				//register_data    = ((read_data & 0x0000002f) | 0X00000003);//MON SEL=0(imon) MON GAIN=0 VREF_ADC=3
		mode             = 0;//write transaction on hdlc slow control
		HDLC_Tx(&FifoInstance,register_address , register_data,mode,SourceAddr,verbose_mode);//transmit of sc data
		read_data=HDLC_Rx(&FifoInstance,DestinationBuffer,ReceiveLength,mode,verbose_mode);//receive acknowledge of sc data
		////////////////////////////////////////////////////////////////////
		xil_printf("data written= %x, readback_data = %x, match = %d\r\n",register_data,read_data,register_data==read_data);
		}while(!(register_data==read_data));

		//register_data    = 2;//iref
		///////////////read IREF Value//////////////////
		mode=0;
		register_address = GBL_CFG_CTR_5;//IMON,  6 BIT REGISTER
		HDLC_Tx(&FifoInstance,register_address , register_data,mode,SourceAddr,verbose_mode);//transmit of sc data
		register_data = HDLC_Rx(&FifoInstance,DestinationBuffer,ReceiveLength,mode,verbose_mode);//read value in read mode or receive acknowledge of sc data in write mode
		//printf("Initial _iref  = %x \r\n",register_data);
		///////////////////////////////////////////////////////

		mode= 1;//write sc
		double imon_adc;

u8 set=0;
configureADC(1);//channel 1
	do{

		HDLC_Tx(&FifoInstance,register_address , register_data,mode,SourceAddr,verbose_mode);//transmit of sc data
		HDLC_Rx(&FifoInstance,DestinationBuffer,ReceiveLength,mode,verbose_mode);//receive acknowledge of sc data

		//usleep(1000);usleep(1000);usleep(1000);
		//usleep(1000);usleep(1000);usleep(1000);
		//usleep(1000);usleep(1000);usleep(1000);
		imon_adc=ReadADC()*-0.0625;//read value from external ADC IN MILLIVOLTS
		printf("%x ,  %f mv\r\n",register_data, imon_adc);

		if(imon_adc>100){
			if((imon_adc-100)>1)register_data--;
			else set=1;
		}
		else
		{
			if((100-imon_adc)>1)register_data++;
			else
				set=1;

		}

		//printf("%f\r\n",imon_adc-100);
	}while(set!=1);
	printf("Iref= %x , imon= %f mv\r\n",register_data, imon_adc);

	return register_data;// return iref register value

}


int CalibrateADC(u32 *SourceAddr,u32 *DestinationBuffer,u8 calibration_mode)
{

	u32 register_address;
	u32 register_data;
	u8 verbose_mode=0;
	u8 mode=0;
	u32 read_data;
	u32 adc_0,adc_1;
	double imon_adc,vmon_adc;
	/////////////write to GBL_CFG_RUN register for RUN bit=1////////////
	     register_address = GBL_CFG_RUN;
	     register_data    = 0X00000001;//SLEEP/RUN BIT=1;
	     mode             = 1;//write transaction on hdlc slow control
	     HDLC_Tx(&FifoInstance,register_address , register_data,mode,SourceAddr,verbose_mode);//transmit of sc data
	     HDLC_Rx(&FifoInstance,DestinationBuffer,ReceiveLength,mode,verbose_mode);//receive acknowledge of sc data
	////////////////////////////////////////////////////////////////////

	/////////////write to GBL_CFG_CTR_4 register ////////////
		register_address = GBL_CFG_CTR_4;//vmon=adc)vref
		u8 loop=0;
		//u32 MON_SEL_BITS=0;
		//u32 MON_GAIN_BITS=0;
		//u32 VREF_ADC_BITS=0;

		MON_GAIN_BITS = MON_GAIN_1;
		//MON_SEL_BITS  = ADC_VREF;
///AAMIR
		////////////////set vref_adc=3////////////////
		VREF_ADC_BITS =  VREF_ADC_3;
		register_data    = (u32)(VREF_ADC_BITS | MON_GAIN_BITS | MON_SEL_BITS);//MON SEL=0(imon) MON GAIN=0 VREF_ADC=3
				mode             = 1;//write transaction on hdlc slow control
			//	HDLC_Tx(register_address , register_data,mode,SourceAddr,verbose_mode);//transmit of sc data
			//	HDLC_Rx(DestinationBuffer,ReceiveLength,mode,verbose_mode);//receive acknowledge of sc data
				//double vmon_adc=ReadADC()*0.0625;//read value from external ADC IN MILLIVOLTS
				//printf("GBL_CFG_CTR_4 = %08x, vmon= %f,vref_adc= %d\r\n",register_data,vmon_adc,(VREF_ADC_BITS>>8));

/////////////////////////////adc calibration starts ////////////////////////////////////
				////set monitoring register to 2
				register_address = GBL_CFG_CTR_4;//imon = preamp inp trans

				mode             = 0;//read transaction on hdlc slow control
				HDLC_Tx(&FifoInstance,register_address , register_data,mode,SourceAddr,verbose_mode);//transmit of sc data
				read_data = HDLC_Rx(&FifoInstance,DestinationBuffer,ReceiveLength,mode,verbose_mode);//receive acknowledge of sc data
				printf("GBL_CFG_CTR_4 read_data = %X\r\n",read_data);
				MON_SEL_BITS  = PREAMP_INP_TRAN;//select preamp_inp_trans dac imon = preamp_inp_trans
				register_data    = (u32)(read_data | VREF_ADC_BITS | MON_GAIN_BITS | MON_SEL_BITS);//MON SEL=0(imon) MON GAIN=0 VREF_ADC=3
				mode             = 1;//write transaction on hdlc slow control
				HDLC_Tx(&FifoInstance,register_address , register_data,mode,SourceAddr,verbose_mode);//transmit of sc data
				HDLC_Rx(&FifoInstance,DestinationBuffer,ReceiveLength,mode,verbose_mode);//receive acknowledge of sc data

			//	HDLC_Tx(register_address , register_data,mode,SourceAddr,verbose_mode);//transmit of sc data
			//	HDLC_Rx(DestinationBuffer,ReceiveLength,mode,verbose_mode);//receive acknowledge of sc data




				mode             = 0;//read transaction on hdlc slow control
				HDLC_Tx(&FifoInstance,register_address , register_data,mode,SourceAddr,verbose_mode);//transmit of sc data
				read_data=HDLC_Rx(&FifoInstance,DestinationBuffer,ReceiveLength,mode,verbose_mode);//receive acknowledge of sc data

				printf("GBL_CFG_CTR_4 write_value = %X ,  read_value = %X, %d\r\n",register_data,read_data,register_data==read_data);
				//imon_adc=ReadADC()*-0.0625;//read value from external ADC IN MILLIVOLTS
				//printf("GBL_CFG_CTR_4 = %08x, imon= %f,vref_adc= %d\r\n",register_data,vmon_adc,(VREF_ADC_BITS>>8));
	////////////////////////////////////////////////////////////////////
				//AAMIR

		///////loop through different values of DAC Preamp_BiasInputTransistor/////////////
				 //register_address 	= 	GBL_CFG_BIAS_1;
				 register_data		=	0;
				 mode				=	1;
				 xil_printf("wait \r\n");
				 do{
					 mode				=	1;
					 register_address 	= 	GBL_CFG_BIAS_1;
					 HDLC_Tx(&FifoInstance,register_address , register_data,mode,SourceAddr,verbose_mode);//transmit of sc data
				 	 HDLC_Rx(&FifoInstance,DestinationBuffer,ReceiveLength,mode,verbose_mode);//receive acknowledge of sc data}while(register_data<255);


                    /////////////verify correct write by reading same data//////////////////////
				 	// mode=0;
				 	//HDLC_Tx(register_address , register_data,mode,SourceAddr,verbose_mode);//transmit of sc data
				 	//read_data=HDLC_Rx(DestinationBuffer,ReceiveLength,mode,verbose_mode);//receive acknowledge of sc data}while(register_data<255);

				 	imon_adc= ReadADC()*-0.0625;//read external adc
				 	////////////////read      internal  adc/////////////
				 	 mode				=	0;
				 	 register_address 	= 	ADC_READ_0;
				 	 HDLC_Tx(&FifoInstance,register_address , 0,mode,SourceAddr,verbose_mode);//transmit of sc data
				 	 adc_0=HDLC_Rx(&FifoInstance,DestinationBuffer,ReceiveLength,mode,verbose_mode);//receive acknowledge of sc data}while(register_data<255);

				 	register_address 	= 	ADC_READ_1;
				 	HDLC_Tx(&FifoInstance,register_address , 0,mode,SourceAddr,verbose_mode);//transmit of sc data
				 	adc_1=HDLC_Rx(&FifoInstance,DestinationBuffer,ReceiveLength,mode,verbose_mode);//receive acknowledge of sc data}while(register_data<255);

				 	//printf("DAC VALUE = %d, ADC_0= %d,ADC_1 =%d, Imon = %fmv\r\n",read_data,adc_0,adc_1,imon_adc);
				 	register_data+=2;

				 }while(register_data<256);

				 	 xil_printf("done_adc calibration\r\n");





		/*do{
				 if(loop==0)		VREF_ADC_BITS =  VREF_ADC_0;
			else if(loop==1)		VREF_ADC_BITS =  VREF_ADC_1;
			else if(loop==2)		VREF_ADC_BITS =  VREF_ADC_2;
			else if(loop==3)		VREF_ADC_BITS =  VREF_ADC_3;
			else 					VREF_ADC_BITS =  VREF_ADC_0;
		register_data    = (u32)(VREF_ADC_BITS | MON_GAIN_BITS | MON_SEL_BITS);//MON SEL=0(imon) MON GAIN=0 VREF_ADC=3
		mode             = 1;//write transaction on hdlc slow control
		HDLC_Tx(register_address , register_data,mode,SourceAddr,verbose_mode);//transmit of sc data
		HDLC_Rx(DestinationBuffer,ReceiveLength,mode,verbose_mode);//receive acknowledge of sc data
		double vmon_adc=ReadADC()*0.0625;//read value from external ADC IN MILLIVOLTS
				//printf("%x ,  %f mv\r\n",register_data, imon_adc);
		printf("GBL_CFG_CTR_4 = %08x, vmon= %f,vref_adc= %d\r\n",register_data,vmon_adc,(VREF_ADC_BITS>>8));

		}while(++loop<4);*/
/*


		register_address = GBL_CFG_CTR_5;//IMON,  6 BIT REGISTER
		//register_data    = 2;//iref
		///////////////read IREF Value//////////////////
		mode=0;
		HDLC_Tx(register_address , register_data,mode,SourceAddr,verbose_mode);//transmit of sc data
		register_data = HDLC_Rx(DestinationBuffer,ReceiveLength,mode,verbose_mode);//read value in read mode or receive acknowledge of sc data in write mode
		printf("register_data = %x \r\n",register_data);
		///////////////////////////////////////////////////////

		mode= 1;//write sc
		double imon_adc;

u8 set=0;
	do{

		HDLC_Tx(register_address , register_data,mode,SourceAddr,verbose_mode);//transmit of sc data
		HDLC_Rx(DestinationBuffer,ReceiveLength,mode,verbose_mode);//receive acknowledge of sc data

		//usleep(1000);usleep(1000);usleep(1000);
		//usleep(1000);usleep(1000);usleep(1000);
		//usleep(1000);usleep(1000);usleep(1000);
		imon_adc=ReadADC()*-0.0625;//read value from external ADC IN MILLIVOLTS
		//printf("%x ,  %f mv\r\n",register_data, imon_adc);

		if(imon_adc>100){
			if((imon_adc-100)>1)register_data--;
			else set=1;
		}
		else
		{
			if((100-imon_adc)>1)register_data++;
			else
				set=1;

		}

		//printf("%f\r\n",imon_adc-100);
	}while(set!=1);
	printf("Iref= %x , imon= %f mv\r\n",register_data, imon_adc);

*/
	return 0;

}

void print_echo_app_header()
{
    xil_printf("%20s %6d %s\r\n", "echo server",
                        echo_port,
                        "$ telnet <board_ip> 7");

}





int CalibrateCAL_DAC(u32 *SourceAddr,u32 *DestinationBuffer,u8 calibration_mode)
{

/* This routine will find fC values corresponding to the DAC values
 * of the CAL_DAC//////////////////////////////////
 */
	u32 register_address;
	u32 register_data;
	u8 verbose_mode=0;
	u8 mode=0;
	u32 read_data;
	//double imon_adc,vmon_adc;
	double step_voltage, base_voltage,charge = 0;

	/////////////write to GBL_CFG_RUN register for RUN bit=1////////////
	     register_address = GBL_CFG_RUN;
	     register_data    = 0X00000001;//SLEEP/RUN BIT=1;
	     mode             = 1;//write transaction on hdlc slow control
	     HDLC_Tx(&FifoInstance,register_address , register_data,mode,SourceAddr,verbose_mode);//transmit of sc data
	     HDLC_Rx(&FifoInstance,DestinationBuffer,ReceiveLength,mode,verbose_mode);//receive acknowledge of sc data
	////////////////////////////////////////////////////////////////////

	/////////////write to GBL_CFG_CTR_4 register ////////////
	     ////////////////set MONTORING REGISTER////////////////
		register_address = GBL_CFG_CTR_4;//vmon=calib vstep


				VREF_ADC_BITS 	=  	    VREF_ADC_3;
				MON_SEL_BITS	=		CALIB_V_STEP;
				MON_GAIN_BITS 	= 		MON_GAIN_1;
				register_data   = (u32)(VREF_ADC_BITS | MON_GAIN_BITS | MON_SEL_BITS);//MON SEL=CALIB VSTEP MON GAIN=0 VREF_ADC=3
				mode            = 1;//write transaction on hdlc slow control
				HDLC_Tx(&FifoInstance,register_address , register_data,mode,SourceAddr,verbose_mode);//transmit of sc data
				HDLC_Rx(&FifoInstance,DestinationBuffer,ReceiveLength,mode,verbose_mode);//receive acknowledge of sc data
				//double vmon_adc=ReadADC()*0.0625;//read value from external ADC IN MILLIVOLTS
				mode            = 0;//write transaction on hdlc slow control
				HDLC_Tx(&FifoInstance,register_address , register_data,mode,SourceAddr,verbose_mode);//transmit of sc data
				read_data=HDLC_Rx(&FifoInstance,DestinationBuffer,ReceiveLength,mode,verbose_mode);//receive acknowledge of sc data
				xil_printf("cfg_ctr_4 set value , READ_DATA = %x \r\n", read_data);

				////////////////set GBL_CFG_CAL_0 REGISTER SET CAL_SEL_POL =? ////////////////


				//register_address 	= 	GBL_CFG_CAL_0;
				register_address 	= 	GBL_CFG_CAL_0;
				register_data       =   0x00004001;//cal_sel_pol=1  cal mode 01 voltage pulse
				mode                =   1;//write transaction on hdlc slow control
				HDLC_Tx(&FifoInstance,register_address , register_data,mode,SourceAddr,verbose_mode);//transmit of sc data
				HDLC_Rx(&FifoInstance,DestinationBuffer,ReceiveLength,mode,verbose_mode);//receive acknowledge of sc data
				//usleep(1000);usleep(1000);usleep(1000);usleep(1000);usleep(1000);
				base_voltage = ReadADC()*0.0000625;//v

				register_data       =   0x00000001;//cal_sel_pol=0  cal mode 01 voltage pulse
				mode                =   1;//write transaction on hdlc slow control
				HDLC_Tx(&FifoInstance,register_address , register_data,mode,SourceAddr,verbose_mode);//transmit of sc data
				HDLC_Rx(&FifoInstance,DestinationBuffer,ReceiveLength,mode,verbose_mode);//receive acknowledge of sc data



		///////loop through cal_dac values /////////////


				 mode				=	0;
				 HDLC_Tx(&FifoInstance,register_address , register_data,mode,SourceAddr,verbose_mode);//transmit of sc data
				 read_data    =    HDLC_Rx(&FifoInstance,DestinationBuffer,ReceiveLength,mode,verbose_mode);//receive acknowledge of sc data}
				 xil_printf("Initial value , READ_DATA = %x \r\n", read_data);

				 CAL_DAC          =   0;
				 register_data		=	(u32)(read_data |((u32)CAL_DAC<<2));
				 mode				=	1;
				 do{


					 //register_address 	= 	GBL_CFG_CAL_0;
					 HDLC_Tx(&FifoInstance,register_address , register_data,mode,SourceAddr,verbose_mode);//transmit of sc data
				 	 HDLC_Rx(&FifoInstance,DestinationBuffer,ReceiveLength,mode,verbose_mode);//receive acknowledge of sc data}

				 	 step_voltage = ReadADC()*0.0000625;//volts
				 	 charge       = (step_voltage - base_voltage)*100;
				 	printf("CAL_DAC= %d , GBL_CFG_CAL_0= %x , step_voltage= %fmv, base_voltage= %fmv, charge= %ffC\r\n",CAL_DAC,register_data,step_voltage*1000,base_voltage*1000,charge);
				 	 CAL_DAC++;
				 	register_data		=	(u32)(read_data |((u32)CAL_DAC<<2));



				 }while(CAL_DAC<255);

				 	 xil_printf("done_CAL_DAC  calibration\r\n");






	return 0;

}

int Scurve(u32 *SourceAddr,u32 *DestinationBuffer,u16 Latency, u16 num_of_triggers,u8 calibration_mode)
{
u64 p=9;
/* This routine will get the scurves for all the channels
 *//////////////////////////////////

	u32 register_address;
	u32 register_data;
	u8 verbose_mode=0;
	u8 mode=0;
	u32 read_data;
	u8 channel;

	for (int outer=0;outer<128;outer++)
		for (int inner=0;inner<128;inner++)
			Hit_array[outer][inner]=0;

	//double imon_adc,vmon_adc;
	//double step_voltage, base_voltage,charge = 0;
	//u8 CAL_DAC;
	/////////////write to GBL_CFG_RUN register for RUN bit=1////////////
	     register_address = GBL_CFG_RUN;
	     register_data    = 0X00000001;//SLEEP/RUN BIT=1;
	     mode             = 1;//write transaction on hdlc slow control
	     HDLC_Tx(&FifoInstance,register_address , register_data,mode,SourceAddr,verbose_mode);//transmit of sc data
	     HDLC_Rx(&FifoInstance,DestinationBuffer,ReceiveLength,mode,verbose_mode);//receive acknowledge of sc data
	////////////////////////////////////////////////////////////////////
	     ///////////////send run mode fast command////////////////
	     *SourceAddr =RUN_MODE;

	     Transmit_fast_command(&FifoInstance, FIFO_DEV_ID,SourceAddr,1,verbose_mode);


	     ////////////////////////SET NOMINAL VALUES FOR THE FRONT END//////////////////////////
	     /////////////////PREAMP  settings ,pre_i_bit,pre_i_bsf,pre_i_blcc,pre_vref

	     	 	 register_address 	 = 	 GBL_CFG_BIAS_2;
	    	     register_data       =   (u32)(PRE_I_BLCC<<8 |PRE_VREF);//pre_i_blcc,pre_vref
	    	     mode                =   1;//write transaction on hdlc slow control
	    	     HDLC_Tx(&FifoInstance,register_address , register_data,mode,SourceAddr,verbose_mode);//transmit of sc data
	    	     HDLC_Rx(&FifoInstance,DestinationBuffer,ReceiveLength,mode,verbose_mode);//receive acknowledge of sc data



	    	     register_address 	 = 	 GBL_CFG_BIAS_1;
	    	     register_data       =   (u32)( (PRE_I_BSF<<8) |PRE_I_BIT);//,pre_i_bit,pre_i_bsf
	    	     mode                =   1;//write transaction on hdlc slow control
	    	     HDLC_Tx(&FifoInstance,register_address , register_data,mode,SourceAddr,verbose_mode);//transmit of sc data
	    	     HDLC_Rx(&FifoInstance,DestinationBuffer,ReceiveLength,mode,verbose_mode);//receive acknowledge of sc data


	    	    /////NOMINAL VALUES FOR THE SHAPER ///////////////////////////////////

	    	     register_address 	 = 	 GBL_CFG_BIAS_3;
	    	     register_data       =   (u32)( (SH_I_BFCAS<<8) |SH_I_BDIFF);//SH_I_BFCAS, SH_I_BDIFF
	    	     mode                =   1;//write transaction on hdlc slow control
	    	     HDLC_Tx(&FifoInstance,register_address , register_data,mode,SourceAddr,verbose_mode);//transmit of sc data
	    	     HDLC_Rx(&FifoInstance,DestinationBuffer,ReceiveLength,mode,verbose_mode);//receive acknowledge of sc data

	    	     /////NOMINAL VALUES FOR THE SD ///////////////////////////////////

	    	     register_address 	 = 	 GBL_CFG_BIAS_4;
	    	     mode=0;
	    	     HDLC_Tx(&FifoInstance,register_address , register_data,mode,SourceAddr,verbose_mode);//transmit of sc data
	    	     read_data = HDLC_Rx(&FifoInstance,DestinationBuffer,ReceiveLength,mode,verbose_mode);


	    	     register_data       =   (u32)( (read_data &  0xff00) | SD_I_BDIFF);//SD_I_BDIFF
	    	     mode                =   1;//write transaction on hdlc slow control
	    	     HDLC_Tx(&FifoInstance,register_address , register_data,mode,SourceAddr,verbose_mode);//transmit of sc data
	    	     HDLC_Rx(&FifoInstance,DestinationBuffer,ReceiveLength,mode,verbose_mode);//receive acknowledge of sc data

	    	     register_address 	 = 	 GBL_CFG_BIAS_5;
	    	     register_data       =   (u32)( (SD_I_BSF<<8) | SD_I_BFCAS);//SD_I_BSF , SD_I_BFCAS
  	    	     mode                =   1;//write transaction on hdlc slow control
	    	     HDLC_Tx(&FifoInstance,register_address , register_data,mode,SourceAddr,verbose_mode);//transmit of sc data
	    	     HDLC_Rx(&FifoInstance,DestinationBuffer,ReceiveLength,mode,verbose_mode);//receive acknowledge of sc data



	     /////////////////////////////////////////////////////////////////////////////////////

	     ///////////set cal mode to 1 cal sel pol to 0   (GBL CFG CAL 0 register)/////////////////////////////////////////////////////////

	     register_address 	 = 	 GBL_CFG_CAL_0;
	     register_data       =   0x00000001;//cal_sel_pol=1  cal mode 01 voltage pulse
	     mode                =   1;//write transaction on hdlc slow control
	     HDLC_Tx(&FifoInstance,register_address , register_data,mode,SourceAddr,verbose_mode);//transmit of sc data
	     HDLC_Rx(&FifoInstance,DestinationBuffer,ReceiveLength,mode,verbose_mode);//receive acknowledge of sc data

	     ///////////set SEL COMP MODE TO 1  (GBL CFG CTR3 register)/////////////////////////////////////////////////////////

	     register_address 	 = 	 GBL_CFG_CTR_3;
	     mode                =   0;//READ transaction on hdlc slow control
	     HDLC_Tx(&FifoInstance,register_address , register_data,mode,SourceAddr,verbose_mode);//transmit of sc data
	     read_data=HDLC_Rx(&FifoInstance,DestinationBuffer,ReceiveLength,mode,verbose_mode);//receive acknowledge of sc data

	     register_data       =  (read_data & 0xfffe) | 0x00000001;//SEL COMP MODE = 01 ARMING (CFD OUTPUT MODE =  ARMING)
	     mode                =  1;//write transaction on hdlc slow control
	     HDLC_Tx(&FifoInstance,register_address , register_data,mode,SourceAddr,verbose_mode);//transmit of sc data
	     HDLC_Rx(&FifoInstance,DestinationBuffer,ReceiveLength,mode,verbose_mode);//receive acknowledge of sc data
	    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	    ///////////set ARM DAC for all channels to 100 /////////////////////////////////////////////////////////

	     	 	register_address  =  0;
	     	    mode              =  0; //READ transaction on hdlc slow control
	     	    HDLC_Tx(&FifoInstance,register_address , register_data,mode,SourceAddr,verbose_mode);//transmit of sc data
	     	    read_data=HDLC_Rx(&FifoInstance,DestinationBuffer,ReceiveLength,mode,verbose_mode);//receive acknowledge of sc data

	     	   register_data       =  (read_data & 0xFFFFffc0) | 100;//
	     	   mode=1;
	     for (register_address=0;register_address<129;register_address++)
	     {

	    	  HDLC_Tx(&FifoInstance,register_address , register_data,mode,SourceAddr,verbose_mode);//transmit of sc data
	    	  HDLC_Rx(&FifoInstance,DestinationBuffer,ReceiveLength,mode,verbose_mode);


	     }

///////////////SET CAL DUR to 200//////////////



	    	  register_address 	 = 	 GBL_CFG_CAL_1;
	    	  mode                =   0;//READ transaction on hdlc slow control
	    	  HDLC_Tx(&FifoInstance,register_address , register_data,mode,SourceAddr,verbose_mode);//transmit of sc data
	    	  read_data=HDLC_Rx(&FifoInstance,DestinationBuffer,ReceiveLength,mode,verbose_mode);//receive acknowledge of sc data
	    	  xil_printf("cal dur sub register initial =%x\r\n",read_data);

	    	  register_data       =  (read_data & 0xFFFFFE00) | 200;//200 CAL DUR
	    	  mode                =  1;//write transaction on hdlc slow control
	    	  HDLC_Tx(&FifoInstance,register_address , register_data,mode,SourceAddr,verbose_mode);//transmit of sc data
	    	  HDLC_Rx(&FifoInstance,DestinationBuffer,ReceiveLength,mode,verbose_mode);//receive acknowledge of sc data
	    	    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	    	  ///////////////SET PS to 7//////////////



	    	  	    	  register_address 	 = 	 GBL_CFG_CTR_0;
	    	  	    	  mode                =   0;//READ transaction on hdlc slow control
	    	  	    	  HDLC_Tx(&FifoInstance,register_address , register_data,mode,SourceAddr,verbose_mode);//transmit of sc data
	    	  	    	  read_data=HDLC_Rx(&FifoInstance,DestinationBuffer,ReceiveLength,mode,verbose_mode);//receive acknowledge of sc data
                          xil_printf("PS sub register initial =%x\r\n",read_data);

	    	  	    	  register_data       =  (read_data & 0x1FFF) | 0XE000;//PS =7
	    	  	    	  mode                =  1;//write transaction on hdlc slow control
	    	  	    	  HDLC_Tx(&FifoInstance,register_address , register_data,mode,SourceAddr,verbose_mode);//transmit of sc data
	    	  	    	  HDLC_Rx(&FifoInstance,DestinationBuffer,ReceiveLength,mode,verbose_mode);//receive acknowledge of sc data
	    	  	    	  xil_printf("PS sub register write  =%x\r\n",register_data);
	    	  	    	  /////////////////////////////////////loop through all channels//////////////////////////////////////////////////////////////////////////////////////////////
	    	  	    	/*//////////////fill buffer///////////////////////////
	    	  	    	  for(int j=0;j<=(Latency+1);j++)
	    	  	    								{
	    	  	    									//	if(j==0)
	    	  	    									//	*(SourceAddr + j) = RUN_MODE;//FAST COMMAND
	    	  	    									 if(j==0)
	    	  	    										*(SourceAddr + j)  = CAL_PULSE;//FAST COMMAND

	    	  	    									else if(j==(Latency+1))
	    	  	    										*(SourceAddr + j)  = LV1A;

	    	  	    									else if (j % 2==0)
	    	  	    										*(SourceAddr + j) = 0x00;

	    	  	    									else
	    	  	    										*(SourceAddr + j) = 0xff;

	    	  	    							xil_printf("Trg Source buffer[%d]= %x\r\n",j,*(SourceAddr + j));

	    	  	    								}



	    	  	    	 xil_printf(" data_len=%d\r\n", data_len);



	    	  	    	  ///////////////////////////////////////////////////////////////////////////////////////*/

	    	  	    	u16  data_len = Latency+2;

	    	  	    	register_address 	= 	GBL_CFG_CAL_0;
	    	  	    	mode				=	0;
	    	  	    	HDLC_Tx(&FifoInstance,register_address , register_data,mode,SourceAddr,verbose_mode);//transmit of sc data
	    	  	    	DATA_GBL_CFG_CAL_0    =    HDLC_Rx(&FifoInstance,DestinationBuffer,ReceiveLength,mode,verbose_mode);//receive acknowledge of sc data}


	    	  	    	  channel=0;
	    	  	    	  do{
	    	  	    		  xil_printf("CHANNEL %d\r\n",channel);
	    	  	    	  ///////////////////////////SET CHANNEL FOR CALIBRATION //////////////////////
	    	  	    /*	register_address       = (u32) channel;//CH0
	    	  	    	mode                =   0;//READ transaction on hdlc slow control
	    	  	    	HDLC_Tx(register_address , register_data,mode,SourceAddr,verbose_mode);//transmit of sc data
	    	  	    	read_data=HDLC_Rx(DestinationBuffer,ReceiveLength,mode,verbose_mode);//receive acknowledge of sc data
	    	  	    	//xil_printf("channel register initial =%x\r\n",read_data);

	    	  	    	register_data		   = (read_data & 0xFFFF7FFF) | 0X00008000;//cal =1-
	    	  	    	mode                =  1;//write transaction on hdlc slow control
	    	  	    	HDLC_Tx(register_address , register_data,mode,SourceAddr,verbose_mode);//transmit of sc data
	    	  	    	HDLC_Rx(DestinationBuffer,ReceiveLength,mode,verbose_mode);//receive acknowledge of sc data
	    	  	    	*/

	    	  	    register_address       = channel;
	    	  	    register_data		   = 0x8000;//cal =1-
	    	  	    mode                =  1;//write transaction on hdlc slow control
	    	  	    HDLC_Tx(&FifoInstance,register_address , register_data,mode,SourceAddr,verbose_mode);//transmit of sc data
	    	  	    HDLC_Rx(&FifoInstance,DestinationBuffer,ReceiveLength,mode,verbose_mode);//receive acknowledge of sc data
	    	  	    	//xil_printf("channel register write =%x\r\n",register_data);
	    	  	    	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////loop through cal dac values/////////////////////////
	    	  	    //	register_address 	= 	GBL_CFG_CAL_0;
	    	  	    //	mode				=	0;
	    	  	    //	 HDLC_Tx(register_address , register_data,mode,SourceAddr,verbose_mode);//transmit of sc data
	    	  	    //	DATA_GBL_CFG_CAL_0    =    HDLC_Rx(DestinationBuffer,ReceiveLength,mode,verbose_mode);//receive acknowledge of sc data}
	    	  	    	// xil_printf("Initial value cal dac  , READ_DATA = %x \r\n", read_data);

	    	  	    	 CAL_DAC          =   0;
	    	  	    	register_address 	= 	GBL_CFG_CAL_0;// added this line
	    	  	    	register_data		=	(u32)((DATA_GBL_CFG_CAL_0 & 0xc03) |(CAL_DAC<<2));
	    	  	    	mode				=	1;

	    	  	    	do{
	    	  	    		// xil_printf("CAL_DAC = %d,GBL_CFG_CAL_0= %02X\r\n", CAL_DAC,register_data);
	    	  	    	 //register_address 	= 	GBL_CFG_CAL_0;
	    	  	    	 HDLC_Tx(&FifoInstance,register_address , register_data,mode,SourceAddr,verbose_mode);//transmit of sc data
	    	  	    	 HDLC_Rx(&FifoInstance,DestinationBuffer,ReceiveLength,mode,verbose_mode);//receive acknowledge of sc data}
   					 	// step_voltage = ReadADC()*0.0000625;//volts
   					 	 //charge       = (step_voltage - base_voltage)*100;
   					 	//printf("CAL_DAC= %d , GBL_CFG_CAL_0= %x , step_voltage= %fmv, base_voltage= %fmv, charge= %ffC\r\n",CAL_DAC,register_data,step_voltage*1000,base_voltage*1000,charge);


   					 	/////////////////send calpulse +lv1as/////////////////
 					 	//*SourceAddr =RUN_MODE;
 					 	//Transmit_fast_command(&FifoInstance, FIFO_DEV_ID,SourceAddr,1,verbose_mode);
   					//   xil_printf("Entering in send_calPulse\r\n");
	    	  	    	//xil_printf("CAL_DAC %03d\t" ,CAL_DAC);

	    	  	    	for(int loop=0;loop<num_of_triggers;loop++)
	    	  	    	 {
	    	  	    	 send_Calpulse_LV1As(&FifoInstance, FIFO_DEV_ID,SourceAddr,data_len,0);//send lv1as with latency
	    	  	    	 //usleep(1000);usleep(1000);usleep(1000);

	    	  	    	 DecodeDataPacket(&FifoInstance, FIFO_DEV_ID,Latency,verbose_mode);//receive and decode data packets
	    	  	    //	xil_printf("%02x%02x%02x%02x%02x%02x%02x%02x", datapacket[4],datapacket[5],datapacket[6],datapacket[7],datapacket[8],datapacket[9],datapacket[10],datapacket[11]);
	    	  	    //	xil_printf("%02x%02x%02x%02x%02x%02x%02x%02x\t\t",datapacket[12],datapacket[13],datapacket[14],datapacket[15],datapacket[16],datapacket[17],datapacket[18],datapacket[19]);
	    	  	    	Hit_array[channel][CAL_DAC] += (datapacket [19-channel/8] & mask_array[channel & 0x7]) ? 1:0;

	    	  	    	 }

	    	  	    	//xil_printf("\r\n");
	    	  	    	//xil_printf("HitArray[%d][%d]= %d\r\n",channel,CAL_DAC, Hit_array[channel][CAL_DAC]);
	    	  	    	 ////////////////////////////unselect channel/////////////////

   						    	  	    	//////////////////////////////////////////////////////////////////////////////////////////
   						    	  	   {
   						    	  	   	    	  	    		 CAL_DAC+=1;if(CAL_DAC>255)CAL_DAC=255;
   						    	  	      					 	     register_data		=	(u32)((DATA_GBL_CFG_CAL_0 & 0xc03) |(CAL_DAC << 2));
   						    	  	    					 	 }
   						    	//  	xil_printf("bottom cal dac loop\r\n");
	    	  	    	}while(CAL_DAC<255);

	    	  	    	 register_address       =  (u32)channel;//CH0
	    	  	    	/* mode                =   0;//READ transaction on hdlc slow control
	    	  	    	 HDLC_Tx(register_address , register_data,mode,SourceAddr,verbose_mode);//transmit of sc data
	    	  	    	 read_data=HDLC_Rx(DestinationBuffer,ReceiveLength,mode,verbose_mode);//receive acknowledge of sc data
	    	  	    	 // 	xil_printf("channel register initial =%x\r\n",read_data);

	    	  	    	 register_data		   = (read_data & 0xFFFF7FFF) | 0X00000000;//cal =0
	    	  	    	 mode                =  1;//write transaction on hdlc slow control
	    	  	    	 HDLC_Tx(register_address , register_data,mode,SourceAddr,verbose_mode);//transmit of sc data
	    	  	    	 HDLC_Rx(DestinationBuffer,ReceiveLength,mode,verbose_mode);//receive acknowledge of sc data
	    	  	    	 //	xil_printf("channel register write = %x\r\n",register_data);
*/

	    	  	    	register_data		   = 0;//cal =0
	    	  	    	mode                =  1;//write transaction on hdlc slow control
	    	  	    	HDLC_Tx(&FifoInstance,register_address , register_data,mode,SourceAddr,verbose_mode);//transmit of sc data
	    	  	    	HDLC_Rx(&FifoInstance,DestinationBuffer,ReceiveLength,mode,verbose_mode);//receive acknowledge of sc data
	    	  	    	  }while(channel++<127);//128);





		 	 xil_printf("done scurve calibration\r\n");






	return 0;

}


//int ProcessDataPackets(XLlFifo *InstancePtr, u16 DeviceId,u16 Latency, u16 num_of_triggers,u8 verbose_mode)
//{
//
//
//}
int DecodeDataPacket(XLlFifo *InstancePtr, u16 DeviceId,u16 Latency, u8 verbose_mode)
{

	u32 i,j;
	u32 Rcv_len;
	u8 Header;
	u8 loop;
	u8 indexx;
	u8 PKT_LEN=22;
	//u32* Rcv;
	//Rcv = (u32)xil_calloc(27000,sizeof(u32));
	//if(Rcv==NULL){
	//		xil_printf("memory allocation fail\r\n");
	//		return 0;
	//		}


	Rcv_len = RxReceive(&FifoInstance, DestinationBuffer,verbose_mode);
	//xil_printf("Rcv_len=%d\r\n", Rcv_len);
	   							//	if (Status != XST_SUCCESS){
	   							//		xil_printf("Receiving data failed");
	   							//		return XST_FAILURE;
	   							//	}

	   								//Error = 0;

	   								if(Rcv_len>0 && Rcv_len<MAX_DATA_BUFFER_SIZE)
	   								{
	   								//	xil_printf("rcv_len=%d\r\n",Rcv_len);
	   								//for(i=0;i<Rcv_len;i++)
	   									/////////////////find header of data packet///////////
	   									j=0;i=0;


	   							//	for(i=0;i<num_of_triggers;i++)
	   								{
	   									//j=0;
	   									do
	   									{
	   									datapacket[0] = *(DestinationBuffer+j);
	   									j++;
	   									}while(!(datapacket[0]== 0x1E || j>=Rcv_len));

	   								if(j>=Rcv_len)
	   									{
	   									 Error_Dpkt = 1;
	   									 xil_printf("Error \r\n");
	   									return 0;
   									}



	   								for( indexx = 1;indexx < 22;indexx++,j++)
	   								{
	   									datapacket[indexx]= *(DestinationBuffer+j);
	   								}
	   								//loop++;
	   								//xil_printf("\t\t/////////////////////////////////data packet %d\r\n",i+1);
	   								//xil_printf("\r\nHeader = %02x\r\n",datapacket[0]);
	   								//xil_printf("EC= %02x\r\n",datapacket[1]);
	   								//xil_printf("BC= %04x\r\n",datapacket[2]<<8 | datapacket[3]  );
	   								//xil_printf("%02x%02x%02x%02x%02x%02x%02x%02x", datapacket[4],datapacket[5],datapacket[6],datapacket[7],datapacket[8],datapacket[9],datapacket[10],datapacket[11]);
	   								//xil_printf("%02x%02x%02x%02x%02x%02x%02x%02x\t\t",datapacket[12],datapacket[13],datapacket[14],datapacket[15],datapacket[16],datapacket[17],datapacket[18],datapacket[19]);
	   								//xil_printf("\r\nchecksum= %02x%02x\r\n",datapacket[20],datapacket[21]);
	   								//u16 chk_sum = crc16(datapacket, 20);
  									   								//loop++;
	   								//xil_printf("checksum calculated = %x\r\n",chk_sum);
	   									   								//xil_printf("Loop= %d\r\n",loop);


	   									   								//	xil_printf("rcv length=%d :: data_packet[%d]=%x \r\n ",Rcv_len,i,*(DestinationBuffer+i));
	   								}
	   								Error_Dpkt=0;




	   								/////////////// print EC COUNTER/////////////////

	   								/////////////////print BC counter///////////////

	   								//u8 msb = *(DestinationBuffer+i);i++;
	   								//u8 lsb = *(DestinationBuffer+i);i++;



	   								}
	   								else
	   								{
	   									xil_printf("No data received\r\n");
	   									XLlFifo_Reset(&FifoInstance);
	   								}

//for(i=0;i<Rcv_len;i++)
	   				//			xil_printf("rcv length=%d :: data_packet[%d]=%x \r\n\t\t//////////////////////////////////////////\r\n ",Rcv_len,i,*(DestinationBuffer+i));
	   								//XLlFifo_Reset(InstancePtr);
//free (Rcv);

return 0;
}

void SendReply(int sd, u32 *Buffer, u32 length)
{
	/* handle request */
			if ((nwrote = write(sd, Buffer, length)) < 0) {
				//for(int i=0;i<5*4;i++)nwrote = write(sd, DestinationBuffer, 5*4);
				xil_printf("%s: ERROR responding to client echo request. received = %d, written = %d\r\n",
						__FUNCTION__, n, nwrote);
				xil_printf("Closing socket %d\r\n", sd);
				//break;
			}
}

/* thread spawned for each connection */
void process_echo_request(void *p,XLlFifo *InstancePtr_tx)
{
	int  sd = (int)p;
	//int RECV_BUF_SIZE = 2048;
	//u8 recv_buf[RECV_BUF_SIZE];
	//int n, nwrote;
	//XLlFifo FifoInstance_tx;
	//XLlFifo FifoInstance_rx;

	while (1) {
		/* read a max of RECV_BUF_SIZE bytes from socket */
		if ((n = read(sd, recv_buf, RECV_BUF_SIZE)) < 0) {
			xil_printf("%s: error reading from socket %d, closing socket\r\n", __FUNCTION__, sd);
			break;
		}






		/* break if the recved message = "quit"
		if (!strncmp(recv_buf, "quit", 4)){xil_printf("break strncmp\r\n");
			break;}*/

		/* break if client closed connection */
		if (n <= 0){xil_printf("break n<=0\r\n");
			break;}

		u32 *SourceAddr;
		SourceAddr= SourceBuffer;
		xil_printf("entering decode receive packet\r\n");
		decode_receive_packet(recv_buf,SourceAddr,sd);
        //for(int i=0;i<5*4;i++)xil_printf("destination buffer%x \r\n",*(DestinationBuffer+i));
		/* handle request */
/////		if ((nwrote = write(sd, DestinationBuffer, 5*4)) < 0) {
			//for(int i=0;i<5*4;i++)nwrote = write(sd, DestinationBuffer, 5*4);
//////			xil_printf("%s: ERROR responding to client echo request. received = %d, written = %d\r\n",
///////					__FUNCTION__, n, nwrote);
////////			xil_printf("Closing socket %d\r\n", sd);
///////			break;
///////		}



	}
//if(flag==0)
//	Initialize_everything();
//xil_printf("source buffer %x  %x  %x  %x\r\n",SourceBuffer[0],SourceBuffer[1],SourceBuffer[2],SourceBuffer[3]);
//xil_printf("destination buffer %x  %x  %x  %x\r\n",DestinationBuffer[0],DestinationBuffer[1],DestinationBuffer[2],DestinationBuffer[3]);




//	check_fifo();//transmit some data





	/* close connection */
	close(sd);
	vTaskDelete(NULL);
}

void echo_application_thread()
{
	int sock, new_sd;
	struct sockaddr_in address, remote;
	int size;

	if ((sock = lwip_socket(AF_INET, SOCK_STREAM, 0)) < 0)
		return;

	address.sin_family = AF_INET;
	address.sin_port = htons(echo_port);
	address.sin_addr.s_addr = INADDR_ANY;

	if (lwip_bind(sock, (struct sockaddr *)&address, sizeof (address)) < 0)
		return;

	lwip_listen(sock, 0);

	size = sizeof(remote);

	while (1) {
		if ((new_sd = lwip_accept(sock, (struct sockaddr *)&remote, (socklen_t *)&size)) > 0) {
			sys_thread_new("echos", process_echo_request,
				(void*)new_sd,
				THREAD_STACKSIZE,
				DEFAULT_THREAD_PRIO);
		}
	}
}