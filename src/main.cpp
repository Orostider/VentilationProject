/*
===============================================================================
 Name        : main.c
 Author      : $(author)
 Version     :
 Copyright   : $(copyright)
 Description : main definition
===============================================================================
 */

#if defined (__USE_LPCOPEN)
#if defined(NO_BOARD_LIB)
#include "chip.h"
#else
#include "board.h"
#endif
#endif

#include <cr_section_macros.h>

// TODO: insert other include files here

// TODO: insert other definitions and declarations here

#include <cstring>
#include <cstdio>

#include "ModbusMaster.h"
#include "I2C.h"
#include "ITM_write.h"
#include "ABBcontroller.h"


/**
 * @brief	Main UART program body
 * @return	Always returns 1
 */
int main(void)
{

#if defined (__USE_LPCOPEN)
	// Read clock settings and update SystemCoreClock variable
	SystemCoreClockUpdate();
#if !defined(NO_BOARD_LIB)
	// Set up and initialize all required blocks and
	// functions related to the board hardware
	Board_Init();
	// Set the LED to the state of "On"
	Board_LED_Set(0, true);
#endif
#endif

	/* Enable and setup SysTick Timer at a periodic rate */
	SysTick_Config(SystemCoreClock / 1000);

	/* Enable RIT */
	Chip_RIT_Init(LPC_RITIMER);

	/* Enable ITM printing */
	ITM_init();


	//abbModbusTest();
	//i2cTest();

	ABBcontroller abbController;
	abbController.startAbb();
	while (1) {
		abbController.readUserinput();

		if (abbController.getMode()){
			abbController.autoMeasure();
		} else{
			abbController.manualMeasure();
		}
	}

	return 1;
}

