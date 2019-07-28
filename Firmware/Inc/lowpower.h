#ifndef _LOWPOWER_H__
#define		_LOWPOWER_H__

#define SCR_DEEPSLEEP 0x0004;

int LPM_sleep(void);
int LPM_stop(void);
int LPM_gpioInit(void);


#endif 