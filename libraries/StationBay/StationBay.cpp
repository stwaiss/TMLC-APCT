	/* Station Bay Object Class for Arduino
	*  This file will manage all of the integer variables associated with a station bay for the
	*  Automated Pnuematic Cycle Tester at The Master Lock Company LLc, Technology Testing Center
	*  Oak Creek Wisconsin.
	*
	*  Any replication of this code without written consent from the The Master Lock Company LLC is strictly Prohibited
	*
	*  Author : Sean Waiss
	*  Date : 4/14/2016
	*/
	#include "StationBay.h"
	#include "Arduino.h"

	StationBay::StationBay(){}	

	StationBay::StationBay(int rp, int pp, int lcd){
		resetPin = rp;
		powerPin = pp;
		LCDPin = lcd;
		cycleCount = 0;
		powerStatus = 0;
		isStuck = 0;
		stationTimer = 0;
		timesIsStuck = 0;

		pinMode(resetPin, INPUT);
		pinMode(powerPin, INPUT);
		pinMode(LCDPin, INPUT);

		digitalWrite(resetPin, LOW);
		digitalWrite(powerPin, LOW);
		digitalWrite(LCDPin, LOW);
	}
	
	StationBay::StationBay(int rp, int pp, int lcd, long cc){
		resetPin = rp;
		powerPin = pp;
		LCDPin = lcd;
		cycleCount = cc;
		powerStatus = 0;
		isStuck = 0;
		stationTimer = 0;
		timesIsStuck = 0;
				
		pinMode(resetPin, INPUT);
		pinMode(powerPin, INPUT);
		pinMode(LCDPin, INPUT);

		digitalWrite(resetPin, LOW);
		digitalWrite(powerPin, LOW);
		digitalWrite(LCDPin, LOW);
	}
	

	long StationBay::getCycleCount(){return cycleCount;}

	int StationBay::getPowerStatus(){return powerStatus;}

	int StationBay::getIsStuck(){return isStuck;}

	int StationBay::getTimesIsStuck(){return timesIsStuck;}

	unsigned long StationBay::getStationTimer(){return stationTimer;}

	int StationBay::getPowerPin(){return powerPin;}

	int StationBay::getResetPin(){return resetPin;}

	int StationBay::getLCDPin(){return LCDPin;}

	void StationBay::incrementCycleCount(){cycleCount++;}

	void StationBay::resetCycleCount(){cycleCount = 0;}

	void StationBay::powerStatusOn(){powerStatus = 1;}

	void StationBay::powerStatusOff(){powerStatus = 0;}

	void StationBay::setIsStuckTrue(){isStuck = 1;}

	void StationBay::setIsStuckFalse(){isStuck = 0;}

	void StationBay::resetTimesIsStuck(){timesIsStuck = 0;}

	void StationBay::incrementTimesIsStuck(){timesIsStuck++;}

	void StationBay::setStationTimer(long st){stationTimer = st;}