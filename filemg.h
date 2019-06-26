// Saves Data to a file
bool saveData(const char *filename, String sensor, String epochtime, String value) {

  // Open file for writing
  File file = sd.open(filename, FILE_WRITE);
  if (!file) {
    //sd.remove(filename);
    Serial.println(F("Failed to create file"));
    file.close();
    return false;
  }

  file.println( sensor + ";" + epochtime + ";" + value);
  // Close the file
  file.close();
  return true;
}

// Count Lines in file
int countline(const char *filename) {
  const int line_buffer_size = 21;
  char buffer[line_buffer_size];
  int line_number = 0;
  ifstream sdin(filename);
  ofstream sdout("new.log");

  while (sdin.getline(buffer, line_buffer_size, '\n') || sdin.gcount()) {
    int count = sdin.gcount();
//    cout << buffer << endl;
    if (count >= 13) {
    //sdout << buffer << endl;
    //cout << line_number << " --" << " (" << count << " chars): " << buffer << endl;
      line_number++;
    }
  }
  ///sdin.close();
  sdout.close();
  return line_number;
}

//------------------------------------------------------------------------------
int movelines(const char *sourcefile, const char *destinationfile, int amount) {
  const int line_buffer_size = 21;
  char buffer[line_buffer_size];
  int countLine = countline(sourcefile);
  int diff = countLine - amount;
  int line_number = 0;
  ifstream sdin(sourcefile);
  ofstream sdout(destinationfile);
 
  if (countLine > amount) {
    while (sdin.getline(buffer, line_buffer_size, '\n') || sdin.gcount()) {
      sdout << buffer << endl;
      cout << line_number << "--" << buffer << endl;
      line_number++;
      if(line_number > diff) break;
    }
      cout << divider << endl;
        sdout.close();
        ofstream sdout(sourcefile);
    while (sdin.getline(buffer, line_buffer_size, '\n') || sdin.gcount()) {
      sdout << buffer << endl;
      cout << line_number << "--" << buffer << endl;
      line_number++;
//      if(line_number > diff) break;
    }
  sdout.close();
  return diff;
  }
}

/*
// Move Lines from Source File to Destination File
int shiftLines(const char *sourcefilename, const char *destinationfilename, int amount) {
  int countLine = countline(sourcefilename);
  Serial.println(F("Lines: ") + countLine);
  delay(100);
  if (countLine > amount) {
    // Open file for reading
    File sourcefile = sd.open(sourcefilename, FILE_READ);
    if (!sourcefile) {
      return -1;
    }
    // Open file for appending
    File destinationfile = sd.open(destinationfilename, FILE_WRITE);
    if (!destinationfile) {
      return -1;
    }
    Serial.println(F("Files opend"));
    delay(100);

    int diff = countLine - amount;
    Serial.println(F("Difference: ") + diff);
    delay(100);

    for (int i = 1; i <= diff; i++) {
      String cont = sourcefile.readStringUntil('\n');
      destinationfile.print(cont);
    }
    // Open file for appending
    File tempfile = sd.open("/temp.txt", FILE_WRITE);
    if (!tempfile) {
      return -1;
    }

    for (int i = diff + 1; i <= countLine; i++) {
      String cont = sourcefile.readStringUntil('\n');
      tempfile.print(cont);
    }
    //tenpfile.flush();
    tempfile.close();
    //sourcefile.flush();
    sourcefile.close();
    //destinationfile.flush();
    destinationfile.close();
    Serial.println(F("Files closed"));
    sd.remove(sourcefilename);
    sd.rename("/temp.txt", sourcefilename);
    sd.remove("/temp.txt");
    return diff;
  }
  return -1;
}
*/
