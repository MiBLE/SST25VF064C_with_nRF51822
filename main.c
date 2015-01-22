/* Copyright (c) 2014 Nordic Semiconductor. All Rights Reserved.
 *
 * The information contained herein is property of Nordic Semiconductor ASA.
 * Terms and conditions of usage are described in detail in NORDIC
 * SEMICONDUCTOR STANDARD SOFTWARE LICENSE AGREEMENT.
 *
 * Licensees are granted free, non-transferable use of the information. NO
 * WARRANTY of ANY KIND is provided. This heading must NOT be removed from
 * the file.
 *
 */

/**@file
 * @defgroup spi_master_example_with_slave_main main.c
 * @{
 * @ingroup spi_master_example
 *
 * @brief SPI master example application to be used with the SPI slave example application.
 */

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include "common.h"
#include "app_error.h"
#include "app_util_platform.h"
#include "nrf_delay.h"
#include "nrf_gpio.h"
#include "boards.h"
#include "SST25VF064C.h"
#include <string.h>

/*
 * This example uses only one instance of the SPI master.
 * Please make sure that only one instance of the SPI master is enabled in config file.
 */
uint32_t result;
char  str[13]="";

/**@brief Function for error handling, which is called when an error has occurred. 
 *
 * @param[in] error_code  Error code supplied to the handler.
 * @param[in] line_num    Line number where the handler is called.
 * @param[in] p_file_name Pointer to the file name. 
 */
void app_error_handler(uint32_t error_code, uint32_t line_num, const uint8_t * p_file_name)
{
    for (;;)
    {
        //No implementation needed.
    }
}

/**@brief Function for application main entry. Does not return. */
int main(void)
{
    unsigned char i=0;  
	  //Configure all LEDs as outputs. 
    nrf_gpio_range_cfg_output(LED_START, LED_STOP);
        
    //Set LED_0 high to indicate that the application is running. 
    nrf_gpio_pin_set(LED_0);
    
    //Initialize SPI master.
    SST25VF064C_init();
    
    //Register SPI master event handler.
    //spi_master_evt_handler_reg(SPI_MASTER_HW, spi_master_event_handler);
    
    for (;;)
    {
        if (m_transfer_completed)
        {
          m_transfer_completed = false;
            
					WP_High();
					EWSR();
					WRSR(0x80);
					Chip_Erase_Operation();
          //Set buffers and start data transfer.
					uint8_t text[13] ={0x48,0x65,0x6c,0x6c,0x6f,0x20,0x77,0x6f,0x72,0x6c,0x64,0x21,'\0'};
					for (i=0;i<13;i++)
					{
						upper_128[i]=text[i];
					}
					Page_Program_Operation(0x100000);
					for (i=0;i<128;i++)
					{
						upper_128[i]=0x00;
					}
					Read_Cont(0x100000,13);
					for (i=0;i<13;i++)
					{
						str[i]=upper_128[i];
					}
					if(strcmp(str,"Hello world!"))
					{
						nrf_gpio_pin_set(LED_1);
					}
					nrf_delay_ms(DELAY_MS);
        }
    }
}

/** @} */
