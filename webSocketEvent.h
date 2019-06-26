//Websocket handling
void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t lenght){
  switch(type){
    case WStype_DISCONNECTED:  
      Serial.printf("[%u] Disconnected!\n", num);
      break;
      
    case WStype_CONNECTED:{
      IPAddress ip = webSocket.remoteIP(num);
      Serial.println(ip.toString());
      //webSocket.sendTXT(num, ledstatus);
      //webSocket.sendTXT(num, "s"+ms);
      //webSocket.sendTXT(num, "b"+blinks);
    }
      break;
      
    case WStype_TEXT:{
      String text = String((char *) &payload[0]);
      Serial.println("WStype_TEXT:" + text);
     
      if(text.startsWith("Bright1")){
        ms=(text.substring(text.indexOf("Bright1")+8,text.length()));
        Serial.println(ms);
        // send data to all connected clients
        webSocket.broadcastTXT(text);
      uint16_t brightness = ms.toInt();//(uint16_t) strtol(ms, NULL, 10);
      Serial.println(brightness);
      brightness = 1024 - pgm_read_word(&tblgamma[brightness]);
      Serial.println(brightness);
      analogWrite(OnBoard_LED_Pin, brightness);
      }

      if(text.startsWith("b")){
        blinks=(text.substring(text.indexOf("b")+1,text.length()));
        Serial.println(blinks);
        // send data to all connected clients
        webSocket.broadcastTXT(text);
      }

      if(text=="LED"){
        webSocket.broadcastTXT("Start blinking.");
        for(int i=0;i<blinks.toInt()*2;i++){
          if(ledstatus == "Led off"){
            digitalWrite(OnBoard_LED_Pin,HIGH); 
            ledstatus = "Led on";       
          }
          else{
            digitalWrite(OnBoard_LED_Pin,LOW);
            ledstatus = "Led off";
          }
          delay(ms.toInt());
        }
        webSocket.broadcastTXT("Done blinking.");
        }
     if(text=="ON"){
       digitalWrite(OnBoard_LED_Pin,HIGH);
        Serial.println("Led on.");
        ledstatus = "Led on";
        // send data to all connected clients
        webSocket.broadcastTXT(ledstatus);
      }

      if(text=="OFF"){
       digitalWrite(OnBoard_LED_Pin,LOW);
        Serial.println("Led off.");
        ledstatus = "Led off";
        // send data to all connected clients
        webSocket.broadcastTXT(ledstatus);
      }
    } 
      break;
      
    case WStype_BIN:
      hexdump(payload, lenght);
      // echo data back to browser
      webSocket.sendBIN(num, payload, lenght);
      break;
  }
}
/*
void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length){
  String payloadString = (const char *)payload;
  Serial.println(payloadString);
  if(type == WStype_TEXT){
    if(payload[0] == '#'){
      uint16_t brightness = (uint16_t) strtol((const char *) &payload[1], NULL, 10);
      brightness = 1024 - brightness;
      analogWrite(OnBoard_LED_Pin, brightness);
//      Serial.print("brightness= ");
//      Serial.println(brightness);
    }
    else{
      for(int i = 0; i < length; i++)
        Serial.print((char) payload[i]);
      Serial.println();
    }
  }
}
*/
