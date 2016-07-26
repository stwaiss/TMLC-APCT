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
          client.println("");
          
          client.println("<!DOCTYPE HTML>");
          
          client.println("<html>");
          
          client.println("<head>");
          client.println("<title>The Technology Testing Center (3TC)</title>");
          client.println("<META HTTP-EQUIV=\"refresh\" CONTENT=\"1\">");
          client.println("<link href=\"https://maxcdn.bootstrapcdn.com/bootstrap/3.3.6/css/bootstrap.min.css\" rel=\"stylesheet\" integrity=\"sha384-1q8mTJOASx8j1Au+a5WDVnPi2lkFfwwEAa8hDDdjZlpLegxhjVME1fgjWPGmkzs7\" crossorigin=\"anonymous\">");
          client.println("</head>");

          client.println("<body>");
          client.println("<div class=\"content\">");
          
          client.println("<h1> The Master Lock Company LLC</h1>");
          client.println("<h2>The Technology Testing Center (3TC)</h2>");
          client.println("<p>Automated Pneumatic Cycle Test Fixture</p>");
          
          String milliseconds = String(millis());
          client.println("<p>" + milliseconds + "</p>");

          //Insert table of StationBay data - Set up table headers
          client.println("<table class=\"table table-bordered table-striped table-hover\">");
          client.println("<thead>");
          client.println("<tr>");

          client.println("<th>Station # </th>");
          client.println("<th>Power Status </th>");
          client.println("<th>Stuck? </th>");
          client.println("<th>Times Stuck </th>");
          client.println("<th>Cycle Count </th>");
          client.println("<th>Station Timer </th>");

          client.println("</tr>");
          client.println("</thead>");

          client.println("<tbody>");
          //print data to table iteratively
          for (int stationNo = 1; stationNo < 6; stationNo++) {

              client.println("<tr>");           
              
              client.println("<td>");
              client.println(stationNo);
              client.println("</td>");
            
              //Change 0 or 1 to OFF or ON
              if(bays[stationNo].getPowerStatus() == 0){
                  client.println("<td> OFF </td>");
              }else if(bays[stationNo].getPowerStatus() == 1){
                  client.println("<td> ON </td>");
              }
            
              //Change 0 or 1 to NO or YES, if YES, set cell background to RED
              if(bays[stationNo].getIsStuck() == 0){
                  client.println("<td> NO </td>");
              } else if(bays[stationNo].getIsStuck() == 1){
                  //client.println("<td BGCOLOR=\"DC143C\"> YES </td>");
                  
                  client.println("<td> YES </td>");
              }
              
              client.println("<td>");
              client.println(bays[stationNo].getTimesIsStuck());
              client.println("</td>");
            
              client.println("<td>");
              client.println(bays[stationNo].getCycleCount());
              client.println("</td>");
            
              client.println("<td>");
              client.println(bays[stationNo].getStationTimer());
              client.println("</td>");
            
              client.println("</tr>");
          }
          client.println("</tbody>");

          client.println("</table>");
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
