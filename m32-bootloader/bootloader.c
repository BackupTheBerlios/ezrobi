/* 
   Copyright (c) 2003, Reza Naima <reza@reza.net>
   All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:

   * Redistributions of source code must retain the above copyright
     notice, this list of conditions and the following disclaimer.
   * Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions and the following disclaimer in
     the documentation and/or other materials provided with the
     distribution.
   * Neither the name of the copyright holders nor the names of
     contributors may be used to endorse or promote products derived
     from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
  POSSIBILITY OF SUCH DAMAGE. 
*/


/*********************************************************/
/*                   I N C L U D E    		             */
/*********************************************************/	

#include "avr-boot.h" /* To become <avr/boot.h> in near future */
#include "bootloader.h"
// my avr-gcc have this header...
// #include "pgmspace.h" /* Inluded in more contemporary avr-gcc bundles */
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <inttypes.h>
#include "usb.h"

/*********************************************************/
/*                   D E F I N E    		             */
/*********************************************************/	
#define  PageSize 	 SPM_PAGESIZE

#define  FALSE		 0
#define  TRUE		 1

#define RunApplication() \
	MCUCR = 0x01; MCUCR = 0x00; \
	__asm__ __volatile__ ("jmp 0x0000\n\t" ::) 


#define debug(x) uart_puts1(x)
/*********************************************************/
/*           G L O B A L    V A R I A B L E S		 */
/*********************************************************/	
volatile uint8_t *usb_boot_dev;

/*********************************************************/
/*               F U N C T I O N S                       */
/*********************************************************/	

/* USB Functions */
#ifdef BOOT_USB
static inline uint8_t RxChar(void) {
	loop_until_bit_is_clear( USB_TXRXPIN, USB_RXE);
	return *usb_boot_dev;
}

static inline void TxChar(uint8_t x) {
	loop_until_bit_is_clear(USB_TXRXPIN, USB_TXE); 
	*usb_boot_dev = x;
}

static inline uint8_t checkLT(void) {
	if (bit_is_clear(USB_TXRXPIN, USB_RXE)) {
		return (*usb_boot_dev == '<');
	}
	return 0;
}

static inline void CommInit(void) {
	/* USB INIT */

	usb_boot_dev = (uint8_t*) USB_DEV_ADDR;
	sbi(MCUCR, SRE);
	DDRD = DDRD & ~(_BV(6) | _BV(7));
	XMCRB = _BV(XMM2) | _BV(XMM1) | _BV(XMM0);

}

#define flushComm() \
	{ uint8_t tmp_char; \
	  while(bit_is_clear(USB_TXRXPIN, USB_RXE)) { \
		tmp_char = *usb_boot_dev;   \
	  } \
	} 


/* UART Functions */

#elif defined BOOT_UART

//not if we have a Mega32
//#define UCSRA UCSR1A
//#define UCSRB UCSR1B
//#define UCSRC UCSR1C
//#define UDR UDR1
//#define UBRRL UBRR1L
//#define UBRRH UBRR1H

static inline uint8_t RxChar(void) {
	loop_until_bit_is_set(UCSRA, RXC);
	return UDR;
}

static inline void TxChar(uint8_t x) {
	loop_until_bit_is_set(UCSRA, UDRE);
	UDR = x;
}

static inline uint8_t checkLT(void) {
	if (bit_is_set(UCSRA, RXC)) {
		return (UDR == '<');
	}
	return 0;
}

static inline void initComm(void) {
#if (F_CPU == 16000000)
# if (BTL_SPEED == 57600)
    UBRRH=0;  UBRRL=34;
# elif (BTL_SPEED == 38400)
    UBRRH=0;  UBRRL=51;
# elif (BTL_SPEED == 19200)
    UBRRH=0;  UBRRL=103;
# elif (BTL_SPEED == 9600)
    UBRRH=0;  UBRRL=207;
# elif (BTL_SPEED == 4800)
    UBRRH=1;  UBRRL=160;
# else
#  error "baudrate: illegal or undefined BTL_SPEED" 
# endif
#else
# error "baudrate: unsupported CPU speed (F_CPU)" 
#endif
    /* enable transmit and recieve */
    UCSRA = _BV(U2X); /* double-speed */
    UCSRC = 0x06;     /* 8 Bit */
    UCSRB = _BV(RXEN) | _BV(TXEN);
}

#define flushComm()

#elif
#error "Need to define a communication method.  Try -DBOOT_USB or -DBOOT_UART"
#endif

static inline void usbprint (char *txt) {
	while (*txt) {
		TxChar( *txt );
		txt++;
	}
}

/*
#define usbprint32(x) usbprint16((x & 0xFFFF0000) >> 16 ); usbprint16(x & 0x0000FFFF);
#define usbprint16(x) usbprint8((x & 0xFF00) >> 8 ); usbprint8(x & 0x00FF);
static inline void usbprint8 ( uint8_t val) {
	static char nibble2hex[16] = {'0','1','2','3','4','5', '6','7','8','9','a', 'b','c','d','e','f'};
	TxChar( nibble2hex[ (val & 0xF0) >> 4 ]);
	TxChar( nibble2hex[ (val & 0x0F)  ]);
}
*/

int bootloader(void) {
	static uint8_t  PageBuffer[PageSize];
	static uint8_t  temp8bit;

	static uint32_t temp_address;
	static uint16_t temp_data;

	static uint16_t page_index;

	static uint8_t  LocalCheckSum = 0;
	static uint8_t  RemoteCheckSum = 0;

	static uint32_t Address;

	cli();          // disable all interrupts

	initComm();

	/* Try to connect 10 times, else Jump to application code */
	for (temp8bit=0; temp8bit<5; temp8bit++) {
		flushComm();
		TxChar('>');
		_delay_loop_2(52428); // ~ .05 seconds @ 4Mhz
		_delay_loop_2(52428); // ~ .05 seconds @ 4Mhz
		if (checkLT()) {
			goto BOOTLOAD;
		}
	}
	RunApplication();
		
  BOOTLOAD:
	// Send PageSize 
	TxChar('0' + (int)((PageSize/1000)%10));
	TxChar('0' + (int)((PageSize/100)%10));
	TxChar('0' + (int)((PageSize/10)%10));
	TxChar('0' + (int)((PageSize)%10));
	TxChar('!');

	while(1) {

		/* GET PAGE ADDRESS */
		temp8bit = RxChar();	
		Address  = ((uint32_t)temp8bit) << (8*3);
		temp8bit = RxChar();	
		Address |= ((uint32_t)temp8bit) << (8*2);
		temp8bit = RxChar();	
		Address |= ((uint32_t)temp8bit) << 8;
		temp8bit = RxChar();	
		Address |= ((uint32_t)temp8bit);

	
		//usbprint("[BL] Got Address : 0x"); usbprint32(Address); usbprint("\r\n");

		// Check to see if we're done programming
		if ( Address == 0xFFFFFFFF )  {
			//RAMPZ = 0; 
			MCUCR = 0x01; 
			MCUCR = 0x00; 
			__asm__ __volatile__ ("jmp 0x0000\n\t" ::); 
		}

		// Read Data and Store in memory, generating the checksum
		LocalCheckSum = 0;
		for (page_index=0; page_index < PageSize; page_index++) {
			loop_until_bit_is_clear( USB_TXRXPIN, USB_RXE);
			PageBuffer[page_index] = *usb_boot_dev;
			/*
			usbprint("[BL] Page Index 0x"); usbprint8(page_index); 
			usbprint(" = 0x"); usbprint8(PageBuffer[page_index]); 
			usbprint("\r\n");
			*/
			LocalCheckSum += PageBuffer[page_index];
		}

	
		// Write Back Data in our Buffer	
		for (page_index = 0; page_index < PageSize; page_index += 1) {
			/*
			usbprint ("[BL] Data = 0x"); 
			usbprint8(PageBuffer[page_index]); 
			usbprint ("("); 
			usbprint8(page_index); 
			usbprint (")\r\n");
			*/
		}

		// The checksum computed on the other side
		RemoteCheckSum = RxChar();
		/*
		usbprint("[BL] Local Checksum = 0x"); usbprint8(LocalCheckSum); usbprint("\r\n");
		usbprint("[BL] Remote Checksum = 0x"); usbprint8(RemoteCheckSum); usbprint("\r\n");
		*/
		if (RemoteCheckSum != LocalCheckSum) {
			TxChar('@'); // Checksum error, try again.
			continue;
		}

		// Erase page.
		boot_page_erase((unsigned long)Address);
		while(boot_rww_busy()) {
			boot_rww_enable();
		}
	
		// Write data to temporary page
		for (page_index = 0; page_index < PageSize; page_index += 2) {
			temp_address = Address + (unsigned long) page_index;
			temp_data    =  (uint16_t)(PageBuffer[page_index]) +
					((uint16_t)(PageBuffer[page_index+1]) << 8);
			boot_page_fill( temp_address, temp_data);
		/*
			usbprint  ("[BL] Writing [0x"); 
			usbprint16(temp_data); 
			usbprint  ("] to address 0x"); 
			usbprint32(temp_address); 
			usbprint  ("\r\n");
		*/
		}

		// Write page.
		boot_page_write((unsigned long)Address);
		while(boot_rww_busy()) {
			boot_rww_enable();
		}

		// Check that the page was written correctly
		for (page_index=0; page_index<PageSize; page_index++) {
			if (PageBuffer[page_index] != pgm_read_byte(Address + page_index)) {
				TxChar('$'); /* VERIFY ERROR, RETRY */
				continue;
			}
		}
		TxChar('!'); // Page Written Correctly
	}
	RunApplication();

	return 0;
}
