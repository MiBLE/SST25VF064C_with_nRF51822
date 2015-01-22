/*Software Driver

SST25VF064C 64 Mbit(8M x 8) Serial Flash Memory

April 29, 2009, Rev. 1.1

ABOUT THE SOFTWARE
This application note provides software driver examples for SST25VF064C,
Serial Flash. Extensive comments are included in each routine to describe 
the function of each routine.  The interface coding uses polling method 
rather than the SPI protocol to interface with these serial devices.  The
functions are differentiated below in terms of the communication protocols
(uses Mode 0) and specific device operation instructions. This code has been 
designed to compile using the Keil compiler.


ABOUT THE SST25VF064C

Companion product datasheets for the SST25VF064C should be reviewed in 
conjunction with this application note for a complete understanding 
of the device.


Device Communication Protocol(pinout related) functions:

Functions                    		Function
------------------------------------------------------------------
init								Initializes clock to set up mode 0.
Send_Byte						Sends one byte using SI pin to send and shift out 1-bit per clock rising edge
Send_Double_Byte		Sends one byte using SI pin to send and shift out 2-bit per clock rising edge
Get_Byte						Receives one byte using SO pin to receive and shift in 1-bit per clock falling edge
Get_Double_Byte			Receives one byte using SO pin to receive and shift in 2-bit per clock falling edge
CE_High							Sets Chip Enable pin of the serial flash to high
CE_Low							Clears Chip Enable of the serial flash to low
Reset_Hold_Low			Clears Hold pin to make serial flash hold or Reset the device
Reset_Hold_High			Unholds the serial flash
WP_Low							Clears WP pin to make serial flash write protected
WP_High							Disables write protection pin

Note:  The pin names of the SST25VF064C are used in this application note. The associated test code
will not compile unless these pinouts (SCK, SI, SO, CE, WP, Reset_Hold) are pre-defined on your
software which should reflect your hardware interfaced. 	 


Device Operation Instruction functions:

Functions                    		Function
------------------------------------------------------------------
Read_Status_Register			Reads the status register of the serial flash
EWSR											Enables the Write Status Register
WRSR											Performs a write to the status register
WREN											Write enables the serial flash
WRDI											Write disables the serial flash
Read_ID										Reads the manufacturer ID and device ID
Jedec_ID_Read							Reads the Jedec ID
Fast_Read_Dual_IO	 				Reads Memory with Dual Address Input and Data Output
Fast_Read_Dual_Output			Reads Memory with Dual Output
Read											Reads one byte from the serial flash and returns byte(max of 25 MHz CLK frequency)
Read_Cont									Reads multiple bytes(max of 25 MHz CLK frequency)
HighSpeed_Read						Reads one byte from the serial flash and returns byte(max of 66 MHz CLK frequency)
HighSpeed_Read_Cont				Reads multiple bytes(max of 66 MHz CLK frequency)
Page_Program							Programs 1 to 256 data bytes
Dual_Input_Page_Program		Programs 1 to 256 data bytes using dual input
Chip_Erase								Erases entire serial flash
Sector_Erase							Erases one sector (4 KB) of the serial flash
Block_Erase_32K						Erases 32 KByte block memory of the serial flash
Block_Erase_64K						Erases 64 KByte block memory of the serial flash
Wait_Busy									Polls status register until busy bit is low
EHLD											Enable Hold pin functionality */
                                                                     
//"C" LANGUAGE DRIVERS 

/********************************************************************/
/* Copyright Silicon Storage Technology, Inc. (SST), 1994-2009	    */
/* Example "C" language Driver of SST25VF064C Serial Flash	    */
/* Hardik Patel, Silicon Storage Technology, Inc.                   */
/*                                                                  */
/* Revision 1.0, February 11, 2009				    */
/* Revision 1.1, April 29, 2009					    */
/*	- Modified Send_byte, make SI and SO as inputs at the end   */
/*	  of Send_Byte.						    */
/*	- Modified Wait_Busy() function. Sends single RDSR command  */   
/*	  and clocks out until the busy bit becomes low (not busy). */
/*                                                                  */
/*								    */
/********************************************************************/

#ifndef SST25VF064C_H__
#define SST25VF064C_H__


#include <stdio.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include "app_error.h"
#include "app_util_platform.h"
#include "spi_master.h"
#include "nrf_delay.h"
#include "nrf_gpio.h"

#if defined(SPI_MASTER_0_ENABLE)
    #define SPI_MASTER_HW SPI_MASTER_0
#elif defined(SPI_MASTER_1_ENABLE)
    #define SPI_MASTER_HW SPI_MASTER_1
#endif

static volatile bool m_transfer_completed = true; /**< A flag to inform about completed transfer. */

#define TX_RX_BUF_LENGTH    16u     /**< SPI transaction buffer length. */
#define DELAY_MS            100u    /**< Timer delay in milliseconds. */

//Data buffers.
static uint8_t m_tx_data[TX_RX_BUF_LENGTH] = {0}; /**< A buffer with data to transfer. */
static uint8_t m_rx_data[TX_RX_BUF_LENGTH] = {0}; /**< A buffer for incoming data. */

extern unsigned char upper_128[128];	/* global array to store read data */
extern unsigned char security_id_32[32];	/* global array to store security_id data */

/* Function Prototypes */

void SST25VF064C_init(void);
void spi_master_init(spi_master_hw_instance_t spi_master_instance, 
                            spi_master_event_handler_t spi_master_event_handler,
                            const bool lsb);
//void Send_Byte(unsigned char out);
//void Send_Double_Byte(unsigned char out);
//unsigned char Get_Byte();
//unsigned char Get_Double_Byte();
//void CE_High();
//void CE_Low();
void Reset_Hold_Low(void);
void Reset_Hold_High(void);
void WP_Low(void);
void WP_High(void);
unsigned char Read_Status_Register(void);
void EWSR(void);
void WRSR(uint8_t  byte);
void WREN(void);
void WRDI(void);
unsigned long Read_ID(uint8_t ID_addr);
unsigned long Jedec_ID_Read(void); 
unsigned char Read(unsigned long Dst);
void Read_Cont(unsigned long Dst, unsigned long no_bytes);
unsigned char HighSpeed_Read(unsigned long Dst); 
void HighSpeed_Read_Cont(unsigned long Dst, unsigned long no_bytes);
void Chip_Erase(void);
void Sector_Erase(unsigned long Dst);
void Block_Erase_32K(unsigned long Dst);
void Block_Erase_64K(unsigned long Dst);
//void Wait_Busy();
//void Fast_Read_Dual_IO(unsigned long Dst, unsigned long no_bytes);
//void Fast_Read_Dual_Output(unsigned long Dst, unsigned long no_bytes);
void Page_Program(unsigned long Dst);
//void Dual_Input_Page_Program(unsigned long Dst);
void EHLD(void);
void ReadSID(unsigned char Dst, unsigned char security_length); 
void ProgSID(void); 
void LockSID(void); 

//The following functions include WREN() function and Wait_Busy() function
//inside them.

void Block_Erase_32K_Operation(unsigned long Dst);
void Block_Erase_64K_Operation(unsigned long Dst);
void Sector_Erase_Operation(unsigned long Dst);
void Page_Program_Operation(unsigned long Dst);
void Chip_Erase_Operation(void);
#endif
