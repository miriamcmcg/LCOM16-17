#include "keyboard.h"

static int hook_id_kbd;
int HAS_2_BYTE = 0;
unsigned short status;

int keyboard_subscribe_int()
{
	hook_id_kbd = IRQ_KBD;
	if (sys_irqsetpolicy(IRQ_KBD, IRQ_REENABLE|IRQ_EXCLUSIVE, &hook_id_kbd) != OK)
	{
		printf("Sys_irqserpolicy failed!\n");
		return -1;
	}
	else
	{
		return IRQ_KBD;
	}
}

int keyboard_unsubscribe_int()
{
	if(sys_irqrmpolicy(&hook_id_kbd) ==OK)
	{
		return 0;
	}
	else
	{
		printf("Sys_irqrmpolicy failed!\n");
		return -1;
	}
}

int keyboard_scan_c(){
	unsigned long scan_code;
	unsigned long st;

	unsigned int c = 0;


	if(sys_inb(OUT_BUF, &scan_code) != OK)
	{
		printf("sys_inb(OUTPUT_BUFFER_FULL, scancode) function failed! \n");
		return 1;
	}

	if(keyboard_scanning(scan_code) == -1)
	{
		printf("Pressed key ESC! \n");
		return -1;

	}

	return 0;


}

int keyboard_scanning(unsigned long scan_code)
{
	if(scan_code == TWO_BYTE_SCANCODE){
		HAS_2_BYTE = 1;
		return 0;
	}

	if(HAS_2_BYTE == 1)
	{
		scan_code = (scan_code | (TWO_BYTE_SCANCODE << 8));
		HAS_2_BYTE = 0;
	}

	if(scan_code & BREAK_CODE_BIT){

		printf("Breakcode: 0x%X \n", scan_code);
		if(scan_code == ESC)
		{
			return -1;
		}
	}
	else
	{
		printf("Makecode: 0x%X \n", scan_code);
	}

	return 0;
}



int keyboard_write_cmd(unsigned long command)
{
	unsigned long stat;
	unsigned int i=0;

	while (1)
	{
		if(sys_inb(STAT_REG, &stat)!=OK)
		{
			printf("sys_inb function failed!\n");
			return -1;
		}

		while (!(stat & INPUT_BUFFER_FULL))
		{
			if(sys_outb(KBD_CMD_REG, command)!=OK)
			{
				printf("sys_outb function failed! \n");
				return -1;
			}
			return 0;
		}
		tickdelay(micros_to_ticks(DELAY_US));
		i++;
	}

}

unsigned long keyboard_read_resp()
{
	unsigned long stat;
	unsigned int i=0;
	unsigned long data;
	while (1)
	{
		if(sys_inb(STAT_REG, &stat)!=OK)
		{
			printf("sys_inb function failed!\n");
			return -1;
		}
		while (!(stat & OUTPUT_BUFFER_FULL))
		{
			if(sys_inb(OUT_BUF, &data)!=OK)
			{
				printf("sys_inb function failed!\n");
				return -1;
			}

			if ((stat &(PARITY|TIMEOUT)) == 0){
				return data;
			}
			else
				return -1;

		}
		tickdelay(micros_to_ticks(DELAY_US));
		i++;
	}


}

int keyboard_get_resp(unsigned long command){
	unsigned int i=0;
	unsigned long data;
	do {
			if (keyboard_write_cmd(command) == -1)
			{
				printf("keyboard_write_cmd function failed\n");
				return 1;
			}
			data =keyboard_read_resp();
			if (data == -1) {
				printf("keyboard_read_resp function failed\n");
				return 1;
			}

			i++;
		} while ((data == ERROR || data == RESEND || data != ACK) && i < NUMBER_TRIES);

	if(i >= NUMBER_TRIES)
	{
		printf("Way too many tries! \n");
		return 1;
	}
	return 0;
}

int keyboard_leds(unsigned int led)
{

	if(keyboard_get_resp(KBD_SWITCH_LED) ==  1)
	{
		return 1;
	}

	status= status^BIT(led);
	keyboard_print_led(led);

	if(keyboard_get_resp(status) ==  1)
	{
		return 1;
	}

	return 0;
}

void keyboard_print_led(unsigned short status)
{
	switch(status){
	case 0:
		printf("Led 0 changed - Scroll Lock Indicator!\n");
		break;
	case 1:
		printf("Led 1 changed - Numeric Lock Indicator!\n");
		break;
	case 2:
		printf("Led 2 changed - Caps Lock Indicator!\n");
		break;
	}

}
