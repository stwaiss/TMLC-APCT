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

//ResetFuntion
void(*resetFunc)(void) = 0;
//*************************************************************************************************************************************************

void setup() {
  lcd.begin(20,4);
  lcd.clear();
  
	Serial.begin(9600);
	Serial.println("Begin");

  
  PCintPort::attachInterrupt(11, stationOneLCD, RISING);   //Station 1, D11
  PCintPort::attachInterrupt(12, stationTwoLCD, RISING);   //Station 2, D12
  PCintPort::attachInterrupt(13, stationThreeLCD, RISING);   //Station 3, D13
  PCintPort::attachInterrupt(A8, stationFourLCD, RISING);   //Station 4, D14
  PCintPort::attachInterrupt(A9, stationFiveLCD, RISING); //Station 5, D15
  
  lcd.setCursor(0,0);
  lcd.print("Beginning Program");

	Serial.println(", Initializing Bays");
	bays[0] = StationBay();
	bays[1] = StationBay(A5, 24, 11);
	bays[2] = StationBay(A4, 25, 12);
	bays[3] = StationBay(A3, 26, 13);
	bays[4] = StationBay(A2, 27, A8);
	bays[5] = StationBay(A1, 28, A9);
	lcd.setCursor(0,1);
	lcd.print("Bays Initialized");
  
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


}

//*****************************************************************************************************************************************

void loop() {
  server.begin();
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
      printIPAddress();
      //wdt_reset();
      break;
    case 1: 
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Ethernet renew failed");
      ethernetCounter++;
      break;
    case 2:
      printIPAddress();
      break;
    case 3:
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Ethernet rebind fail");
      ethernetCounter++;
      break;
    case 4:
      printIPAddress();
      break;
    default:
      break;
  }
  writeToHTML();
}

//**************************************************************************************************************************************
//Pin Change Interrupts
//Cycle Counting interrupts
void stationOneLCD(){
  bays[1].incrementCycleCount();
  bays[1].setIsStuckFalse();
  bays[1].setStationTimer(millis() + 7000);
}

void stationTwoLCD(){
  bays[2].incrementCycleCount();
  bays[2].setIsStuckFalse();
  bays[2].setStationTimer(millis() + 7000);
}

void stationThreeLCD(){
  bays[3].incrementCycleCount();
  bays[3].setIsStuckFalse();
  bays[3].setStationTimer(millis() + 7000);
}

void stationFourLCD(){
  bays[4].incrementCycleCount();
  bays[4].setIsStuckFalse();
  bays[4].setStationTimer(millis() + 7000);
}

void stationFiveLCD(){
  bays[5].incrementCycleCount();
  bays[5].setIsStuckFalse();
  bays[5].setStationTimer(millis() + 7000);
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

void printIPAddress(){
  if(mainLCDTimerRefresh < millis()){
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Type IP Address Into");
    lcd.setCursor(0,1);
    lcd.print("Your Web Browser");
    lcd.setCursor(0,2);
    lcd.print("IP Address:");
    lcd.setCursor(0,3);
    for(byte thisByte = 0; thisByte < 4; thisByte++){
      lcd.print(Ethernet.localIP()[thisByte], DEC);
     if(thisByte <= 2){
        lcd.print(".");
     }
    }
    mainLCDTimerRefresh = millis() + 5000;
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
    if(analogRead(bays[i].getResetPin())*5.0/1023.0 > 4.8 && bays[i].getPowerStatus()== 0){ //Initial Check
      delay(50);                                                                            //Debounce Noise
      if(analogRead(bays[i].getResetPin())*5.0/1023.0>4.0){                                 //Secondary Check
        bays[i].resetCycleCount();
        bays[i].resetTimesIsStuck();   
      }
    }
  }
}

void checkTimers(){
  for(int i = 1; i < 6; i++){
    if((long) millis() > bays[i].getStationTimer() && bays[i].getPowerStatus() == 1){
       if(bays[i].getIsStuck() == 0){
          bays[i].incrementTimesIsStuck();
       }
       bays[i].setIsStuckTrue(); 
    } 
    else{
       bays[i].setIsStuckFalse();
    }
  }
}

//*************************************************************************************************************************************************************
  
void writeToHTML(){
  EthernetClient client = server.available();
  if (client) {
    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so you can send a reply
        if (c == '\n' && currentLineIsBlank) {
          // send a standard http response header
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println("Connection: close");  // the connection will be closed after completion of the response
          client.println();
          client.println("");
          
          client.println("<!DOCTYPE HTML>");
          
          client.println("<html>");
          
          client.println("<head>");
          client.println("<title>The Technology Testing Center (3TC)</title>");
          client.println("<META HTTP-EQUIV=\"refresh\" CONTENT=\"5\">");          //Content = Seconds before next refresh
          client.println("<link href=\"https://maxcdn.bootstrapcdn.com/bootstrap/3.3.6/css/bootstrap.min.css\" rel=\"stylesheet\" integrity=\"sha384-1q8mTJOASx8j1Au+a5WDVnPi2lkFfwwEAa8hDDdjZlpLegxhjVME1fgjWPGmkzs7\" crossorigin=\"anonymous\">");
          client.println("</head>");

          client.println("<body>");
          client.println("<div class=\"content\">");
          
          client.println("<h1> The Master Lock Company LLC</h1>");
          client.println("<h2>The Technology Testing Center (3TC)</h2>");
          client.println("<p>Automated Pneumatic Cycle Test Fixture</p>");

          //Insert table of StationBay data - Set up table headers
          client.println("<table class=\"table table-bordered table-striped table-hover\">");
          client.println("<thead>");
          client.println("<tr>");

          client.println("<th style=\"text-align:center\">Station # </th>");
          client.println("<th style=\"text-align:center\">Power Status </th>");
          client.println("<th style=\"text-align:center\">Stuck? </th>");
          client.println("<th style=\"text-align:center\">Times Stuck </th>");
          client.println("<th style=\"text-align:center\">Cycle Count </th>");
          client.println("</tr>");
          client.println("</thead>");

          client.println("<tbody>");
          //print data to table iteratively
          for (int stationNo = 1; stationNo < 6; stationNo++) {

              client.println("<tr>");           
              
              client.println("<td align = \"center\">");
              client.println(stationNo);
              client.println("</td>");
            
              //Change 0 or 1 to OFF or ON
              if(bays[stationNo].getPowerStatus() == 0){
                  client.println("<td align = \"center\"> OFF </td>");
              }else if(bays[stationNo].getPowerStatus() == 1){
                  client.println("<td align = \"center\"> ON </td>");
              }
            
              //Change 0 or 1 to NO or YES, if YES, set cell background to RED
              if(bays[stationNo].getIsStuck() == 0){
                  client.println("<td align = \"center\"> NO </td>");
              } else if(bays[stationNo].getIsStuck() == 1){
                  client.println("<td align = \"center\" BGCOLOR=\"DC143C\"> YES </td>");
                  
              }
              
              client.println("<td align = \"center\">");
              client.println(bays[stationNo].getTimesIsStuck());
              client.println("</td>");
            
              client.println("<td align = \"center\">");
              client.println(bays[stationNo].getCycleCount());
              client.println("</td>");
            
//              client.println("<td align = \"center\">");
//              client.println(bays[stationNo].getStationTimer());
//              client.println("</td>");
            
              client.println("</tr>");
          }
          client.println("</tbody>");

          client.println("</table>");
          client.println("<p>Page refreshes every ~5 seconds</p>");
          client.println("<p>Content created by Sean T. Waiss and Ernest Pazera</p>");
          
          client.println("</div>");
          client.println("</body>");
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



