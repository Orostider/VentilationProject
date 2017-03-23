/*
 * ABBcontroller.cpp
 *
 *  Created on: 10.3.2017
 *      Authors: Tommi & Hege
 */

#include "ABBcontroller.h"

static volatile int counter;
static volatile uint32_t systicks;
static volatile int counterAutoMeasure;
static volatile int counterAvg;
static volatile bool flagAutoMeasure;
static volatile bool flagAvg;
static volatile bool flagMeasure;
static volatile bool flagSlowAlert;
static volatile bool flagPreventAlert = false;
static volatile int counterSlow;
static uint16_t pascLimit = 120;

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

	counterAutoMeasure++;
	counterAvg++;
	if (counterAutoMeasure >= 5000) {
		counterAutoMeasure = 0;
		flagAutoMeasure = true;
		flagAvg = true;
	}

	if (counterAvg >= 500){
		counterAvg = 0;
		flagMeasure = true;
	}
	counterSlow++;
	if (counterSlow >= 60000){
		counterSlow = 0;
		if (!flagPreventAlert){
			flagSlowAlert = true;
		}

	}
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
	frequency = 3000;
	pasc = 20;
	tickLimit = 300;
	oneStep = 250;

	pressureAvg = 0;
	pressureCount = 0;
	pressureCurrent = 0;



	// ------------------------- USER INTRERFACE INITIALIZATION
	// Setting up LCD
	DigitalIoPin* pin1 = new DigitalIoPin(0, 8, false);
	DigitalIoPin* pin2 = new DigitalIoPin(1, 6, false);
	DigitalIoPin* pin3 = new DigitalIoPin(1, 8, false);
	DigitalIoPin* pin4 = new DigitalIoPin(0, 5, false);
	DigitalIoPin* pin5 = new DigitalIoPin(0, 6, false);
	DigitalIoPin* pin6 = new DigitalIoPin(0, 7, false);
	lcd = new LiquidCrystal(pin1, pin2, pin3, pin4, pin5, pin6);

	// Setting default state for user interface
	userInterfaceState = menu;
	selection = automaticMode;

	// Initializing switches
	switch1Ok = new DigitalIoPin(0, 17, true, true, true);
	switch2Left = new DigitalIoPin(1, 11, true, true, true);
	switch3Right = new DigitalIoPin(1, 9, true, true, true);

	drawUserInterface();

	frequencyTemp = 50; pascTemp = 50;
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
		Sleep(delay);
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

bool ABBcontroller::manualMeasure(){
	if (!flagAutoMeasure) {
		if (flagMeasure){
			ABBcontroller::measure();
		}
		return false;
	}else {
		flagAutoMeasure = false;
		ABBcontroller::drawUserInterface();
		ABBcontroller::setFrequency(frequency);
		return true;
	}
}


bool ABBcontroller::autoMeasure(){
	if (!flagAutoMeasure) {
		if (flagMeasure){
			ABBcontroller::measure();
		}
		return false;

	} else {
		flagAutoMeasure = false;
		//uint16_t inc;
		int i = 0;
		int j = 0;
		uint8_t result;

		if (ABBcontroller::compare() == 1 && (frequency -oneStep) >= 1000){
			// slave: read (2) 16-bit registers starting at register 102 to RX buffer
			j = 0;
			do {
				result = node->readHoldingRegisters(102, 2);
				j++;
			} while(j < 3 && result != node->ku8MBSuccess);
			// note: sometimes we don't succeed on first read so we try up to threee times
			// if read is successful print frequency and current (scaled values)
			if (result == node->ku8MBSuccess) {
				printf("F=%4d, I=%4d  (ctr=%d)\n", node->getResponseBuffer(0), node->getResponseBuffer(1),j);
			}
			else {
				printf("ctr=%d\n",j);
			}
			// lippu tähän

			//Sleep(3000);
			i++;
			if(i >= 20) {
				i=0;
			}
			frequency= frequency-oneStep;
			ABBcontroller::setFrequency(frequency);

		} else if (ABBcontroller::compare() == -1&& (frequency +oneStep) <= 20000) {

			//while(ABBcontroller::measureAndCompare() == -1){
			// slave: read (2) 16-bit registers starting at register 102 to RX buffer
			j = 0;
			do {
				result = node->readHoldingRegisters(102, 2);
				j++;
			} while(j < 3 && result != node->ku8MBSuccess);
			// note: sometimes we don't succeed on first read so we try up to threee times
			// if read is successful print frequency and current (scaled values)
			if (result == node->ku8MBSuccess) {
				printf("F=%4d, I=%4d  (ctr=%d)\n", node->getResponseBuffer(0), node->getResponseBuffer(1),j);
			}
			else {
				printf("ctr=%d\n",j);
			}

			//Sleep(3000);
			i++;
			if(i >= 20) {
				i=0;
			}
			frequency+=oneStep;
			ABBcontroller::setFrequency(frequency);
			//}
		} else {
		}

		return true;
	}
}
void ABBcontroller::measure(){
	I2C i2c(0, 100000);

	uint8_t pressureData[3];
	uint8_t readPressureCmd = 0xF1;
	int16_t pressure = 0;

	if (i2c.transaction(0x40, &readPressureCmd, 1, pressureData, 3)) {
		/* Output temperature. */
		pressure = (pressureData[0] << 8) | pressureData[1];

	}
	else {
		DEBUGOUT("Error reading pressure.\r\n");
	}
	//Sleep(100);
	pressure = pressure/240.0;

	if (!flagAvg){
		pressureCount += pressure;
		flagMeasure = false;
	} else {
		pressureAvg = pressureCount/5.0;
		pressureCurrent = pressureAvg;
		pressureCount = 0;
		flagAvg = false;
		DEBUGOUT("Pressure read over I2C is %.1f Pa\r\n",	pressureCurrent);
		ABBcontroller::drawUserInterface();
	}
}

int ABBcontroller::compare(){
	int comparison;

	if(pressureCurrent >= pasc){
		comparison = pressureCurrent - pasc;
	} else {
		comparison = pasc - pressureCurrent;
	}

	if (comparison < 2){
		oneStep = 25;
		flagPreventAlert = true;
	} else if (comparison < 5){
		oneStep = 50;
	} else if (comparison > 30){
		oneStep = 1000;
	}else if (comparison > 10){
		oneStep = 500;
	} else {
		oneStep = 250;
	}


	if (pressureCurrent == pasc){
		return 0;
	} else if (pressureCurrent > pasc){
		return 1;
	} else {
		return -1;
	}
}

void ABBcontroller::drawUserInterface() {
	if (flagSlowAlert) { // set UI state to show alert
		flagSlowAlert = false;
		userInterfaceState = warningUnreachablePressure;
	}

	// print current UI state to ITM
	char testbuffer[30];
	snprintf ( testbuffer, 100, "state: %d selection: %d \n", userInterfaceState, selection);
	ITM_write(testbuffer);
	lcd->clear();
	lcd->setCursor(0,0);

	char buffer [33];
	std::string tempString = "";

	switch(userInterfaceState) {
	case menu:
		if (selection == automaticMode) {
			// first row
			lcd->print("Automatic Mode");
			// second row
			lcd->setCursor(0,1);
			if (autoMode) { // Wanted pressure + current pressure
				itoa(pasc, buffer, 10);
				tempString = "WP:" + std::string(buffer) + " CP:";
				itoa(pressureCurrent, buffer, 10);
				tempString += std::string(buffer);
				lcd->print(tempString);

			} else lcd->print("Activate");

		} else if (selection == manualMode) {
			// first row
			lcd->print("Manual Mode");
			// second row
			lcd->setCursor(0,1);
			if (!autoMode) { // Freq ja P
				itoa(frequencyTemp, buffer, 10);
				tempString = "F:" + std::string(buffer) + "% P:";

				itoa(pressureCurrent, buffer, 10);
				tempString += std::string(buffer) + " Pa";
				lcd->print(tempString);

			} else lcd->print("Activate");
		}
		break;

	case automaticMode:
		// upper line
		lcd->print("Set pressure:");
		//lower line
		lcd->setCursor(0,1);
		itoa(pascTemp, buffer, 10);
		tempString = std::string(buffer) + " Pa";
		lcd->print(tempString);
		break;

	case manualMode:
		// upper line
		lcd->print("Set fan speed:");
		// lower line
		lcd->setCursor(0,1);
		itoa(frequencyTemp, buffer, 10);
		tempString = std::string(buffer) + "%";
		lcd->print(tempString);
		break;

	case warningUnreachablePressure:
		lcd->print("Unreachable");
		lcd->setCursor(0,1);
		lcd->print("pressure!");
		break;

	default:
		ITM_write("drawUserInterface: Error!\n");
		break;
	}
}

void ABBcontroller::readUserinput() {
	int userInput = 5;

	/*
	// ohjailu switcheillä pois käytöstä.
	if (switch1Ok->read()) {
		userInput = ok;
	} else if (switch2Left->read()) {
		userInput = left;
	} else if (switch3Right->read()) {
		userInput = right;
	} else {
		return;
	}*/

	int c;
	c = Board_UARTGetChar();
	switch(c) {
	case 49: // 1
		userInput = left;
		break;
	case 50: // 2
		userInput = right;
		break;

	case 51: // 3
		userInput = ok;
		break;
	default: // ei painettu mitään
		return;
	}
	Board_UARTPutChar(c);
	Board_UARTPutChar('\r');
	Board_UARTPutChar('\n');

	if (userInterfaceState == menu) { // MENU - TOGGLE BETWEEN MODES
		if (userInput == ok) {
			userInterfaceState = selection;
			// temppien settaus  tähän
		} else if (userInput == left || userInput == right) {
			if (selection == automaticMode) {
				selection = manualMode;
			} else selection = automaticMode;
		}

	} else if (userInterfaceState == manualMode) { // SETTTING FREQUENCY/FAN SPEED FOR MANUAL MODE
		switch (userInput) {
		case ok:
			frequency = (20000/100)*frequencyTemp;
			userInterfaceState = menu;
			autoMode = false;
			break;

		case left:
			frequencyTemp -= 10;
			if (frequencyTemp <= 0) frequencyTemp = 100;
			break;

		case right:
			frequencyTemp += 10;
			if (frequencyTemp >= 100) frequencyTemp = 10;
			break;

		default:
			ITM_write("error'\n");
			break;
		}
	} else if (userInterfaceState == automaticMode) { // SETTING PRESSURE LEVEL FOR AUTOMATIC MODE
		switch (userInput) {
		case ok:
			pasc = pascTemp;
			userInterfaceState = menu;
			autoMode = true;
			flagPreventAlert = false;
			counterSlow = 0;
			break;

		case left:
			pascTemp -= 5;
			if (pascTemp <= 1) pascTemp = pascLimit;
			break;

		case right:
			pascTemp += 5;
			if (pascTemp >= pascLimit) pascTemp = 5;
			break;

		default:
			ITM_write("error'\n");
			break;
		}
	} else if(userInterfaceState == warningUnreachablePressure) { // NOTIFICATION - given pressure level might not be reachable
		if (userInput == ok) {
			flagPreventAlert = true;
			userInterfaceState = menu;
		}
	} else {
		ITM_write("readUserInput: Something went wrong!\n");
	}
	drawUserInterface();
	Sleep(200); // We don't want to immediately read another input
}

ABBcontroller::~ABBcontroller() {
	// TODO Auto-generated destructor stub
}

