/*
 * app_header.h
 *
 *  Created on: 02-Jun-2026
 *      Author: Admin
 */

#ifndef INC_APP_HEADER_H_
#define INC_APP_HEADER_H_


#define  APP_MAGIC 	0x1ABCDEF0
typedef struct
{
	uint32_t ota_flag;
    uint32_t magic_key;
    uint32_t size;      // application size in bytes
    uint32_t crc;       // CRC32 of application
    uint32_t version;
} app_header_t;


#endif /* INC_APP_HEADER_H_ */
