/*
INCLUDES.H
*/
#ifndef _INCLUDES_H
#define _INCLUDES_H

#include <stdio.h>
#include <string.h>


#include <avr/io.h>
#include <avr/interrupt.h>
#include "avr/wdt.h"
#include "lc798x.h"
#include "util/delay.h"



/* If you have uC/OS-II version below 2.70 uncomment the includes for os_cpu.h and os_cfg.h */
#include  "os_cpu.h" 
#include  "os_cfg.h" 


/* ISR support macros */
#include "avr_isr.h"


#include "uCOS_II.H"

#endif //_INCLUDES_H
