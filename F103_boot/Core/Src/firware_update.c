/*
 * firware_update.c
 *
 *  Created on: 13-Jun-2026
 *      Author: Admin
 */


#include "main.h"
extern UART_HandleTypeDef huart2;
#define APP_MAX_SIZE      48128U         // Maximum application size (47 KB)
BL_Status_t Bootloader_validation_header(const app_header_t* header);

BL_Status_t Bootloader_WriteChunk(uint32_t flash_address,
                                  uint8_t *buffer,
                                  uint32_t length);

BL_Status_t  Bootloader_EraseApplication(void);
BL_Status_t Bootloader_ReceiveFirmware(const app_header_t *header);


void Bootloader_UpdateMode(void)
{
    uint8_t rx_buffer[5];
    app_header_t rx_header;
    char tx_msg[100];

    HAL_UART_Transmit(&huart2,
                      (uint8_t *)"Waiting for PC...\r\n",
                      strlen("Waiting for PC...\r\n"),
                      100);

    while (1)
    {
        /* Wait for HELLO */
        if (HAL_UART_Receive(&huart2,
                             rx_buffer,
                             5,
                             HAL_MAX_DELAY) == HAL_OK)
        {
            if (memcmp(rx_buffer, "HELLO", 5U) == 0)
            {
                HAL_UART_Transmit(&huart2,
                                  (uint8_t *)"ACK\r\n",
                                  strlen("ACK\r\n"),
                                  100);

                /* Receive application header */
                if (HAL_UART_Receive(&huart2,
                                     (uint8_t *)&rx_header,
                                     sizeof(app_header_t),
                                     HAL_MAX_DELAY) == HAL_OK)
                {
                    HAL_UART_Transmit(&huart2,
                                      (uint8_t *)"HEADER RECEIVED\r\n",
                                      strlen("HEADER RECEIVED\r\n"),
                                      100);

                    sprintf(tx_msg,
                            "Magic=%08lX Size=%lu CRC=%08lX Ver=%lu\r\n",
                            (unsigned long)rx_header.magic_key,
                            (unsigned long)rx_header.size,
                            (unsigned long)rx_header.crc,
                            (unsigned long)rx_header.version);

                    HAL_UART_Transmit(&huart2,
                                      (uint8_t *)tx_msg,
                                      strlen(tx_msg),
                                      100);

                    /* Validate Header */
                    if (Bootloader_validation_header(&rx_header) != BL_OK)
                    {
                        /* Invalid header */
                        continue;
                    }

                    HAL_UART_Transmit(&huart2,
                                      (uint8_t *)"READY FOR FLASH ERASE\r\n",
                                      strlen("READY FOR FLASH ERASE\r\n"),
                                      100);

                    /* Erase application Flash */
                    if (Bootloader_EraseApplication() != BL_OK)
                    {
                        continue;
                    }

                    /* Receive firmware and write to Flash */
                    if (Bootloader_ReceiveFirmware(&rx_header) != BL_OK)
                    {
                        continue;
                    }

                    /* Firmware transfer completed */
                    HAL_UART_Transmit(&huart2,
                                      (uint8_t *)"UPDATE COMPLETE\r\n",
                                      strlen("UPDATE COMPLETE\r\n"),
                                      100);
                }
            }
        }
    }
}


/*
 * This function is to validate that , the recived header is right or wrong  */
BL_Status_t Bootloader_validation_header(const app_header_t* header)
{
	//check the magic key
	if(header->magic_key!=APP_MAGIC)
	{
		  HAL_UART_Transmit(&huart2,
		                          (uint8_t *)"HEADER_FAIL : INVALID MAGIC\r\n",
		                          strlen("HEADER_FAIL : INVALID MAGIC\r\n"),
		                          100);
		  return BL_ERROR;
	}

	// check application size
	if(header->size==0U)
	{
		  HAL_UART_Transmit(&huart2,
		                          (uint8_t *)"HEADER_FAIL : INVALID SIZE\r\n",
		                          strlen("HEADER_FAIL : INVALID SIZE\r\n"),
		                          100);

		        return BL_ERROR;
	}

	if(header->size > APP_MAX_SIZE)
	{
		HAL_UART_Transmit(&huart2,
	                          (uint8_t *)"HEADER_FAIL : APP TOO LARGE\r\n",
	                          strlen("HEADER_FAIL : APP TOO LARGE\r\n"),
	                          100);

	        return BL_ERROR;
	    }
	  /* Header is valid */
	    HAL_UART_Transmit(&huart2,
	                      (uint8_t *)"HEADER_OK\r\n",
	                      strlen("HEADER_OK\r\n"),
	                      100);

	    return BL_OK;

}




/*this fucntion is to erase the flash memory before the  new firmware is return to this*/

BL_Status_t  Bootloader_EraseApplication(void)
{
	FLASH_EraseInitTypeDef EraseInit;
	uint32_t PageError=0;

	HAL_FLASH_Unlock();

	EraseInit.TypeErase=FLASH_TYPEERASE_PAGES;
	EraseInit.PageAddress=APP_START_ADDR;
	EraseInit.NbPages=47;


	if(HAL_FLASHEx_Erase(&EraseInit,&PageError)!=HAL_OK)
	{
		HAL_FLASH_Lock();
		HAL_UART_Transmit(&huart2,
						 (uint8_t *)"FLASH ERASE FAILED\r\n",
						 strlen("FLASH ERASE FAILED\r\n"),
						 100);

		return BL_ERROR;
	}


	HAL_FLASH_Lock();
    HAL_UART_Transmit(&huart2,
                      (uint8_t *)"FLASH ERASE OK\r\n",
                      strlen("FLASH ERASE OK\r\n"),
                      100);

    return BL_OK;


}

#define FW_PACKET_SIZE 256
BL_Status_t Bootloader_ReceiveFirmware(const app_header_t *header)
{
    uint8_t firmware_buffer[FW_PACKET_SIZE];

    uint32_t remaining = header->size;
    uint32_t chunk_size;
    uint32_t flash_address = APP_START_ADDR;

    while (remaining > 0)
    {
        /* Decide how many bytes to receive */
        if (remaining > FW_PACKET_SIZE)
        {
            chunk_size = FW_PACKET_SIZE;
        }
        else
        {
            chunk_size = remaining;
        }

        /* Receive one chunk from PC */
        if (HAL_UART_Receive(&huart2,
                             firmware_buffer,
                             chunk_size,
                             HAL_MAX_DELAY) != HAL_OK)
        {
            HAL_UART_Transmit(&huart2,
                              (uint8_t *)"UART RECEIVE FAILED\r\n",
                              strlen("UART RECEIVE FAILED\r\n"),
                              100);

            return BL_ERROR;
        }

        /* Write chunk into Flash */
        if (Bootloader_WriteChunk(flash_address,
                                  firmware_buffer,
                                  chunk_size) != BL_OK)
        {
            HAL_UART_Transmit(&huart2,
                              (uint8_t *)"FLASH WRITE FAILED\r\n",
                              strlen("FLASH WRITE FAILED\r\n"),
                              100);

            return BL_ERROR;
        }

        /* Inform PC that this chunk is complete */
        HAL_UART_Transmit(&huart2,
                          (uint8_t *)"ACK\r\n",
                          strlen("ACK\r\n"),
                          100);

        /* Advance Flash address */
        flash_address += chunk_size;

        /* Reduce remaining firmware size */
        remaining -= chunk_size;
    }

    HAL_UART_Transmit(&huart2,
                      (uint8_t *)"FIRMWARE RECEIVED\r\n",
                      strlen("FIRMWARE RECEIVED\r\n"),
                      100);

    return BL_OK;
}


BL_Status_t Bootloader_WriteChunk(uint32_t flash_address,
                                  uint8_t *buffer,
                                  uint32_t length)
{
    uint16_t data;

    HAL_FLASH_Unlock();

    for (uint32_t i = 0; i < length; i += 2)
    {
        data = buffer[i];

        if ((i + 1) < length)
        {
            data |= ((uint16_t)buffer[i + 1] << 8);
        }
        else
        {
            /* Last byte: pad upper byte with 0xFF */
            data |= 0xFF00U;
        }

        if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD,
                              flash_address,
                              data) != HAL_OK)
        {
            HAL_FLASH_Lock();
            return BL_ERROR;
        }

        flash_address += 2;
    }

    HAL_FLASH_Lock();

    return BL_OK;
}
