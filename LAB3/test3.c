#include "test3.h"
#include "i8042.h"
#include "i8254.h"
#include "keyboard.h"
#include "timer.h"
#include <minix/syslib.h>
#include <minix/drivers.h>


int kbd_test_scan(unsigned short ass) {
	int ipc_status;
	message msg;
	int r;
	int irq_set;
	int kbd_hook_id;



	int resp;
	if(ass < 0 && ass > 1)
	{
		printf("ass value is invalid! \n");
		return 1;
	}

	kbd_hook_id =keyboard_subscribe_int();
	if(kbd_hook_id == -1)
	{
		printf("keyboard_subscribe_int function failed! \n");
		return 1;
	}

	irq_set = BIT(kbd_hook_id);

	while(1) {
			/* Get a request message. */
			if ( (r=driver_receive(ANY, &msg, &ipc_status)) !=0 ) {
				printf("driver_receive failed with: %d", r);
				continue;
			}
			if (is_ipc_notify(ipc_status)) { /* received notification */
				switch (_ENDPOINT_P(msg.m_source)) {
				case HARDWARE: /* hardware interrupt notification */
					if (msg.NOTIFY_ARG & irq_set) { /* subscribed interrupt */
						{
							switch(ass){
							case 0:
								resp = keyboard_scan_c();
								break;
							case 1:
								resp = keyboard_scan_asm();
								break;

							}
						}

						break;
				default:
					break; /* no other notifications expected: do nothing */
					}
				}
			} else { /* received a standard message, not a notification */
				/* no standard messages expected: do nothing */
			}
			if(resp == -1)
				break;
	}


	if(keyboard_unsubscribe_int() == 0)
		return 0;
	else
	{
		printf("keyboard_unsubscribe_int function failed! \n");
		return 1;
	}

}

int kbd_test_leds(unsigned short n, unsigned short *leds)
{
	int kbd_hook_id;
	int timer_hook_id;
	int ipc_status;
	message msg;
	int r;
	int irq_set;
	int second=0;
	int i=0;


	if(timer_set_square(STANDARD_TIMER, STANDARD_FREQ) == 1)
	{
		printf("timer_set_square function failed! \n");
		return 1;
	}

	timer_hook_id = timer_subscribe_int();
	if(timer_hook_id == -1)
	{
		printf("timer_subscribe_int function failed! \n");
		return 1;
	}

	irq_set = BIT(timer_hook_id);

	while(second < n*STANDARD_FREQ) {
			if ( (r=driver_receive(ANY, &msg, &ipc_status)) !=0 ) {
				printf("driver_receive failed with: %d", r);
				continue;
			}
			if (is_ipc_notify(ipc_status)) {
				switch (_ENDPOINT_P(msg.m_source)) {
				case HARDWARE:
					if (msg.NOTIFY_ARG & irq_set) {
						second++;
						if (second % STANDARD_FREQ == 0)
						{
							if(keyboard_leds(leds[i]) != 0)
							{
								printf("Error on keyboard_leds function! \n");
							}
							i++;
						}
					}
					break;
				default:
					break;
				}
			} else {
			}
		}

	if(timer_unsubscribe_int() == 0)
		return 0;
	else
	{
		printf("timer_unsubscribe_int function failed! \n");
		return 1;
	}


}

int kbd_test_timed_scan(unsigned short n) {

	int ipc_status;
	message msg;
	int r;
	int timer_irq_set;
	int kbd_irq_set;
	int kbd_hook_id;
	int timer_hook_id;
	int timedOut = 0;
	int resp;
	int exitcondition = 0;

	if (timer_set_square(STANDARD_TIMER, STANDARD_FREQ) == 1) {
		printf("timer_set_square function failed! \n");
		return 1;
	}

	timer_hook_id = timer_subscribe_int();
	if (timer_hook_id == -1) {
		printf("timer_subscribe_int function failed! \n");
		return 1;
	}
	timer_irq_set = BIT(timer_hook_id);

	kbd_hook_id = keyboard_subscribe_int();
	if (kbd_hook_id == -1) {
		printf("_subscribe_int function failed! \n");
		return 1;
	}
	kbd_irq_set = BIT(kbd_hook_id);

	while (1) {
		if ((r = driver_receive(ANY, &msg, &ipc_status)) != 0) {
			printf("driver_receive failed with: %d", r);
			continue;
		}
		if (is_ipc_notify(ipc_status)) {
			switch (_ENDPOINT_P(msg.m_source)) {
			case HARDWARE:
				if (msg.NOTIFY_ARG & timer_irq_set) {
					timedOut++;
					if (timedOut % STANDARD_FREQ == 0){
						printf("Second %d without scanning! \n",timedOut/STANDARD_FREQ);
					}
				}
				if (msg.NOTIFY_ARG & kbd_irq_set) {
					resp = keyboard_scan_c();
					timedOut = 0;
				}
				break;
			default:
				break;
			}
		} else {
		}
		if (resp == -1) {
			break;
		} else if (timedOut >= n * STANDARD_FREQ) {
			printf("Scan timed Out! \n");
			break;
		}
	}
	if (timer_unsubscribe_int() == 0)
		return 0;
	else {
		printf("timer_unsubscribe_int function failed! \n");
		return 1;
	}
	if (keyboard_unsubscribe_int() == 0)
			return 0;
	else {
		printf("keyboard_unsubscribe_int function failed! \n");
		return 1;
	}

}