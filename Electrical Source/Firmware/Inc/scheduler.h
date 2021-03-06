#ifndef SCHEDULER_H__
#define SCHEDULER_H__

#include "globals.h"
#include "timers.h"
#include <stdint.h>

#define EEPROM_START	0x08080000
#define DATA_WIDTH		2
#define LASTHOUR_ADDR 0x08080030
#define	LASTMINS_ADDR 0x08080031

	
void scheduler(int* GPS_active, int* XB_VHF_active);


#endif // SCHEDULER_H__