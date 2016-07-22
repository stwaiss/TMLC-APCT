/* Station Bay Object Class Header File for Arduino
*  This is the header file associated with a station bay class file for the
*  Automated Pnuematic Cycle Tester at The Master Lock Company LLc, Technology Testing Center
*  Oak Creek Wisconsin.
*
*  Any replication of this code without written consent from the The Master Lock Company LLC is strictly Prohibited
*
*  Author : Sean Waiss	
*  Date : 4/14/2016
*/

#ifndef StationBay_h
#define StationBay_h

#include "Arduino.h"

class StationBay{

	private:
	int resetPin;
	int powerPin;
	int LCDPin;
	volatile unsigned long cycleCount;
	volatile int powerStatus;
	volatile int isStuck;
	volatile int timesIsStuck;
	volatile unsigned long stationTimer;
	
	public:
	StationBay();
	StationBay(int rp, int pp, int lcd);
	StationBay(int rp, int pp, int lcd, long cc);
	long getCycleCount();
	int getPowerStatus();
	int getIsStuck();
	unsigned long getStationTimer();
	int getTimesIsStuck();
	int getPowerPin();
	int getResetPin();
	int getLCDPin();

	void incrementCycleCount();
	void resetCycleCount();
	void powerStatusOff();
	void powerStatusOn();
	void setIsStuckTrue();
	void setIsStuckFalse();
	void resetTimesIsStuck();
	void incrementTimesIsStuck();
	void setStationTimer(long st);
};
#endif