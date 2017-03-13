/*
 * ABBcontroller.h
 *
 *  Created on: 10.3.2017
 *      Author: Tommi
 */

#ifndef ABBCONTROLLER_H_
#define ABBCONTROLLER_H_

#include <cstring>
#include <cstdio>
#include <stdlib.h> // itoa


#include "ModbusMaster.h"
#include "I2C.h"
#include "ITM_write.h"
#include "LiquidCrystal.h"
#include "DigitalIoPin.h"

class ABBcontroller {
public:
	ABBcontroller();
	void autoMeasure();
	virtual ~ABBcontroller();
	bool getMode();
	void printRegister(uint16_t reg);
	bool setFrequency(uint16_t freq);
	int measureAndCompare();
	bool startAbb();
	bool stopAbb();
	void printData();
	// UI functions
	void drawUserInterface(); // näin alkuun ~
	void readUserinput();

private:
	bool autoMode;
	ModbusMaster *node;
	//I2C i2c;
	uint16_t frequency;
	uint16_t pasc;

	// User interface
	LiquidCrystal* lcd;
	enum userInterfaceStates { menu, automaticMode, manualMode, endOfEnum };
	int userInterfaceState, selection;
	enum interfaceControls { ok, left, right };
	DigitalIoPin *switch1Ok, *switch2Left, *switch3Right;
	uint16_t frequencyTemp, pascTemp;

};

#endif /* ABBCONTROLLER_H_ */
