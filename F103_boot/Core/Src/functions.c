
#include"main.h"
extern UART_HandleTypeDef huart2;
#define APP_MAX_SIZE      48128U 




/*
 * This function erases the application flash memory before
 * receiving and writing the new firmware.
 */
BL_Status_t Bootloader_EraseApplication(void)
{
    FLASH_EraseInitTypeDef EraseInit;
    uint32_t PageError = 0;

    HAL_FLASH_Unlock();

    EraseInit.TypeErase = FLASH_TYPEERASE_PAGES;
    EraseInit.PageAddress = APP_START_ADDR;
    EraseInit.NbPages = 47;

    if (HAL_FLASHEx_Erase(&EraseInit, &PageError) != HAL_OK)
    {
        HAL_FLASH_Lock();
        HAL_UART_Transmit(&huart2, (uint8_t *)"FLASH ERASE FAILED\r\n", strlen("FLASH ERASE FAILED\r\n"), 100);

        return BL_ERROR;
    }

    HAL_FLASH_Lock();
    HAL_UART_Transmit(&huart2, (uint8_t *)"FLASH ERASE OK\r\n", strlen("FLASH ERASE OK\r\n"), 100);

    return BL_OK;
}




#define FW_PACKET_SIZE 256U

/*
 * This function receives the application firmware from the PC
 * in chunks and writes each chunk into Flash memory.
 */
BL_Status_t Bootloader_ReceiveFirmware(const app_header_t *header)
{
    uint8_t firmware_buffer[FW_PACKET_SIZE];

    uint32_t remaining = header->size;
    uint32_t chunk_size;
    uint32_t flash_address = APP_START_ADDR;

    while (remaining > 0U)
    {
        /* Decide chunk size */
        if (remaining > FW_PACKET_SIZE)
        {
            chunk_size = FW_PACKET_SIZE;
        }
        else
        {
            chunk_size = remaining;
        }

        /* Receive one chunk from PC */
        if (HAL_UART_Receive(&huart2, firmware_buffer, chunk_size, HAL_MAX_DELAY) != HAL_OK)
        {
            HAL_UART_Transmit(&huart2, (uint8_t *)"UART RECEIVE FAILED\r\n", strlen("UART RECEIVE FAILED\r\n"), 100);
            return BL_ERROR;
        }

        /* Write chunk into Flash */
        if (Bootloader_WriteChunk(flash_address, firmware_buffer, chunk_size) != BL_OK)
        {
            HAL_UART_Transmit(&huart2, (uint8_t *)"FLASH WRITE FAILED\r\n", strlen("FLASH WRITE FAILED\r\n"), 100);
            return BL_ERROR;
        }

        /* Inform PC that this chunk is complete */
        HAL_UART_Transmit(&huart2, (uint8_t *)"ACK\r\n", strlen("ACK\r\n"), 100);

        /* Advance Flash address */
        flash_address += chunk_size;

        /* Reduce remaining firmware size */
        remaining -= chunk_size;
    }

    HAL_UART_Transmit(&huart2, (uint8_t *)"FIRMWARE RECEIVED\r\n", strlen("FIRMWARE RECEIVED\r\n"), 100);

    return BL_OK;
}




BL_Status_t Bootloader_WriteChunk(uint32_t flash_address,uint8_t *buffer, uint32_t length)
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

        if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, flash_address, data) != HAL_OK)
        {
            HAL_FLASH_Lock();
            return BL_ERROR;
        }

        flash_address += 2;
    }

    HAL_FLASH_Lock();

    return BL_OK;
}






BL_Status_t Bootloader_VerifyFirmwareCRC(const app_header_t *header)
{
    uint32_t calculated_crc;
    char buff[50];
    calculated_crc = Bootloader_CalculateCRC(APP_START_ADDR, header->size);

    sprintf(buff, "Calculated CRC = 0x%08lX\r\n", calculated_crc);
    HAL_UART_Transmit(&huart2,(uint8_t*)buff,strlen(buff),100);
    sprintf(buff,"Recived CRC= 0x%08lX\r\n",header->crc);

    HAL_UART_Transmit(&huart2,(uint8_t*)buff,strlen(buff),100);

    if (calculated_crc != header->crc)
    {
        HAL_UART_Transmit(&huart2,(uint8_t *)"CRC FAILED\r\n",strlen("CRC FAILED\r\n"),100);

        return BL_ERROR;
    }

    HAL_UART_Transmit(&huart2,(uint8_t *)"CRC OK\r\n",strlen("CRC OK\r\n"),100);

    return BL_OK;
}



BL_Status_t Bootloader_EraseHeader(void)
{
    FLASH_EraseInitTypeDef EraseInit;
    uint32_t PageError = 0;

    HAL_FLASH_Unlock();

    EraseInit.TypeErase   = FLASH_TYPEERASE_PAGES;
    EraseInit.PageAddress = APP_HEADER_ADDR;
    EraseInit.NbPages     = 1;   // 1K header region = 1 page on F103 (1K page size)

    if (HAL_FLASHEx_Erase(&EraseInit, &PageError) != HAL_OK)
    {
        HAL_FLASH_Lock();
        HAL_UART_Transmit(&huart2, (uint8_t *)"HEADER ERASE FAILED\r\n", strlen("HEADER ERASE FAILED\r\n"), 100);
        return BL_ERROR;
    }

    HAL_FLASH_Lock();
    HAL_UART_Transmit(&huart2, (uint8_t *)"HEADER ERASE OK\r\n", strlen("HEADER ERASE OK\r\n"), 100);

    return BL_OK;
}

BL_Status_t Bootloader_WriteHeader(const app_header_t *header)
{
    BL_Status_t status;

    status = Bootloader_WriteChunk(APP_HEADER_ADDR,
                                    (uint8_t *)header,
                                    sizeof(app_header_t));

    if (status != BL_OK)
    {
        HAL_UART_Transmit(&huart2, (uint8_t *)"HEADER WRITE FAILED\r\n", strlen("HEADER WRITE FAILED\r\n"), 100);
        return BL_ERROR;
    }

    HAL_UART_Transmit(&huart2, (uint8_t *)"HEADER WRITE OK\r\n", strlen("HEADER WRITE OK\r\n"), 100);
    return BL_OK;
}

