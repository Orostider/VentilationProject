/*
 * ABBcontroller.cpp
 *
 *  Created on: 10.3.2017
 *      Author: Tommi
 */

#include "ABBcontroller.h"
#include <cstring>
#include <cstdio>

#include "ModbusMaster.h"
#include "I2C.h"
#include "ITM_write.h"

static volatile int counter;
static volatile uint32_t systicks;

#ifdef __cplusplus
extern "C" {
#endif
/**
 * @brief	Handle interrupt from SysTick timer
 * @return	Nothing
 */
void SysTick_Handler(void)
{
	systicks++;
	if(counter > 0) counter--;
}
#ifdef __cplusplus
}
#endif

void Sleep(int ms)
{
	counter = ms;
	while(counter > 0) {
		__WFI();
	}
}

/* this function is required by the modbus library */
uint32_t millis() {
	return systicks;
}



ABBcontroller::ABBcontroller() {
	autoMode = true;
	node = new ModbusMaster(2);
	//I2C i2c = new I2C(0, 100000);
	frequency = 0;
	pasc = 50;

	// UI
	// Initializing the LCD
	DigitalIoPin* pin1 = new DigitalIoPin(0, 8, false);
	DigitalIoPin* pin2 = new DigitalIoPin(1, 6, false);
	DigitalIoPin* pin3 = new DigitalIoPin(1, 8, false);
	DigitalIoPin* pin4 = new DigitalIoPin(0, 5, false);
	DigitalIoPin* pin5 = new DigitalIoPin(0, 6, false);
	DigitalIoPin* pin6 = new DigitalIoPin(0, 7, false);
	lcd = new LiquidCrystal(pin1, pin2, pin3, pin4, pin5, pin6);
	lcd->print("A");

	// Setting default state for user interface
	userInterfaceState = menu;
	selection = automaticMode;

	// Initializing switches
	switch1Ok = new DigitalIoPin(0, 17, true, true, true);
	switch2Left = new DigitalIoPin(1, 11, true, true, true);
	switch3Right = new DigitalIoPin(1, 9, true, true, true);

	drawUserInterface();
}

bool ABBcontroller::startAbb(){
	node->begin(9600); // set transmission rate - other parameters are set inside the object and can't be changed here

	printRegister( 3); // for debugging

	node->writeSingleRegister(0, 0x0406); // prepare for starting

	printRegister(3); // for debugging

	Sleep(1000); // give converter some time to set up
	// note: we should have a startup state machine that check converter status and acts per current status
	//       but we take the easy way out and just wait a while and hope that everything goes well

	printRegister(3); // for debugging

	node->writeSingleRegister(0, 0x047F); // set drive to start mode

	printRegister(3); // for debugging

	Sleep(1000); // give converter some time to set up
	// note: we should have a startup state machine that check converter status and acts per current status
	//       but we take the easy way out and just wait a while and hope that everything goes well

	printRegister(3); // for debugging
	return true;
}

void ABBcontroller::printRegister(uint16_t reg){
	uint8_t result;
	// slave: read 16-bit registers starting at reg to RX buffer
	result = node->readHoldingRegisters(reg, 1);

	// do something with data if read is successful
	if (result == node->ku8MBSuccess)
	{
		printf("R%d=%04X\n", reg, node->getResponseBuffer(0));
	}
	else {
		printf("R%d=???\n", reg);
	}

}

bool ABBcontroller::setFrequency(uint16_t freq){
	uint8_t result;
	int ctr;
	bool atSetpoint;
	const int delay = 500;

	node->writeSingleRegister(1, freq); // set motor frequency

	printf("Set freq = %d\n", freq/40); // for debugging

	// wait until we reach set point or timeout occurs
	ctr = 0;
	atSetpoint = false;
	do {
		//		Sleep(delay);
		// read status word
		result = node->readHoldingRegisters(3, 1);
		// check if we are at setpoint
		if (result == node->ku8MBSuccess) {
			if(node->getResponseBuffer(0) & 0x0100) atSetpoint = true;
		}
		ctr++;
	} while(ctr < 20 && !atSetpoint);

	printf("Elapsed: %d\n", ctr * delay); // for debugging
	frequency = freq;
	return atSetpoint;

}

bool ABBcontroller::getMode(){
	return ABBcontroller::autoMode;
}

void ABBcontroller::autoMeasure(){
	uint16_t i;
	if (ABBcontroller::measureAndCompare() == 1){
		i = frequency+1;
		ABBcontroller::setFrequency(i);
	} else if (ABBcontroller::measureAndCompare() == -1 && (frequency -1) != 0) {
		i = frequency-1;
		ABBcontroller::setFrequency(i);
	} else {

	}
}

int ABBcontroller::measureAndCompare(){
	I2C i2c(0, 100000);

	uint8_t pressureData[3];
	uint8_t readPressureCmd = 0xF1;
	int16_t pressure = 0;

	//ITM_write("täs");
	if (i2c.transaction(0x40, &readPressureCmd, 1, pressureData, 3)) {
		/* Output temperature. */
		pressure = (pressureData[0] << 8) | pressureData[1];
		DEBUGOUT("Pressure read over I2C is %.1f Pa\r\n",	pressure/240.0);
	}
	else {
		DEBUGOUT("Error reading pressure.\r\n");
	}
	//Sleep(1000);

	if (pressure > pasc){
		return 1;
	} else if (pressure == pasc){
		return 0;
	} else {
		return -1;
	}
}

void ABBcontroller::drawUserInterface() {
	ITM_write("?????????\n");
	lcd->clear();
	lcd->setCursor(0,0);
	switch(userInterfaceState) {
	case menu:
		ITM_write("? \n");
		if (selection == automaticMode) {
			ITM_write("auto \n");
			lcd->print("Automatic Mode");
		} else if (selection == manualMode) {
			ITM_write("manual \n");
			lcd->print("Manual Mode");
		}

		break;

	case automaticMode:
		ITM_write("?? \n");
		lcd->print("set Automatic mode");
		break;

	case manualMode:
		ITM_write("??? \n");
		lcd->print("set manual mode");
		break;

	default:
		ITM_write("ei näin! \n");
		break;
	}
}

void ABBcontroller::readUserinput() {
	int userInput = 5;
	if (switch1Ok->read()) {
		ITM_write("ok\n");
		userInput = ok;
	}
	else if (switch2Left->read()) {
		ITM_write("left\n");
		userInput = left;
	}
	else if (switch3Right->read()) {
		ITM_write("right\n");
		userInput = right;
	} else {
		return;
	}


	if (userInterfaceState == menu) { // MENU
		ITM_write("Menu\n");
		switch (userInput) {
		case ok:
			userInterfaceState = selection;
			break;
		case left:
			if (selection == 1) {
				selection = 2;
			} else selection = 1;
			break;
		case right:
			if (selection == 1) {
				selection = 2;
			} else selection = 1;
			break;
		default:
			ITM_write("error at if '(userInterfaceState == menu)'\n");
			break;
		}

	} else if (userInterfaceState == autoMode) {
		ITM_write("Auto.\n");
		switch (userInput) {
		case ok:
			userInterfaceState = menu;
			break;
		case left:

			break;
		case right:

			break;
		default:
			ITM_write("error'\n");
			break;
		}
	} else if (userInterfaceState == manualMode) {
		ITM_write("Manual\n");
		switch (userInput) {
		case ok:
			userInterfaceState = menu;
			break;
		case left:

			break;
		case right:

			break;
		default:
			ITM_write("error'\n");
			break;
		}
	} else {
		ITM_write("ei tänne\n");
	}
	drawUserInterface();
	Sleep(100); // poista tää
}
ABBcontroller::~ABBcontroller() {
	// TODO Auto-generated destructor stub
}

