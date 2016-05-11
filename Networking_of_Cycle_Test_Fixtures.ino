  /* Networking of Pneumatic Cycle Tester for Arduino
  *  This file runs Interrupt Service Routines and Ethernet platform for the
  *  Automated Pnuematic Cycle Tester at The Master Lock Company LLC, Technology Testing Center
  *  Oak Creek, Wisconsin.
  *
  *  Any replication of this code without written consent from the The Master Lock Company LLC is strictly prohibited
  *
  *  Author : Sean Waiss
  *  Date : 4/18/2016
  */

#include <LiquidCrystal.h>
#include <EEPROM.h>
#include <Ethernet.h>
#include <SPI.h>
#include <avr/interrupt.h>
#include <StationBay.h>
//#include <avr/wdt.h>

StationBay bays[6];
byte mac[] = {0x00, 0xAA, 0xBB, 0xCC, 0xDE, 0x02};
EthernetClient client;
EthernetServer server(80);
LiquidCrystal lcd(9,8,7,6,5,4);
int mainLCDTimerRefresh = 0;

//ResetFuntion
void(*resetFunc)(void) = 0;

//ISR(WDT_vect){
//  lcd.clear();
//  lcd.setCursor(0,0);
//  lcd.print("About to Reset");
//  lcd.setCursor(0,1);
//  lcd.print("Saving Cycle counts");
//  for(int i = 0; i < 500; i++){
//    
//  }
//  storeCycleCounts();
//  resetFunc();
//}

//*************************************************************************************************************************************************

void setup() {
	Serial.begin(9600);
	Serial.println("Begin");
	noInterrupts();

  // //Enable External Interrupts 1 - 5 for power buttons
  EIMSK |= (1<<INT0);
  EICRA |= (1<<ISC00);
  EIMSK |= (1<<INT1);    //Sets External Interrupt Mask to one for INT1 (Enables)
  EICRA |= (1<<ISC10);   //Sets edge detection bit to change
  EIMSK |= (1<<INT2);
  EICRA |= (1<<ISC20);
  EIMSK |= (1<<INT3);
  EICRA |= (1<<ISC30);
  EIMSK |= (1<<INT5);
  EICRB |= (1<<ISC50);

  //Enable all ports for Pin Change Interrupt
  //PCICR |= 0b00000111;
  PCICR |= (1<<PCIE2);
  PCICR |= (1<<PCIE1);
  PCICR |= (1<<PCIE0);
  
  PCMSK2 |= (1<<PCINT23);
  PCMSK2 |= (1<<PCINT22);
  PCMSK2 |= (1<<PCINT21);
  PCMSK2 |= (1<<PCINT20);
  PCMSK2 |= (1<<PCINT19);
  PCMSK1 |= (1<<PCINT10);
  PCMSK1 |= (1<<PCINT9);
  PCMSK0 |= (1<<PCINT7);
  PCMSK0 |= (1<<PCINT6);
  PCMSK0 |= (1<<PCINT5);

  Serial.println("Completed Masking");

  delay(1000);
  lcd.begin(20,4);
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Beginning Program");
  for(int i = 0; i < 1024; i++){
	if(EEPROM.read(i) == 1){
	  readInCycleCounts();
	  lcd.setCursor(0,1);
	  lcd.print("Reading in counts");
	  delay(3000);
	} 
  }
  if(EEPROM.read(0) == 0){
	  Serial.print("No EEPROM, Initializing Bays");
	  bays[0] = StationBay();
	  bays[1] = StationBay(15, 21, A15);
	  bays[2] = StationBay(14, 20, A14);
	  bays[3] = StationBay(13, 19, A13);
	  bays[4] = StationBay(12, 18, A12);
	  bays[5] = StationBay(11, 03, A11);
	  lcd.setCursor(0,1);
	  lcd.print("Bays Initialized");
  }
  
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

  interrupts();

  //MCUSR = 0; //reset the status register of the MCU
  //WDTCSR |= ((1<<WDCE) | (1<<WDE));
  //// set the "Interrupt Mode" with a timeout of 8 sec
  //WDTCSR = ((1<<WDIE)| (1<<WDP3) | (1<<WDP0));
  //SREG |= (1<<SREG_I); //re-enable global interrupts
}

//*****************************************************************************************************************************************

void loop() {
  server.begin();
  int ethernetCounter = 0;
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
      Serial.print("In case 0");
      printIPAddress();
      //wdt_reset();
      break;
    case 1:
      Serial.print("In case 1");    
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Ethernet renew failed");
      ethernetCounter++;
      break;
    case 2:
     Serial.print("In case 2");
      printIPAddress();
      //wdt_reset();
      break;
    case 3:
      Serial.print("In case 3");
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Ethernet rebind fail");
      ethernetCounter++;
      break;
    case 4:
	  Serial.print("In case 4");
      printIPAddress();
      //wdt_reset();
      break;
    default:
      break;
  }
  writeToHTML();
}

//**************************************************************************************************************************************

//Pin Change Interrupt Service Routines
//LCD Interrupts
ISR(PCINT5_vect){
  bays[5].incrementCycleCount();
  bays[5].setIsStuckFalse();
  bays[5].setStationTimer(millis() + 7000);
  checkTimers();
}
ISR(PCINT6_vect){
  bays[4].incrementCycleCount();
  bays[4].setIsStuckFalse();
  bays[4].setStationTimer(millis() + 7000);
  checkTimers();
}
ISR(PCINT7_vect){
  bays[3].incrementCycleCount();
  bays[3].setIsStuckFalse();
  bays[3].setStationTimer(millis() + 7000);
  checkTimers();
}
ISR(PCINT9_vect){
  bays[2].incrementCycleCount();
  bays[2].setIsStuckFalse();
  bays[2].setStationTimer(millis() + 7000);
  checkTimers();
}
ISR(PCINT10_vect){
  bays[1].incrementCycleCount();
  bays[1].setIsStuckFalse();
  bays[1].setStationTimer(millis() + 7000);
  checkTimers();
}

//Reset Button Interrupts
ISR(PCINT19_vect){
  bays[5].resetCycleCount();
  bays[5].resetTimesIsStuck();
}
ISR(PCINT20_vect){
  bays[4].resetCycleCount();
  bays[4].resetTimesIsStuck();
}
ISR(PCINT21_vect){
  bays[3].resetCycleCount();
  bays[3].resetTimesIsStuck();
}
ISR(PCINT22_vect){
  bays[2].resetCycleCount();
  bays[2].resetTimesIsStuck();
}
ISR(PCINT23_vect){
  bays[1].resetCycleCount();
  bays[1].resetTimesIsStuck();
}

//External Interrupt Service Routines 
//Power Toggle Button Interrupts
ISR(INT0_vect) {
	if (bays[1].getPowerStatus() == 0) {
		bays[1].powerStatusOn();
		bays[1].setStationTimer(millis() + 7000);
	}
	else {
		bays[1].powerStatusOff();
	}
}
ISR(INT1_vect){
	if (bays[2].getPowerStatus() == 0){
		bays[2].powerStatusOn();
		bays[2].setStationTimer(millis() + 7000);
	}else{
		bays[2].powerStatusOff();
	}
}
ISR(INT2_vect){
	 if(bays[3].getPowerStatus() == 0){
		bays[3].powerStatusOn();
		bays[3].setStationTimer(millis() + 7000);
	 }else{
		bays[3].powerStatusOff();
	 }
}
ISR(INT3_vect){
	if (bays[4].getPowerStatus() == 0) {
		bays[4].powerStatusOn();
		bays[4].setStationTimer(millis() + 7000);
	}
	else {
		bays[4].powerStatusOff();
	}
}
ISR(INT5_vect){
	if (bays[5].getPowerStatus() == 0) {
		bays[5].powerStatusOn();
		bays[5].setStationTimer(millis() + 7000);
	}
	else {
		bays[5].powerStatusOff();
	}
}

//*****************************************************************************************************************************************

//store cycle counts to EEPROM before reset
void storeCycleCounts(){
  int station = 1;
  for(int byteNo = 0; byteNo <=9; byteNo+=2, station++){
    EEPROM.write(byteNo, highByte(bays[station].getCycleCount()));
    EEPROM.write(byteNo + 1, lowByte(bays[station].getCycleCount()));
  }
}

//read in stored cycle counts from EEPROM and then wipe
void readInCycleCounts(){
  int station = 1;
  bays[0] = StationBay();
  for(int byteNo = 0; byteNo < 10; byteNo+=2){
    byte high = EEPROM.read(byteNo);
    byte low = EEPROM.read(byteNo + 1);
    int cc = (high<<8) + low;

    switch(station){
      case(1):
        bays[1] = StationBay(15,21,A15, cc);
        break;
      case(2):
        bays[2] = StationBay(14,20,A14, cc);
        break;
      case(3):
        bays[3] = StationBay(13,19,A13, cc);
        break;
      case(4):
        bays[4] = StationBay(12,18,A12, cc);
        break;
      case(5):
        bays[5] = StationBay(11,03,A11, cc);
        break;
      default:
        break;
    }
  }

  //clears EEPROM
  for(int i = 0; i < EEPROM.length(); i++){
    EEPROM.write(i, 0);
  }
  //wdt_reset();
}

void printIPAddress(){
  Serial.println("printing IP Address");
  if(mainLCDTimerRefresh < millis()){
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Currently Online");
    lcd.setCursor(0,1);
    lcd.print("IP Address:");
    Serial.print("IP Address:");
    lcd.setCursor(0,2);
    for(byte thisByte = 0; thisByte < 4; thisByte++){
      lcd.print(Ethernet.localIP()[thisByte], DEC);
      Serial.print(Ethernet.localIP()[thisByte], DEC);
     if(thisByte <= 2){
        lcd.print(".");
        Serial.print(".");
     }
    }
    mainLCDTimerRefresh = millis() + 5000;
  }
  else{
    return;
  }
}

void checkTimers(){
  for(int i = 1; i < 6; i++){
    if(millis() > bays[i].getStationTimer() && bays[i].getPowerStatus() == 1){
       if(bays[i].getIsStuck() == 0){
          bays[i].incrementTimesIsStuck();
       }
       bays[i].setIsStuckTrue(); 
    } else{
       bays[i].setIsStuckFalse();
    }
  }
}

void writeToHTML(){
  EthernetClient client = server.available();
  if (client) {
    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        Serial.write(c);
        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so you can send a reply
        if (c == '\n' && currentLineIsBlank) {
          // send a standard http response header
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println("Connection: close");  // the connection will be closed after completion of the response
          client.println();
          client.println("<!DOCTYPE HTML>");
          client.println("<html>");
          client.println("<head>");
          client.println("<b><i><font size =\"12\"> The Master Lock Company LLC</font></b></i>");
          client.println("<br/>");
          client.println("The Technology Testing Center (3TC)");
          client.println("<br/>");
          client.println("Automated Pneumatic Cycle Test Fixture");
          client.println("</head><META HTTP-EQUIV=\"refresh\" CONTENT=\"1\">");
          client.println("<br/>");
          client.println("<br/>");
		  String milliseconds = String(millis());
		  client.println(milliseconds + "<br/>");
          
          //Insert table of StationBay data - Set up table headers
          client.println("<table border = \"2\" style=\"width:100%>\"");
          client.println("<th></th>");
          client.println("<th>Station # </th>");
          client.println("<th>Power Status </th>");
          client.println("<th>Stuck? </th>");
          client.println("<th>Times Stuck </th>");
          client.println("<th>Cycle Count </th>");
		  client.println("<th>Station Timer </th>");

          //print data to table iteratively
          for (int stationNo = 1; stationNo < 6; stationNo++) {
            String stationNumber = String(stationNo);
            String cycleCount = String(bays[stationNo].getCycleCount());
			String stationTimer = String(bays[stationNo].getStationTimer());
            String timesIsStuck = String(bays[stationNo].getTimesIsStuck());
            client.println("<tr>");           
            client.println("<td>" + stationNumber + "</td>");
            
            //Change 0 or 1 to OFF or ON
            if(bays[stationNo].getPowerStatus() == 0){
              client.println("<td> OFF </td>");
            } else if(bays[stationNo].getPowerStatus() == 1){
               client.println("<td> ON </td>");
            }
            
            //Change 0 or 1 to NO or YES, if YES, set cell background to RED
            if(bays[stationNo].getIsStuck() == 0){
              client.println("<td> NO </td>");
            } else if(bays[stationNo].getIsStuck() == 1){
               client.println("<td BGCOLOR=\"DC143C\"> YES </td>");
            }
            client.println("<td>" + timesIsStuck + "</td>");
            client.println("<td>" + cycleCount + "</td>");
			client.println("<td>" + stationTimer + "</td>");
            client.println("</tr>");
          }
          client.println("</table>");
          client.println("</html>");
          break;
        }
        if (c == '\n') {
          // you're starting a new line
          currentLineIsBlank = true;
        } else if (c != '\r') {
          // you've gotten a character on the current line
          currentLineIsBlank = false;
        }
      }
    }
    // give the web browser time to receive the data
    delay(1);
    // close the connection:
    client.stop();
   //wdt_reset();
  }
}
