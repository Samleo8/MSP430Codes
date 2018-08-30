/***************************
 * MAIN.H
 * Use this header file to attach functions and define constants
****************************/

#include "msp430fr4133.h"

//Number parsing
#define TRUE 0xFF
#define FALSE 0x00

#define ABS(n) (n>0)?n:-n
#define MAX(m,n) (m>n)?m:n
#define MIN(m,n) (m<n)?m:n

//Timers/PWMs
/* PWM PERIOD
 *  Currently using timer SMCLK (4MHz), divided by 8 and predivided by 8
 *  ==> 62500Hz
 *
 * PWM DUTY CYCLES:
 *      Stop: 0.5
 *      Full Clockwise: 0.9
 *      Full AntiClockwise: 0.1
 */

#define PWM_ONE_SECOND 62500
#define PWM_PERIOD 62500

#define PWM_STOP 0.5
#define PWM_CLOCK_FULL 0.9
#define PWM_ANTICLOCK_FULL 0.1
#define PWM_MULTIPLIER 0.04 //(PWM_CLOCK_FULL-PWM_ANTICLOCK_FULL)/20

void Init_GPIO(void);
void Init_Clock(void);
void Init_Timer(void);

void MotorControl(unsigned int, int);
