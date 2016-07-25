  /* Networking of Pneumatic Cycle Tester for Arduino
  *  This file runs Interrupt Service Routines and Ethernet platform for the
  *  Automated Pnuematic Cycle Tester at The Master Lock Company LLC, Technology Testing Center
  *  Oak Creek, Wisconsin.
  *
  *  Any replication of this code without written consent from The Master Lock Company LLC is strictly prohibited
  *
  *  Author : Sean Waiss, Chris Ostram, Brandon Howard, Ernest Panzera
  *  Date : 4/18/2016
  */
  
#include <LiquidCrystal.h>
#include <EEPROM.h>
#include <Ethernet.h>
#include <SPI.h>
#include <avr/interrupt.h>
#include <StationBay.h>
#include <PinChangeInt.h>

//Declares an array of 6 station bay objects
StationBay bays[6];
byte mac[] = { 0x00, 0xAA, 0xBB, 0xCC, 0xDE, 0x02 };
EthernetClient client;
EthernetServer server(80);
LiquidCrystal lcd(37,36,35,34,33,32);
unsigned long mainLCDTimerRefresh = 0;
int ethernetCounter = 0;

//Global Functions
void(*resetFunc)(void) = 0;
void writeToHTML(void);

void setup() {
  lcd.begin(20,4);
  lcd.clear();
  
	Serial.begin(9600);
	Serial.println("Begin");
	  
  PCintPort::attachInterrupt(5, isrPCI5, RISING);
  PCintPort::attachInterrupt(6, isrPCI6, RISING);
  PCintPort::attachInterrupt(7, isrPCI7, RISING);
  PCintPort::attachInterrupt(9, isrPCI9, RISING);
  PCintPort::attachInterrupt(16, isrPCI16, RISING);
  
  lcd.setCursor(0,0);
  lcd.print("Beginning Program");
//  for(int i = 0; i < 1024; i++){
//  	if(EEPROM.read(i) == 1){
//  	  readInCycleCounts();
//  	  lcd.setCursor(0,1);
//  	  lcd.print("Reading in counts");
//  	  delay(3000);
//  	} 
//  }
//  if(EEPROM.read(0) == 0){
//	  Serial.println("No EEPROM, Initializing Bays");
//	  bays[0] = StationBay();
//	  bays[1] = StationBay(4, 24, A8);
//	  bays[2] = StationBay(5, 25, 14);
//	  bays[3] = StationBay(6, 26, 13);
//	  bays[4] = StationBay(7, 27, 12);
//	  bays[5] = StationBay(8, 28, 11);
//	  lcd.setCursor(0,1);
//	  lcd.print("Bays Initialized");
//  } 
  
  lcd.setCursor(0,3);
  lcd.print("Trying to connect");
  Serial.println("Trying to connect");
  if(Ethernet.begin(mac) == 0){
  	lcd.clear();
  	lcd.setCursor(5,0);
  	lcd.print("Failed to");
  	lcd.setCursor(3, 1);
  	lcd.print("configure DHCP");
  	lcd.setCursor(3,3);
  	lcd.print("RESET REQUIRED");
  	delay(5000);
  	resetFunc();
  }
  lcd.clear();
  Serial.println("lcd cleared");

  server.begin();
}

//*****************************************************************************************************************************************

void loop() {
  
  checkTimers();
  checkPower();
  checkReset();
  
  if(ethernetCounter >= 5){
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Ethernet has failed");
      lcd.setCursor(0,1);
      lcd.print("5 times consecutively");
      lcd.setCursor(0,2);
      lcd.print("RESET REQUIRED");
      delay(10000);
	    resetFunc();
  }
  switch(Ethernet.maintain()){
    case 0:
      //Serial.print("In case 0");
      printIPAddress();
      //wdt_reset();
      break;
    case 1:
      //Serial.print("In case 1");    
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Ethernet renew failed");
      ethernetCounter++;
      break;
    case 2:
     //Serial.print("In case 2");
      printIPAddress();
      //wdt_reset();
      break;
    case 3:
      //Serial.print("In case 3");
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Ethernet rebind fail");
      ethernetCounter++;
      break;
    case 4:
	  //Serial.print("In case 4");
      printIPAddress();
      //wdt_reset();
      break;
    default:
      break;
  }
  writeToHTML();
}

//**************************************************************************************************************************************
//Pin Change Interrupts
//LCD interrupts
void isrPCI5(){
  bays[5].incrementCycleCount();
  bays[5].setIsStuckFalse();
  bays[5].setStationTimer(millis() + 7000);
  //checkTimers(); 
}

void isrPCI6(){
  bays[4].incrementCycleCount();
  bays[4].setIsStuckFalse();
  bays[4].setStationTimer(millis() + 7000);
  //checkTimers();
}

void isrPCI7(){
  bays[3].incrementCycleCount();
  bays[3].setIsStuckFalse();
  bays[3].setStationTimer(millis() + 7000);
  //checkTimers();
}

void isrPCI9(){
  bays[2].incrementCycleCount();
  bays[2].setIsStuckFalse();
  bays[2].setStationTimer(millis() + 7000);
  //checkTimers();
}

void isrPCI16(){
  bays[1].incrementCycleCount();
  bays[1].setIsStuckFalse();
  bays[1].setStationTimer(millis() + 7000);
  //checkTimers();
}

//*****************************************************************************************************************************************

//store cycle counts to EEPROM before reset
//void storeCycleCounts(){
//  int station = 1;
//  for(int byteNo = 0; byteNo <=9; byteNo+=2, station++){
//    EEPROM.write(byteNo, highByte(bays[station].getCycleCount()));
//    EEPROM.write(byteNo + 1, lowByte(bays[station].getCycleCount()));
//  }
//}

//read in stored cycle counts from EEPROM and then wipe
//void readInCycleCounts(){
//  int station = 1;
//  bays[0] = StationBay();
//  for(int byteNo = 0; byteNo < 10; byteNo+=2){
//    byte high = EEPROM.read(byteNo);
//    byte low = EEPROM.read(byteNo + 1);
//    int cc = (high<<8) + low;
//
//    switch(station){
//      case(1):
//        bays[1] = StationBay(21,31,15, cc);
//        break;
//      case(2):
//        bays[2] = StationBay(20,32,14, cc);
//        break;
//      case(3):
//        bays[3] = StationBay(19,33,13, cc);
//        break;
//      case(4):
//        bays[4] = StationBay(18,34,12, cc);
//        break;
//      case(5):
//        bays[5] = StationBay(03,35,11, cc);
//        break;
//      default:
//        break;
//    }
//  }
//
//  //clears EEPROM
//  for(int i = 0; i < 1024; i++){
//    EEPROM.write(i, 0);
//  }
//  //wdt_reset();
//}

void printIPAddress(){
  //Serial.println("printing IP Address");
  if(mainLCDTimerRefresh < millis()){
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Currently Online");
    lcd.setCursor(0,1);
    lcd.print("IP Address:");
   // Serial.print("IP Address:");
    lcd.setCursor(0,2);
    for(byte thisByte = 0; thisByte < 4; thisByte++){
      lcd.print(Ethernet.localIP()[thisByte], DEC);
     // Serial.print(Ethernet.localIP()[thisByte], DEC);
     if(thisByte <= 2){
        lcd.print(".");
       // Serial.print(".");
     }
    }
    mainLCDTimerRefresh = millis() + 5000; // Why are you doing this?
  }
  else{
    return;
  }
}

void checkPower(){
  for(int i = 1; i < 6; i++){
    if(digitalRead(bays[i].getPowerPin()) == HIGH){
      bays[i].powerStatusOn();
      if(bays[i].getStationTimer() == 0){
        bays[i].setStationTimer(millis() + 7000);
      }
    }
    else{
      bays[i].powerStatusOff();
      bays[i].setStationTimer(0);
    } 
  }
}

void checkReset(){
  for(int i = 1; i < 6; i++){
    if(digitalRead(bays[i].getResetPin()) == HIGH){
      bays[i].resetCycleCount();
      bays[i].resetTimesIsStuck();
    }
  }
}

void checkTimers(){
  for(int i = 1; i < 6; i++){
    if((unsigned long) millis() > bays[i].getStationTimer() && bays[i].getPowerStatus() == 1){
       if(bays[i].getIsStuck() == 0){ // Used to run through only once.
          bays[i].incrementTimesIsStuck();
       }
       bays[i].setIsStuckTrue(); 
    } 
    else{
       bays[i].setIsStuckFalse();
    }
  }
}
