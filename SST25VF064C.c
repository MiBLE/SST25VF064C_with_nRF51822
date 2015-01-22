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

#include <stdio.h>
#include <stdlib.h>
#include "common.h"
#include "app_error.h"
#include "app_util_platform.h"
#include "spi_master.h"
#include "nrf_delay.h"
#include "nrf_gpio.h"
#include "boards.h"
#include "SST25VF064C.h"

unsigned char upper_128[128];	/* global array to store read data */
unsigned char security_id_32[32];	/* global array to store security_id data */


/**@brief Function for SPI master event callback.
 *
 * Upon receiving an SPI transaction complete event, checks if received data are valid.
 *
 * @param[in] spi_master_evt    SPI master driver event.
 */
static void spi_master_event_handler(spi_master_evt_t spi_master_evt)
{
    switch (spi_master_evt.evt_type)
    {
        case SPI_MASTER_EVT_TRANSFER_COMPLETED:
            
            //Check if data are vaild.
            //result = flash_check(TX_RX_BUF_LENGTH);
            //APP_ERROR_CHECK_BOOL(result);
        
            //Inform application that transfer is completed.
            //m_transfer_completed = true;
            break;
        
        default:
            //No implementation needed.
            break;
    }
}

/**@brief Function for SST25VF064C_ initialization.
 *
 * This initialize SST25VF064C
 *
 * @param[in] spi_master_evt    SPI master driver event.
*/
void SST25VF064C_init(void)
{
	spi_master_init(SPI_MASTER_HW,spi_master_event_handler,false);
}

/************************************************************************/
/* PROCEDURE: init							*/
/*									*/
/* This procedure initializes the SCK to low. Must be called prior to 	*/
/* setting up mode 0.							*/
/*									*/
/* Input:								*/
/*		None							*/
/*									*/
/* Output:								*/
/*		SCK							*/
/************************************************************************/
/**@brief Function for initializing a SPI master driver.
 *
 * @param[in] spi_master_instance       An instance of SPI master module.
 * @param[in] spi_master_event_handler  An event handler for SPI master events.
 * @param[in] lsb                       Bits order LSB if true, MSB if false.
 */
void spi_master_init(spi_master_hw_instance_t spi_master_instance, 
                            spi_master_event_handler_t spi_master_event_handler,
                            const bool lsb)
{
    uint32_t err_code = NRF_SUCCESS;

    //Configure SPI master.
    spi_master_config_t spi_config = SPI_MASTER_INIT_DEFAULT;
    
    switch (spi_master_instance)
    {
        #ifdef SPI_MASTER_0_ENABLE
        case SPI_MASTER_0:
        {
            spi_config.SPI_Pin_SCK = SPIM0_SCK_PIN;
            spi_config.SPI_Pin_MISO = SPIM0_MISO_PIN;
            spi_config.SPI_Pin_MOSI = SPIM0_MOSI_PIN;
            spi_config.SPI_Pin_SS = SPIM0_SS_PIN;
        }
        break; 
        #endif /* SPI_MASTER_0_ENABLE */

        #ifdef SPI_MASTER_1_ENABLE
        case SPI_MASTER_1:
        {
            spi_config.SPI_Pin_SCK = SPIM1_SCK_PIN;
            spi_config.SPI_Pin_MISO = SPIM1_MISO_PIN;
            spi_config.SPI_Pin_MOSI = SPIM1_MOSI_PIN;
            spi_config.SPI_Pin_SS = SPIM1_SS_PIN;
        }
        break;
        #endif /* SPI_MASTER_1_ENABLE */
        
        default:
            break;
    }
    
    spi_config.SPI_CONFIG_ORDER = (lsb ? SPI_CONFIG_ORDER_LsbFirst : SPI_CONFIG_ORDER_MsbFirst);
    
    err_code = spi_master_open(spi_master_instance, &spi_config);
    APP_ERROR_CHECK(err_code);
    
    //Register event handler for SPI master.
    spi_master_evt_handler_reg(spi_master_instance, spi_master_event_handler);
}

/************************************************************************/
/* PROCEDURE: Reset_Hold_Low()						*/
/*									*/
/* This procedure clears the Reset_Hold pin to low.				*/
/*									*/
/* Input:								*/
/*		None							*/
/*									*/
/* Output:								*/
/*		Hold							*/
/************************************************************************/
void Reset_Hold_Low(void)
{
	nrf_gpio_cfg_output(RESET_HOLD_PIN);
	nrf_gpio_pin_clear(RESET_HOLD_PIN);				/* clear Hold pin */
}

/************************************************************************/
/* PROCEDURE: Reset_Hold_High()						*/
/*									*/
/* This procedure sets the Reset_Hold pin to high.				*/
/*									*/
/* Input:								*/
/*		None							*/
/*									*/
/* Output:								*/
/*		Hold							*/
/************************************************************************/
void Reset_Hold_High(void)
{
	nrf_gpio_cfg_output(RESET_HOLD_PIN);
	nrf_gpio_pin_set(RESET_HOLD_PIN);				/* set Hold pin */
}

/************************************************************************/
/* PROCEDURE: WP_Low()							*/
/*									*/
/* This procedure clears the WP pin to low.				*/
/*									*/
/* Input:								*/
/*		None							*/
/*									*/
/* Output:								*/
/*		WP							*/
/************************************************************************/
void WP_Low(void)
{
	nrf_gpio_cfg_output(WRITE_PROTECT_PIN);
	nrf_gpio_pin_clear(WRITE_PROTECT_PIN);					/* clear WP pin */
}

/************************************************************************/
/* PROCEDURE: WP_High()							*/
/*									*/
/* This procedure sets the WP pin to high.				*/
/*									*/
/* Input:								*/
/*		None							*/
/*									*/
/* Output:								*/
/*		WP							*/
/************************************************************************/
void WP_High(void)
{
	nrf_gpio_cfg_output(WRITE_PROTECT_PIN);
	nrf_gpio_pin_set(WRITE_PROTECT_PIN);				/* set WP pin */
}

/************************************************************************/
/* PROCEDURE: Read_Status_Register					*/
/*									*/
/* This procedure read the status register and returns the byte.	*/
/*									*/
/* Input:								*/
/*		None							*/
/*									*/
/* Returns:								*/
/*		byte							*/
/************************************************************************/
unsigned char Read_Status_Register(void)
{
	unsigned char byte = 0;
	uint8_t  p_tx_data[1]={0x05};
	uint8_t  p_rx_data[sizeof(p_tx_data)+1];
	uint32_t err_code = spi_master_send_recv(SPI_MASTER_HW, p_tx_data, sizeof(p_tx_data), p_rx_data,sizeof(p_rx_data));
  APP_ERROR_CHECK(err_code);
	nrf_delay_ms(DELAY_MS);
	//Send_Byte(0x05);			/* send RDSR command */
	byte = p_rx_data[sizeof(p_tx_data)+0];			/* receive byte */
	return byte;
}

/************************************************************************/
/* PROCEDURE: EWSR							*/
/*									*/
/* This procedure Enables Write Status Register.  			*/
/*									*/
/* Input:								*/
/*		None							*/
/*									*/
/* Returns:								*/
/*		Nothing							*/
/************************************************************************/
void EWSR(void)
{
	uint8_t  p_tx_data[1]={0x50};
	uint8_t  p_rx_data[sizeof(p_tx_data)+0];
	uint32_t err_code = spi_master_send_recv(SPI_MASTER_HW, p_tx_data, sizeof(p_tx_data), p_rx_data,sizeof(p_rx_data));
  APP_ERROR_CHECK(err_code);
	nrf_delay_ms(DELAY_MS);
}

/************************************************************************/
/* PROCEDURE: WRSR							*/
/*									*/
/* This procedure writes a byte to the Status Register.			*/
/*									*/
/* Input:								*/
/*		byte							*/
/*									*/
/* Returns:								*/
/*		Nothing							*/
/************************************************************************/
void WRSR(uint8_t  byte)
{
	uint8_t  p_tx_data[2]={0x01,byte};
	uint8_t  p_rx_data[sizeof(p_tx_data)+0];
	uint32_t err_code = spi_master_send_recv(SPI_MASTER_HW, p_tx_data, sizeof(p_tx_data), p_rx_data,sizeof(p_rx_data));
  APP_ERROR_CHECK(err_code);
	nrf_delay_ms(DELAY_MS);
	//Send_Byte(0x01);			/* select write to status register */
	//Send_Byte(byte);			/* data that will change the status of BPx or BPL (only bits 2,3,4,5,7 can be written) */
}

/************************************************************************/
/* PROCEDURE: WREN							*/
/*									*/
/* This procedure enables the Write Enable Latch.  It can also be used 	*/
/* to Enables Write Status Register.					*/
/*									*/
/* Input:								*/
/*		None							*/
/*									*/
/* Returns:								*/
/*		Nothing							*/
/************************************************************************/
void WREN(void)
{
	uint8_t p_tx_data[1]={0x06};
	uint8_t  p_rx_data[sizeof(p_tx_data)+0];
	uint32_t err_code = spi_master_send_recv(SPI_MASTER_HW, p_tx_data, sizeof(p_tx_data), p_rx_data,sizeof(p_rx_data));
	APP_ERROR_CHECK(err_code);
	nrf_delay_ms(DELAY_MS);
	//Send_Byte(0x06);			/* send WREN command */
}

/************************************************************************/
/* PROCEDURE: WRDI							*/
/*									*/
/* This procedure disables the Write Enable Latch.			*/
/*									*/
/* Input:								*/
/*		None							*/
/*									*/
/* Returns:								*/
/*		Nothing							*/
/************************************************************************/
void WRDI(void)
{
	uint8_t p_tx_data[1]={0x04};
	uint8_t  p_rx_data[sizeof(p_tx_data)+0];
	uint32_t err_code = spi_master_send_recv(SPI_MASTER_HW, p_tx_data, sizeof(p_tx_data), p_rx_data,sizeof(p_rx_data));
	APP_ERROR_CHECK(err_code);
	nrf_delay_ms(DELAY_MS);
	//Send_Byte(0x04);			/* send WRDI command */
}


/************************************************************************/
/* PROCEDURE: Read_ID							*/
/*									*/
/* This procedure Reads the manufacturer's ID and device ID.  It will 	*/
/* use 90h or ABh as the command to read the ID (90h in this sample).   */
/* It is up to the user to give the last byte ID_addr to determine      */
/* whether the device outputs manufacturer's ID first, or device ID 	*/
/* first.  Please see the product datasheet for details.  Returns ID in */
/* variable byte.							*/
/*									*/
/* Input:								*/
/*		ID_addr							*/
/*									*/
/* Returns:								*/
/*		byte:	ID1(Manufacture's ID = BFh or Device ID = 4Bh)	*/
/*									*/
/************************************************************************/
unsigned long Read_ID(uint8_t ID_addr)
{
	unsigned long temp=0;
	uint8_t  p_tx_data[4]={0x90,0x00,0x00,ID_addr};
	uint8_t  p_rx_data[sizeof(p_tx_data)+2];
	uint32_t err_code = spi_master_send_recv(SPI_MASTER_HW, p_tx_data, sizeof(p_tx_data), p_rx_data,sizeof(p_rx_data));
	APP_ERROR_CHECK(err_code);
	nrf_delay_ms(DELAY_MS);
//	Send_Byte(0x90);			/* send read ID command (90h or ABh) */
//  Send_Byte(0x00);			/* send address */
//	Send_Byte(0x00);			/* send address */
//	Send_Byte(ID_addr);			/* send address - either 00H or 01H */
	temp = (temp|p_rx_data[sizeof(p_tx_data)+0])<<8;		/* receive byte */
	temp = (temp|p_rx_data[sizeof(p_tx_data)+1]);		/* receive byte */
	return temp;
}

/************************************************************************/
/* PROCEDURE: Jedec_ID_Read						*/
/*									*/
/* This procedure Reads the manufacturer's ID (BFh), memory type (25h)  */
/* and device ID (4Bh).  It will use 9Fh as the JEDEC ID command.    	*/
/* Please see the product datasheet for details.  			*/
/*									*/
/* Input:								*/
/*		None							*/
/*									*/
/* Returns:								*/
/*		IDs_Read:ID1(Manufacture's ID = BFh, Memory Type (25h), */
/*		 and Device ID (4Bh)					*/
/*									*/
/************************************************************************/
unsigned long Jedec_ID_Read(void) 
{
	unsigned long temp=0;
	//Send_Byte(0x9F);		 	/* send JEDEC ID command (9Fh) */
  //temp = (temp | Get_Byte()) << 8; 	/* receive byte */
	//temp = (temp | Get_Byte()) << 8;	
	//temp = (temp | Get_Byte()); 	 	/* temp value = 0xBF254B */
	uint8_t  p_tx_data[1]={0x9F};
	uint8_t  p_rx_data[sizeof(p_tx_data)+3];
	uint32_t err_code = spi_master_send_recv(SPI_MASTER_HW, p_tx_data, sizeof(p_tx_data), p_rx_data,sizeof(p_rx_data));
	APP_ERROR_CHECK(err_code);
	nrf_delay_ms(DELAY_MS);
	temp = (temp | p_rx_data[0+sizeof(p_tx_data)]) << 8; 	/* receive byte */
	temp = (temp | p_rx_data[1+sizeof(p_tx_data)]) << 8;	
	temp = (temp | p_rx_data[2+sizeof(p_tx_data)]); 	 	/* temp value = 0xBF254B */
	return temp;
}

/************************************************************************/
/* PROCEDURE:	Read							*/
/*									*/		
/* This procedure reads one address of the device.  It will return the 	*/
/* byte read in variable byte.						*/
/*									*/
/*									*/
/*									*/
/* Input:								*/
/*		Dst:	Destination Address 000000H - 7FFFFFH		*/
/*      								*/
/*									*/
/* Returns:								*/
/*		byte							*/
/*									*/
/************************************************************************/
unsigned char Read(unsigned long Dst) 
{
	unsigned char byte = 0;	
	uint8_t p_tx_data[4]={0x03,((Dst & 0xFFFFFF) >> 16),((Dst & 0xFFFF) >> 8),(Dst & 0xFF)};
	uint8_t  p_rx_data[sizeof(p_tx_data)+1];
	uint32_t err_code = spi_master_send_recv(SPI_MASTER_HW, p_tx_data, sizeof(p_tx_data), p_rx_data,sizeof(p_rx_data));
	APP_ERROR_CHECK(err_code);
	nrf_delay_ms(DELAY_MS);
	
	//Send_Byte(0x03); 			/* read command */
	//Send_Byte(((Dst & 0xFFFFFF) >> 16));	/* send 3 address bytes */
	//Send_Byte(((Dst & 0xFFFF) >> 8));
	//Send_Byte(Dst & 0xFF);
	byte = p_rx_data[0+sizeof(p_tx_data)];
	return byte;				/* return one byte read */
}

/************************************************************************/
/* PROCEDURE:	Read_Cont						*/
/*									*/		
/* This procedure reads multiple addresses of the device and stores	*/
/* data into 128 byte buffer. Maximum byte that can be read is 128 bytes*/
/*									*/
/* Input:								*/
/*		Dst:		Destination Address 000000H - 7FFFFFH	*/
/*      	no_bytes	Number of bytes to read	(max = 128)	*/
/*									*/
/* Returns:								*/
/*		Nothing							*/
/*									*/
/************************************************************************/
void Read_Cont(unsigned long Dst, unsigned long no_bytes)
{
	unsigned long i = 0;
	uint8_t p_tx_data[4]={0x03,((Dst & 0xFFFFFF) >> 16),((Dst & 0xFFFF) >> 8),(Dst & 0xFF)};
	uint8_t  p_rx_data[sizeof(p_tx_data)+no_bytes];
	uint32_t err_code = spi_master_send_recv(SPI_MASTER_HW, p_tx_data, sizeof(p_tx_data), p_rx_data,sizeof(p_rx_data));
	APP_ERROR_CHECK(err_code);
	nrf_delay_ms(DELAY_MS);
	//Send_Byte(0x03); 			/* read command */
	//Send_Byte(((Dst & 0xFFFFFF) >> 16)); 	/* send 3 address bytes */
	//Send_Byte(((Dst & 0xFFFF) >> 8));
	//Send_Byte(Dst & 0xFF);
	for (i = 0; i < no_bytes; i++)		/* read until no_bytes is reached */
	{
		upper_128[i] = p_rx_data[i+sizeof(p_tx_data)];	/* receive bytes */
	}
}

/************************************************************************/
/* PROCEDURE:	HighSpeed_Read						*/
/*									*/		
/* This procedure reads one address of the device.  It will return the 	*/
/* byte read in variable byte.						*/
/*									*/
/*									*/
/*									*/
/* Input:								*/
/*		Dst:	Destination Address 000000H - 7FFFFFH		*/
/*      								*/
/*									*/
/* Returns:								*/
/*		byte							*/
/*									*/
/************************************************************************/
unsigned char HighSpeed_Read(unsigned long Dst) 
{
	unsigned char byte = 0;	
	uint8_t p_tx_data[5]={0x0B,((Dst & 0xFFFFFF) >> 16),((Dst & 0xFFFF) >> 8),(Dst & 0xFF),0xFF};
	uint8_t  p_rx_data[sizeof(p_tx_data)+1];
	uint32_t err_code = spi_master_send_recv(SPI_MASTER_HW, p_tx_data, sizeof(p_tx_data), p_rx_data,sizeof(p_rx_data));
	APP_ERROR_CHECK(err_code);
	nrf_delay_ms(DELAY_MS);
	//Send_Byte(0x0B); 			/* read command */
	//Send_Byte(((Dst & 0xFFFFFF) >> 16));	/* send 3 address bytes */
	//Send_Byte(((Dst & 0xFFFF) >> 8));
	//Send_Byte(Dst & 0xFF);
	//Send_Byte(0xFF);			/*dummy byte*/
	byte = p_rx_data[0+sizeof(p_tx_data)];
	return byte;				/* return one byte read */
}

/************************************************************************/
/* PROCEDURE:	HighSpeed_Read_Cont					*/
/*									*/		
/* This procedure reads multiple addresses of the device and stores	*/
/* data into 128 byte buffer. Maximum byte that can be read is 128 bytes*/
/*									*/
/* Input:								*/
/*		Dst:		Destination Address 000000H - 7FFFFFH	*/
/*      	no_bytes	Number of bytes to read	(max = 128)	*/
/*									*/
/* Returns:								*/
/*		Nothing							*/
/*									*/
/************************************************************************/
void HighSpeed_Read_Cont(unsigned long Dst, unsigned long no_bytes)
{
	unsigned long i = 0;
	uint8_t  p_tx_data[5]={0x0B,((Dst & 0xFFFFFF) >> 16),((Dst & 0xFFFF) >> 8),(Dst & 0xFF),0xFF};
	uint8_t  p_rx_data[sizeof(p_tx_data)+no_bytes];
	uint32_t err_code = spi_master_send_recv(SPI_MASTER_HW, p_tx_data, sizeof(p_tx_data), p_rx_data,sizeof(p_rx_data));
	APP_ERROR_CHECK(err_code);
	nrf_delay_ms(DELAY_MS);
	//Send_Byte(0x0B); 			/* read command */
	//Send_Byte(((Dst & 0xFFFFFF) >> 16)); 	/* send 3 address bytes */
	//Send_Byte(((Dst & 0xFFFF) >> 8));
	//Send_Byte(Dst & 0xFF);
	//Send_Byte(0xFF);			/*dummy byte*/
	for (i = 0; i < no_bytes; i++)		/* read until no_bytes is reached */
	{
		upper_128[i] = p_rx_data[i+sizeof(p_tx_data)];	/* receive byte and store at address 80H - FFH */
	}
}


									

/************************************************************************/
/* PROCEDURE:	Fast_Read_Dual_IO					*/
/*									*/		
/* This procedure reads multiple addresses of the device and stores	*/
/* data into 128 byte buffer. Maximum byte that can be read is 128 bytes*/
/*									*/
/* Input:								*/
/*		Dst:		Destination Address 000000H - 7FFFFFH	*/
/*      	no_bytes	Number of bytes to read	(max = 128)	*/
/*									*/
/* Returns:								*/
/*		Nothing							*/
/*									*/
/************************************************************************/

//void Fast_Read_Dual_IO(unsigned long Dst, unsigned long no_bytes)
//{ 
// 	unsigned long i = 0;
//	CE_Low();					/* enable device */
//	Send_Byte(0xbb); 				/* read command */
//	Send_Double_Byte(((Dst & 0xFFFFFF) >> 16)); 	/* send 3 address bytes */
//	Send_Double_Byte(((Dst & 0xFFFF) >> 8));
//	Send_Double_Byte(Dst & 0xFF);
//	Send_Double_Byte(Dst & 0xFF);  //Dummy cycle
// 	for (i = 0; i < no_bytes; i++)			/* read until no_bytes is reached */
//	{
//		upper_128[i] = Get_Double_Byte();	/* receive byte and store at address 80H - FFH */
//	}
//	CE_High();					/* disable device */
//}

									

/************************************************************************/
/* PROCEDURE:	Fast_Read_Dual_Output					*/
/*									*/		
/* This procedure reads multiple addresses of the device and stores	*/
/* data into 128 byte buffer. Maximum byte that can be read is 128 bytes*/
/*									*/
/* Input:								*/
/*		Dst:		Destination Address 000000H - 7FFFFFH	*/
/*      	no_bytes	Number of bytes to read	(max = 128)	*/
/*									*/
/* Returns:								*/
/*		Nothing							*/
/*									*/
/************************************************************************/

//void Fast_Read_Dual_Output(unsigned long Dst, unsigned long no_bytes)
//{ 
// 	unsigned long i = 0;
//	CE_Low();					/* enable device */
//	Send_Byte(0x3b); 				/* read command */
//	Send_Byte(((Dst & 0xFFFFFF) >> 16)); 		/* send 3 address bytes */
//	Send_Byte(((Dst & 0xFFFF) >> 8));
//	Send_Byte(Dst & 0xFF);
//	Send_Byte(Dst & 0xFF);  //Dummy cycle
// 	for (i = 0; i < no_bytes; i++)			/* read until no_bytes is reached */
//	{
//		upper_128[i] = Get_Double_Byte();	/* receive byte and store at address 80H - FFH */
//	}
//	CE_High();					/* disable device */
//}	


/************************************************************************/
/* PROCEDURE: Chip_Erase						*/
/*									*/
/* This procedure erases the entire Chip.				*/
/*									*/
/* Input:								*/
/*		None							*/
/*									*/
/* Returns:								*/
/*		Nothing							*/
/************************************************************************/
void Chip_Erase(void)
{						
	uint8_t  p_tx_data[1]={0x60};
	uint8_t  p_rx_data[sizeof(p_tx_data)+0];
	uint32_t err_code = spi_master_send_recv(SPI_MASTER_HW, p_tx_data, sizeof(p_tx_data), p_rx_data,sizeof(p_rx_data));
	APP_ERROR_CHECK(err_code);
	nrf_delay_ms(DELAY_MS);
	//Send_Byte(0x60);			/* send Chip Erase command (60h or C7h) */
}




/************************************************************************/
/* PROCEDURE: Sector_Erase						*/
/*									*/
/* This procedure Sector Erases the Chip.				*/
/*									*/
/* Input:								*/
/*		Dst:		Destination Address 000000H - 7FFFFFH	*/
/*									*/
/* Returns:								*/
/*		Nothing							*/
/************************************************************************/
void Sector_Erase(unsigned long Dst)
{
	uint8_t p_tx_data[4]={0x20,((Dst & 0xFFFFFF) >> 16),((Dst & 0xFFFF) >> 8),(Dst & 0xFF)};
	uint8_t  p_rx_data[sizeof(p_tx_data)+0];
	uint32_t err_code = spi_master_send_recv(SPI_MASTER_HW, p_tx_data, sizeof(p_tx_data), p_rx_data,sizeof(p_rx_data));
	APP_ERROR_CHECK(err_code);
	nrf_delay_ms(DELAY_MS);
	//Send_Byte(0x20);			/* send Sector Erase command */
	//Send_Byte(((Dst & 0xFFFFFF) >> 16)); 	/* send 3 address bytes */
	//Send_Byte(((Dst & 0xFFFF) >> 8));
	//Send_Byte(Dst & 0xFF);
}

/************************************************************************/
/* PROCEDURE: Block_Erase_32K						*/
/*									*/
/* This procedure Block Erases 32 KByte of the Chip.			*/
/*									*/
/* Input:								*/
/*		Dst:		Destination Address 000000H - 7FFFFFH	*/
/*									*/
/* Returns:								*/
/*		Nothing							*/
/************************************************************************/
void Block_Erase_32K(unsigned long Dst)
{
	uint8_t p_tx_data[4]={0x52,((Dst & 0xFFFFFF) >> 16),((Dst & 0xFFFF) >> 8),(Dst & 0xFF)};
	uint8_t  p_rx_data[sizeof(p_tx_data)+0];
	uint32_t err_code = spi_master_send_recv(SPI_MASTER_HW, p_tx_data, sizeof(p_tx_data), p_rx_data,sizeof(p_rx_data));
	APP_ERROR_CHECK(err_code);
	nrf_delay_ms(DELAY_MS);
	//Send_Byte(0x52);			/* send 32 KByte Block Erase command */
	//Send_Byte(((Dst & 0xFFFFFF) >> 16)); 	/* send 3 address bytes */
	//Send_Byte(((Dst & 0xFFFF) >> 8));
	//Send_Byte(Dst & 0xFF);
}

/************************************************************************/
/* PROCEDURE: Block_Erase_64K						*/
/*									*/
/* This procedure Block Erases 64 KByte of the Chip.			*/
/*									*/
/* Input:								*/
/*		Dst:		Destination Address 000000H - 7FFFFFH	*/
/*									*/
/* Returns:								*/
/*		Nothing							*/
/************************************************************************/
void Block_Erase_64K(unsigned long Dst)
{
	uint8_t p_tx_data[4]={0xD8,((Dst & 0xFFFFFF) >> 16),((Dst & 0xFFFF) >> 8),(Dst & 0xFF)};
	uint8_t  p_rx_data[sizeof(p_tx_data)+0];
	uint32_t err_code = spi_master_send_recv(SPI_MASTER_HW, p_tx_data, sizeof(p_tx_data), p_rx_data,sizeof(p_rx_data));
	APP_ERROR_CHECK(err_code);
	nrf_delay_ms(DELAY_MS);
	//Send_Byte(0xD8);			/* send 64KByte Block Erase command */
	//Send_Byte(((Dst & 0xFFFFFF) >> 16)); 	/* send 3 address bytes */
	//Send_Byte(((Dst & 0xFFFF) >> 8));
	//Send_Byte(Dst & 0xFF);
}

/************************************************************************/
/* PROCEDURE: Wait_Busy							*/
/*									*/
/* This procedure waits until device is no longer busy (can be used by	*/
/* Page-Program, Sector-Erase, Block-Erase, Chip-Erase).		*/
/*									*/
/* Input:								*/
/*		None							*/
/*									*/
/* Returns:								*/
/*		Nothing							*/
/************************************************************************/
 				
//  void Wait_Busy()
//{


//		//unsigned char temp = 0;

//		 //CE=0;
//		uint8_t * const p_rx_data;
//    //const uint16_t len;
//	  uint32_t err_code = spi_master_send_recv(SPI_MASTER_HW, 0x05, 1, p_rx_data, 0);
//	  	//Send_Byte(0x05);		/* send RDSR command */

////		 SCK=1;
////		 SCK=0;
////		 SCK=1;
////		 SCK=0;
////		 SCK=1;
////		 SCK=0;
////		 SCK=1;
////		 SCK=0;
////		 SCK=1;
////		 SCK=0;
////		 SCK=1;
////		 SCK=0;
////		 SCK=1;
////		 SCK=0;
////		 temp=SO;		//Get the Value of Busy bit in the status register
////		 temp=SO;		//Get the Value of Busy bit in the status register
////		 temp=SO;		//Get the Value of Busy bit in the status register
////		 temp=SO;		//Get the Value of Busy bit in the status register
////		 temp=SO;		//Get the Value of Busy bit in the status register

//		 while (temp==1)
//		 {
//		 SCK=1;
//		 SCK=0;
//		 SCK=1;
//		 SCK=0;
//		 SCK=1;
//		 SCK=0;
//		 SCK=1;
//		 SCK=0;
//		 SCK=1;
//		 SCK=0;
//		 SCK=1;
//		 SCK=0;
//		 SCK=1;
//		 SCK=0;
//		 SCK=1;
//		 SCK=0;
//		 temp=SO;		//Get the Value of Busy bit in the status register
//		 temp=SO;		//Get the Value of Busy bit in the status register
//		 temp=SO;		//Get the Value of Busy bit in the status register
//		 temp=SO;		//Get the Value of Busy bit in the status register
//		 temp=SO;		//Get the Value of Busy bit in the status register

//		 }
//		 CE=1;

//}	  

/************************************************************************/
/* PROCEDURE: EHLD			  				*/
/* 									*/
/* Enables Hold pin functionality					*/
/*									*/
/* Input:								*/
/*		None							*/
/* Returns:								*/
/*		Nothing							*/
/************************************************************************/

void EHLD(void)
{
	uint8_t  p_tx_data[1]={0xaa};
	uint8_t  p_rx_data[sizeof(p_tx_data)+0];
	uint32_t err_code = spi_master_send_recv(SPI_MASTER_HW, p_tx_data, sizeof(p_tx_data), p_rx_data,sizeof(p_rx_data));
	APP_ERROR_CHECK(err_code);
	nrf_delay_ms(DELAY_MS);
	//Send_Byte(0xaa);			
}


/************************************************************************/
/* PROCEDURE:	Page_Program						*/
/*									*/
/* This procedure does page programming.  The destination		*/
/* address should be provided.						*/
/* The data array of 128 bytes contains the data to be programmed.  	*/
/* Since the size of the data array is 128 bytes rather than 256 bytes, */
/* this page program procedure programs 128 bytes			*/
/*									*/
/*									*/
/* Assumption:  Address being programmed is already erased and is NOT	*/
/*		block protected.					*/
/*									*/
/*									*/
/*									*/
/* Input:								*/
/*		Dst:		Destination Address 000000H - 7FFFFFH	*/
/*		upper_128[128] contains 128 bytes of data to program.   */
/*      								*/
/*									*/
/* Returns:								*/
/*		Nothing							*/
/*									*/
/************************************************************************/
void Page_Program(unsigned long Dst)
{
	unsigned char i=0;
	uint8_t  p_tx_data[4+128]={0x02,((Dst & 0xFFFFFF) >> 16),((Dst & 0xFFFF) >> 8),(Dst & 0xFF),};
	for (i=0;i<=127;i++)
	{	
		p_tx_data[i+4]=upper_128[i];
	}
	uint8_t  p_rx_data[sizeof(p_tx_data)+0];
	uint32_t err_code = spi_master_send_recv(SPI_MASTER_HW, p_tx_data, sizeof(p_tx_data), p_rx_data,sizeof(p_rx_data));
	APP_ERROR_CHECK(err_code);
	nrf_delay_ms(DELAY_MS);
	//Send_Byte(0x02); 			/* send Byte Program command */
	//Send_Byte(((Dst & 0xFFFFFF) >> 16));	/* send 3 address bytes */
	//Send_Byte(((Dst & 0xFFFF) >> 8));
	//Send_Byte(Dst & 0xFF);
	//Send_Byte(upper_128[i]);	/* send byte to be programmed */
}



/************************************************************************/
/* PROCEDURE:	Dual_Input_Page_Program					*/
/*									*/
/* This procedure does page programming.  The destination		*/
/* address should be provided.						*/
/* The data array of 128 bytes contains the data to be programmed.  	*/
/* Since the size of the data array is 128 bytes rather than 256 bytes, */
/* this page program procedure programs 128 bytes			*/
/*									*/
/*									*/
/* Assumption:  Address being programmed is already erased and is NOT	*/
/*		block protected.					*/
/*									*/
/*									*/
/*									*/
/* Input:								*/
/*		Dst:		Destination Address 000000H - 7FFFFFH	*/
/*		upper_128[128] contains 128 bytes of data to program.   */
/*      								*/
/*									*/
/* Returns:								*/
/*		Nothing							*/
/*									*/
/************************************************************************/

//void Dual_Input_Page_Program(unsigned long Dst)
//{
//	unsigned char i;
//	i=0;	
//  
//	CE_Low();				/* enable device */
//	Send_Byte(0xa2); 			/* send Byte Program command */
//	Send_Byte(((Dst & 0xFFFFFF) >> 16));	/* send 3 address bytes */
//	Send_Byte(((Dst & 0xFFFF) >> 8));
//	Send_Byte(Dst & 0xFF);
//	for (i=0;i<=127;i++)
//	{	Send_Double_Byte(upper_128[i]);	/* send byte to be programmed */
//	}
//	CE_High();				/* disable device */
//}

/************************************************************************/
/* PROCEDURE:	ReadSID	(Read Security ID)				*/
/*									*/		
/* This procedure reads the security ID					*/
/*									*/
/*									*/
/*									*/
/* Input:								*/
/*		Security Id destination address, Security Id length	*/
/*      								*/
/*									*/
/* Returns:								*/
/*		None							*/
/*									*/
/************************************************************************/
void ReadSID(unsigned char Dst, unsigned char security_length) 
{
	unsigned char i=0;	
	if (security_length>32)
	{ 
		security_length=32;
	}
	
	uint8_t p_tx_data[3]={0x88,(Dst & 0xFF),(Dst & 0xFF)};
	uint8_t  p_rx_data[sizeof(p_tx_data)+security_length+Dst];
	uint32_t err_code = spi_master_send_recv(SPI_MASTER_HW, p_tx_data, sizeof(p_tx_data), p_rx_data,sizeof(p_rx_data));
	APP_ERROR_CHECK(err_code);
	nrf_delay_ms(DELAY_MS);
	
	for (i=Dst;i<(security_length+Dst);i++)
	{ 
		if (i<32)
		{
			security_id_32[i] = p_rx_data[i+sizeof(p_tx_data)]; 
		}
		else
		{
			security_id_32[i-32] = p_rx_data[i+sizeof(p_tx_data)]; 
		}
	}	
	//Send_Byte(0x88); 		
	//Send_Byte(Dst & 0xFF);
	//Send_Byte(Dst & 0xFF);  //dummy
}

/************************************************************************/
/* PROCEDURE:	ProgSID	(Program Security ID)				*/
/*									*/		
/* This procedure programs the security ID				*/
/*									*/
/*									*/
/*									*/
/* Input:								*/
/*	Security is stored in the security_id_32[] array		*/
/*									*/
/* Returns:								*/
/*	None								*/
/*									*/
/************************************************************************/
void ProgSID(void) 
{
	unsigned char i=0;
	uint8_t p_tx_data[2+24]={0xa5,0x08,};
	for (i=0;i<24;i++)
	{	
		p_tx_data[2+i]=security_id_32[i+8];
	}
	uint8_t  p_rx_data[sizeof(p_tx_data)+0];
	uint32_t err_code = spi_master_send_recv(SPI_MASTER_HW, p_tx_data, sizeof(p_tx_data), p_rx_data,sizeof(p_rx_data));
	APP_ERROR_CHECK(err_code);
	nrf_delay_ms(DELAY_MS);
	//Send_Byte(0xa5); 		
	//Send_Byte(0x08);	  		/*address of user programmable area*/
	//for (i=0;i<24;i++)
	//{	
	//Send_Byte(security_id_32[i+8]);
	//}
}

/************************************************************************/
/* PROCEDURE:	LockSID							*/
/*									*/		
/* This procedure Locks the security ID setting				*/
/*									*/
/*									*/
/*									*/
/* Input:								*/
/*		None							*/
/*      								*/
/*									*/
/* Returns:								*/
/*		None							*/
/*									*/
/************************************************************************/
void LockSID(void) 
{
	uint8_t p_tx_data[1]={0x85};
	uint8_t  p_rx_data[sizeof(p_tx_data)+0];
	uint32_t err_code = spi_master_send_recv(SPI_MASTER_HW, p_tx_data, sizeof(p_tx_data), p_rx_data,sizeof(p_rx_data));
	APP_ERROR_CHECK(err_code);
	nrf_delay_ms(DELAY_MS);
	//Send_Byte(0x85); 		
}

/************************************************************************/
/* PROCEDURE: Block_Erase_32K_Operation					*/
/*									*/
/* This procedure Block Erases 32 KByte of the Chip. WREN() and 	*/
/* Wait_Busy() are included in this procedure.				*/
/*									*/
/* Input:								*/
/*		Dst:	Destination Address 000000H - 7FFFFFH		*/
/*									*/
/* Returns:								*/
/*		Nothing							*/
/************************************************************************/ 
void Block_Erase_32K_Operation(unsigned long Dst)
{  	
	//WREN
	WREN();
	//Send_Byte(0x06);				/* send WREN command */
	
	//Block Erase 32K
	uint8_t p_tx_data[4]={0x52,((Dst & 0xFFFFFF) >> 16),((Dst & 0xFFFF) >> 8),(Dst & 0xFF)};
	uint8_t  p_rx_data[sizeof(p_tx_data)+0];
	uint32_t err_code = spi_master_send_recv(SPI_MASTER_HW, p_tx_data, sizeof(p_tx_data), p_rx_data,sizeof(p_rx_data));
	APP_ERROR_CHECK(err_code);
	nrf_delay_ms(DELAY_MS);
	
	//Send_Byte(0x52);				/* send 32 KByte Block Erase command */
	//Send_Byte(((Dst & 0xFFFFFF) >> 16)); 		/* send 3 address bytes */
	//Send_Byte(((Dst & 0xFFFF) >> 8));
	//Send_Byte(Dst & 0xFF);

	//Wait Busy
	while ((Read_Status_Register()& 0x01) == 0x01)	/* waste time until not busy */
	Read_Status_Register();
}

/************************************************************************/
/* PROCEDURE: Block_Erase_64K_Operation					*/
/*									*/
/* This procedure Block Erases 64 KByte of the Chip. WREN() and 	*/
/* Wait_Busy() are included in this procedure.				*/		
/*									*/
/* Input:								*/
/*		Dst:	Destination Address 000000H - 7FFFFFH		*/
/*									*/
/* Returns:								*/
/*		Nothing							*/
/************************************************************************/
void Block_Erase_64K_Operation(unsigned long Dst)
{  	
	//WREN
	WREN();
	//Send_Byte(0x06);				/* send WREN command */

	//Block Erase 32K
	uint8_t p_tx_data[4]={0xD8,((Dst & 0xFFFFFF) >> 16),((Dst & 0xFFFF) >> 8),(Dst & 0xFF)};
	uint8_t  p_rx_data[sizeof(p_tx_data)+0];
	uint32_t err_code = spi_master_send_recv(SPI_MASTER_HW, p_tx_data, sizeof(p_tx_data), p_rx_data,sizeof(p_rx_data));
	APP_ERROR_CHECK(err_code);
	nrf_delay_ms(DELAY_MS);
	//Send_Byte(0xD8);				/* send 64KByte Block Erase command */
	//Send_Byte(((Dst & 0xFFFFFF) >> 16)); 		/* send 3 address bytes */
	//Send_Byte(((Dst & 0xFFFF) >> 8));
	//Send_Byte(Dst & 0xFF);

	//Wait Busy
	while ((Read_Status_Register()& 0x01) == 0x01)	/* waste time until not busy */
	Read_Status_Register();
}


/************************************************************************/
/* PROCEDURE: Sector_Erase_Operation					*/
/*									*/
/* This procedure Sector Erases the Chip. WREN() and 			*/
/* Wait_Busy() are included in this procedure.				*/
/*									*/
/* Input:								*/
/*		Dst:	Destination Address 000000H - 7FFFFFH		*/
/*									*/
/* Returns:								*/
/*		Nothing							*/
/************************************************************************/
void Sector_Erase_Operation(unsigned long Dst)
{
	//WREN
	WREN();
	//Send_Byte(0x06);				/* send WREN command */

	//Sector Erase
	uint8_t p_tx_data[4]={0x20,((Dst & 0xFFFFFF) >> 16),((Dst & 0xFFFF) >> 8),(Dst & 0xFF)};
	uint8_t  p_rx_data[sizeof(p_tx_data)+0];
	uint32_t err_code = spi_master_send_recv(SPI_MASTER_HW, p_tx_data, sizeof(p_tx_data), p_rx_data,sizeof(p_rx_data));
	APP_ERROR_CHECK(err_code);
	nrf_delay_ms(DELAY_MS);
	//Send_Byte(0x20);				/* send Sector Erase command */
	//Send_Byte(((Dst & 0xFFFFFF) >> 16)); 		/* send 3 address bytes */
	//Send_Byte(((Dst & 0xFFFF) >> 8));
	//Send_Byte(Dst & 0xFF);

	//Wait Busy
	while ((Read_Status_Register()& 0x01) == 0x01)	/* waste time until not busy */
	Read_Status_Register();
}


/************************************************************************/
/* PROCEDURE:	Page_Program Operation					*/
/*									*/
/* This procedure does page programming.  The destination		*/
/* address should be provided.						*/
/* The data array of 128 bytes contains the data to be programmed.  	*/
/* Since the size of the data array is 128 bytes rather than 256 bytes, */
/* this page program procedure programs 128 bytes. WREN() and 		*/
/* Wait_Busy() are included in this procedure.				*/
/*									*/
/*									*/
/* Assumption:  Address being programmed is already erased and is NOT	*/
/*		block protected.					*/
/*									*/
/*									*/
/*									*/
/* Input:								*/
/*		Dst:	Destination Address 000000H - 7FFFFFH		*/
/*		upper_128[128] contains 128 bytes of data to program.   */
/*      								*/
/*									*/
/* Returns:								*/
/*		Nothing							*/
/*									*/
/************************************************************************/
void Page_Program_Operation(unsigned long Dst)
{
	unsigned char i=0;
	//WREN
	WREN();
	//Send_Byte(0x06);				/* send WREN command */

	//Page Program
	uint8_t  p_tx_data[4+128]={0x02,((Dst & 0xFFFFFF) >> 16),((Dst & 0xFFFF) >> 8),(Dst & 0xFF),};
	for (i=0;i<=127;i++)
	{	
		p_tx_data[i+4]=upper_128[i];
	}
	uint8_t  p_rx_data[sizeof(p_tx_data)+0];
	uint32_t err_code = spi_master_send_recv(SPI_MASTER_HW, p_tx_data, sizeof(p_tx_data), p_rx_data,sizeof(p_rx_data));
	APP_ERROR_CHECK(err_code);
	nrf_delay_ms(DELAY_MS);
	//Send_Byte(0x02); 				/* send Byte Program command */
	//Send_Byte(((Dst & 0xFFFFFF) >> 16));		/* send 3 address bytes */
	//Send_Byte(((Dst & 0xFFFF) >> 8));
	//Send_Byte(Dst & 0xFF);
	//for (i=0;i<=127;i++)
	//{	
		//Send_Byte(upper_128[i]);		/* send byte to be programmed */
	//}

	//Wait Busy
	while ((Read_Status_Register()& 0x01) == 0x01)	/* waste time until not busy */
	Read_Status_Register();
}

/************************************************************************/
/* PROCEDURE: Chip_Erase_Operation					*/
/*									*/
/* This procedure erases the entire Chip. WREN() and 			*/
/* Wait_Busy() are included in this procedure.				*/
/*									*/
/* Input:								*/
/*		None							*/
/*									*/
/* Returns:								*/
/*		Nothing							*/
/************************************************************************/
void Chip_Erase_Operation(void)
{	
	 //WREN
	WREN();
	//Send_Byte(0x06);				/* send WREN command */
	
	//Chip Erase 					
	uint8_t  p_tx_data[1]={0x60};
	uint8_t  p_rx_data[sizeof(p_tx_data)+0];
	uint32_t err_code = spi_master_send_recv(SPI_MASTER_HW, p_tx_data, sizeof(p_tx_data), p_rx_data,sizeof(p_rx_data));
	APP_ERROR_CHECK(err_code);
	nrf_delay_ms(DELAY_MS);
	//Send_Byte(0x60);				/* send Chip Erase command (60h or C7h) */

	//Wait Busy
	while ((Read_Status_Register()& 0x01) == 0x01)	/* waste time until not busy */
	Read_Status_Register();
}
