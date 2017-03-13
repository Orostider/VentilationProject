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
private:
	bool autoMode;

};

#endif /* ABBCONTROLLER_H_ */
