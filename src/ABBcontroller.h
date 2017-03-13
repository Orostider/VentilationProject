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

#include "ModbusMaster.h"
#include "I2C.h"
#include "ITM_write.h"

class ABBcontroller {
public:
	ABBcontroller();
	void autoMeasure();
	virtual ~ABBcontroller();
	bool getMode();
	bool measureAndCompare();
	void printRegister(uint16_t reg);
	bool setFrequency(uint16_t freq);
	bool startAbb();
	bool stopAbb();


private:
	bool autoMode;
	ModbusMaster *node;
	//I2C i2c;
	uint16_t freq;

};

#endif /* ABBCONTROLLER_H_ */
