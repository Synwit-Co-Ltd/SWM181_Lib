#ifndef __CAN_SWM240_H__
#define __CAN_SWM240_H__

extern volatile uint8_t g_ucCanRxFlag;


void can_slave_init(void);

void can_recv_thread(void);

uint8_t canSend(CAN_PORT notused, Message *m);


#endif //__CAN_SWM240_H__
