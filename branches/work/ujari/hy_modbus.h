/* 		hy_modbus.h

   By S.Alford
 
   Modified by Jasem Mutlaq for Ujari Observatory

   These library of functions are designed to enable a program send and
   receive data from a Huanyang VFD. This device does not use a standard
   Modbus function code or data structure.
 
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
 
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
 
   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.                    
 
   This code has its origins with libmodbus
   
 */


#ifndef HY_MODBUS_H
#define HY_MODBUS_H

#include <stdint.h>

class Ujari;

#define MAX_DATA_LENGTH 		246
#define MAX_QUERY_LENGTH 		55
#define MAX_RESPONSE_LENGTH 	1024
#define MIN_QUERY_SIZE        	8
#define MAX_PACKET_SIZE      	8

/* Time out between frames in microsecond */
#define TIME_OUT_BEGIN_OF_FRAME 500000
#define TIME_OUT_END_OF_FRAME   500000

#ifndef FALSE
#define FALSE 					0
#endif

#ifndef TRUE
#define TRUE 					1
#endif

/* Local */
#define COMM_TIME_OUT           -0x0C
#define PORT_SOCKET_FAILURE     -0x0D
#define SELECT_FAILURE          -0x0E
#define TOO_MANY_DATAS          -0x0F
#define INVALID_CRC             -0x10
#define INVALID_EXCEPTION_CODE  -0x11


// Huanyang Function Codes
#define	FUNCTION_READ			0x01
#define	FUNCTION_WRITE			0x02
#define	WRITE_CONTROL_DATA		0x03
#define	READ_CONTROL_STATUS		0x04
#define	WRITE_FREQ_DATA			0x05
#define	LOOP_TEST				0x08

/***********************************************************************

	 Note: All functions used for sending or receiving data via
	       modbus return these return values.


	Returns:	string_length if OK
			0 if failed
			Less than 0 for exception errors

***********************************************************************/

#define COMMS_FAILURE 			0
#define ILLEGAL_FUNCTION 		-1
#define ILLEGAL_PARAMETER	 	-2
#define ILLEGAL_DATA_VALUE 		-3
#define SLAVE_DEVICE_FAILURE 	-4
#define ACKNOWLEDGE 			-5
#define SLAVE_DEVICE_BUSY 		-6
#define NEGATIVE_ACKNOWLEDGE 	-7
#define MEMORY_PARITY_ERROR 	-8

#define PORT_FAILURE 			-11

#define MAX_HOST_NAME           64

typedef struct _modbus_param_t {
    char host[MAX_HOST_NAME];		/* host */
    int port;                   /* port */
	int print_errors;			/* whether or not to print errors.  0=no print, 1=print */
    Ujari *telescope;
    //int baud;					/* Bauds: 19200 */
    //char parity[5];				/* Parity: "even", "odd", "none" */
    //int data_bit;				/* Data bit */
    //int stop_bit;				/* Stop bit */
    //struct termios old_tios;	/* Save old termios settings */
	int fd;						/* Descriptor (tty or socket) */
	int debug;					/* Flag debug */
} modbus_param_t;


/* Initializes the modbus_param_t structure for RTU.
   - device: "/dev/ttyS0"
   - baud:   9600, 19200, 57600, 115200, etc
   - parity: "even", "odd" or "none" 
   - data_bits: 5, 6, 7, 8 
   - stop_bits: 1, 2
*/

typedef struct _modbus_data_t {
	unsigned char slave;			/* slave address */
	unsigned char function;			/* function code */
	unsigned char parameter;		/* PDxxx paramter */
	int data;						/* Data to send */
	unsigned char ret_length;		/* length of data returned from slave */
	unsigned char ret_parameter;	/* parameter returned from slave */
	int ret_data;					/* Data returned from slave */
} modbus_data_t;

int hy_modbus(modbus_param_t *mb_param, modbus_data_t *mb_data);
int hy_modbusN( modbus_param_t *mb_param, modbus_data_t *mb_data );

void modbus_init_rtu(modbus_param_t *mb_param, const char *host, int port, Ujari* telescope);
void modbus_close(modbus_param_t *mb_param);
int modbus_connect(modbus_param_t *mb_param);
void modbus_close(modbus_param_t *mb_param);
const char *modbus_strerror(int errnum);


#endif  /* HY_MODBUS_H */
