float consgamma   = 2.8; // Correction factor
uint16_t max_in  = 255; // Top end of INPUT range
uint16_t max_out = 1023; // Top end of OUTPUT range
char output[10];

  // Print Gamma Correction Values
  /*  Serial.println(F("\n//---------------------------------------------------------------"));
    Serial.print(F("Gamma Correction Values:"));
    Serial.print("const uint16_t PROGMEM gamma[] = {");
    for (int i = 0; i <= max_in; i++) {
      if (i > 0) Serial.print(',');
      if ((i % 16) == 0) Serial.print("\n  ");
      sprintf(output, "%5u", (uint16_t)(pow((float)i / (float)max_in, consgamma) * max_out + 0.5));
      Serial.print(output);
    }
    Serial.println("");
    Serial.println("  };");

      Serial.println(F("\n//---------------------------------------------------------------"));
      Serial.print(F("Gamma test:"));
      for (uint16_t i = 0; i <= 255; i++) {
    //result = pgm_read_byte(&gamma8[input]);
      Serial.print(i);Serial.print(" ");
      Serial.println(pgm_read_word(&tblgamma[i]));
      }
  */
