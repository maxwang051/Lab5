// ***** 0. Documentation Section *****
// TableTrafficLight.c for (Lab 10 edX), Lab 5 EE319K
// Runs on LM4F120/TM4C123
// Program written by: MAX WANG (mw35322), CONNOR WIDDER
// Date Created: 1/24/2015 
// Last Modified: 3/2/2016 
// Section 1-2pm     TA: Wooseok Lee
// Lab number: 5
// Hardware connections
// TO STUDENTS "REMOVE THIS LINE AND SPECIFY YOUR HARDWARE********
// east/west red light connected to PB5
// east/west yellow light connected to PB4
// east/west green light connected to PB3
// north/south facing red light connected to PB2
// north/south facing yellow light connected to PB1
// north/south facing green light connected to PB0
// pedestrian detector connected to PE2 (1=pedestrian present)
// north/south car detector connected to PE1 (1=car present)
// east/west car detector connected to PE0 (1=car present)
// "walk" light connected to PF3 (built-in green LED)
// "don't walk" light connected to PF1 (built-in red LED)

// ***** 1. Pre-processor Directives Section *****
#include <stdint.h>
#include "TExaS.h"
#include "tm4c123gh6pm.h"
#include "SysTick.h"

// ***** 2. Global Declarations Section *****

// FUNCTION PROTOTYPES: Each subroutine defined
void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts

#define GoS 0
#define GoSW 1
#define GoSP 2
#define WaitSW 3
#define WaitSP 4
#define WaitSWP 5
#define GoW 6
#define GoWP 7
#define WaitWS 8
#define WaitWP 9
#define WaitWSP 10
#define GoP 11
#define WaitP 12
#define Flash1 13
#define Flash2 14
#define Flash3 15
#define Flash4 16
#define Check1 17
#define Check2 18
#define Check3 19
#define Check4 20


typedef const struct State {
	uint32_t trafficOut;
	uint32_t pedestrianOut;
	uint32_t waitTime;
	uint32_t nextState[8];
} State;

State FSM[21] = {
	{0x21, 0x02, 80, {GoS, WaitSW, GoS, WaitSW, WaitSP, WaitSWP, WaitSP, WaitSWP}}, // GoS
	{0x21, 0x02, 80, {WaitSW, WaitSW, WaitSW, WaitSW, WaitSW, WaitSW, WaitSW, WaitSW}}, // GoSW
	{0x21, 0x02, 80, {WaitSP, WaitSP, WaitSP, WaitSP, WaitSP, WaitSP, WaitSP, WaitSP}}, // GoSP
	{0x22, 0x02, 40, {GoW, GoW, GoW, GoW, GoW, GoW, GoW, GoW}}, // WaitSW
	{0x22, 0x02, 40, {GoP, GoP, GoP, GoP, GoP, GoP, GoP, GoP}}, // WaitSP
	{0x22, 0x02, 40, {GoWP, GoWP, GoWP, GoWP, GoWP, GoWP, GoWP, GoWP}}, // WaitSWP
	{0x0C, 0x02, 80, {GoW, GoW, WaitWS, WaitWS, WaitWP, WaitWP, WaitWSP, WaitWSP}}, // GoW
	{0x0C, 0x02, 80, {WaitWP, WaitWP, WaitWP, WaitWP, WaitWP, WaitWP, WaitWP, WaitWP}}, // GoWP
	{0x14, 0x02, 40, {GoS, GoS, GoS, GoS, GoS, GoS, GoS, GoS}}, // WaitWS
	{0x14, 0x02, 40, {GoP, GoP, GoP, GoP, GoP, GoP, GoP, GoP}}, // WaitWP
	{0x14, 0x02, 40, {GoSP, GoSP, GoSP, GoSP, GoSP, GoSP, GoSP, GoSP}}, // WaitWSP
	{0x24, 0x08, 80, {GoP, WaitP, WaitP, WaitP, GoP, WaitP, WaitP, WaitP}}, // GoP
	{0x24, 0x00, 5, {Flash1, Flash1, Flash1, Flash1, Flash1, Flash1, Flash1, Flash1}}, // WaitP
	//{0x24, 0x00, 5, {GoW, GoW, GoW, GoW, GoW, GoW, GoW, GoW}}, // WaitPW
	//{0x24, 0x00, 5, {GoSW, GoSW, GoSW, GoSW, GoSW, GoSW, GoSW, GoSW}}, // WaitPSW
	{0x24, 0x02, 5, {Flash2, Flash2, Flash2, Flash2, Flash2, Flash2, Flash2, Flash2}}, // Flash1
	{0x24, 0x00, 5, {Flash3, Flash3, Flash3, Flash3, Flash3, Flash3, Flash3, Flash3}}, // Flash2
	{0x24, 0x02, 5, {Flash4, Flash4, Flash4, Flash4, Flash4, Flash4, Flash4, Flash4}}, // Flash3
	{0x24, 0x00, 5, {GoS, GoW, GoS, GoSW, GoP, GoW, GoS, GoSW}} // Flash4
	//{,,5{1,2,3,Check2,5,6,7,8}},//Check1
	//{,,5{1,2,3,Check3,5,6,7,8}},//Check2
	//{,,5{1,2,3,Check4,5,6,7,8}},//Check3
	//{,,5{1,2,3,GoP,5,6,7,8}},//Check4
};
// ***** 3. Subroutines Section *****

int main(void){ 
  TExaS_Init(SW_PIN_PE210, LED_PIN_PB543210); // activate grader and set system clock to 80 MHz
	
	SYSCTL_RCGCGPIO_R |= 0x32;
	int delay = SYSCTL_RCGC2_R;
	GPIO_PORTB_DIR_R |= 0x3F; 				// initialize port b for traffic lights
	GPIO_PORTB_AFSEL_R &= 0xC0;
	GPIO_PORTB_DEN_R |= 0x3F;
	GPIO_PORTE_DIR_R &= 0xF8;					// initialize port e for sensor inputs
	GPIO_PORTE_AFSEL_R &= 0xF8;
	GPIO_PORTE_DEN_R |= 0x07;
	GPIO_PORTF_DIR_R |= 0x0A;					// initialize port f for pedestrian lights
	GPIO_PORTF_AFSEL_R &= 0xF5;
	GPIO_PORTF_DEN_R |= 0x0A;
	SysTick_Init();
  
	uint32_t state = 0;
	
  EnableInterrupts();
  while(1){
		uint32_t sensorData = GPIO_PORTE_DATA_R & 0x07;
		uint32_t walkHeld = sensorData & 0x04;
		GPIO_PORTB_DATA_R = FSM[state].trafficOut;
		GPIO_PORTF_DATA_R = FSM[state].pedestrianOut;
		
		SysTick_Wait10ms(FSM[state].waitTime);
		walkHeld &= GPIO_PORTE_DATA_R & 0x04;
		sensorData &= 0x03;
		sensorData |= walkHeld;

    state = FSM[state].nextState[sensorData];
  }
}

