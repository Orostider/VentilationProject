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
	bool autoMeasure();
	bool manualMeasure();
	virtual ~ABBcontroller();
	bool getMode();
	void printRegister(uint16_t reg);
	bool setFrequency(uint16_t freq);
	void measure();
	int compare();
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
	static const uint16_t frequencyLimit = 100;
	uint16_t frequency;
	uint16_t pasc;
	int tickLimit; //One tick is 100ms
	uint16_t oneStep;
	float pressureCurrent;
	float pressureAvg;
	int pressureCount;
	// User interface
	LiquidCrystal* lcd;
	enum userInterfaceStates { menu, automaticMode, manualMode, warningUnreachablePressure, endOfEnum };
	int userInterfaceState, selection;
	enum interfaceControls { ok, left, right };
	DigitalIoPin *switch1Ok, *switch2Left, *switch3Right;
	uint16_t frequencyTemp, pascTemp;

};

#endif /* ABBCONTROLLER_H_ */
