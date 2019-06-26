/*
   EDB_SDCARD.pde
   Extended Database Library + External SD CARD storage demo

   Thanks to https://github.com/firebull/arduino-edb/ for the SD CARD example.

   The Extended Database library project page is here:
   http://www.arduino.cc/playground/Code/ExtendedDatabaseLibrary

*/

//#include "Arduino.h"
#include <TimeLib.h>                        //https://github.com/PaulStoffregen/Time
#include <EDB.h>
#include "SdFat.h"
#include "sdios.h"
SdFat sd;

// Use the external SPI SD card as storage
#include <SPI.h>
//#include <SD.h>

// OnBoard LED is plugged into pin 2 on the ESP8266
#define OnBoard_LED_Pin D4
#define SD_PIN D2  // SD Card CS pin
#define TABLE_SIZE 262144 //Record Limit: 16383

// The number of demo records that should be created.  This should be less
// than (TABLE_SIZE - sizeof(EDB_Header)) / sizeof(LogEvent).  If it is higher,
// operations will return EDB_OUT_OF_RANGE for all records outside the usable range.
#define RECORDS_TO_CREATE 200

const char *db_name = "edb_test.db";
File dbFile;

// Arbitrary record definition for this table.
// This should be modified to reflect your record needs.
struct LogEvent {
  long epoch;
  int sensor_t1;
  int sensor_t2;
  int sensor_d1;
} logEvent;

// The read and write handlers for using the SD Library
// Also blinks the led while writing/reading
void writer (unsigned long address, byte data) {
  digitalWrite(OnBoard_LED_Pin, HIGH);
  dbFile.seek(address);
  dbFile.write(data);
  dbFile.flush();
  digitalWrite(OnBoard_LED_Pin, LOW);
}

byte reader (unsigned long address) {
  digitalWrite(OnBoard_LED_Pin, HIGH);
  dbFile.seek(address);
  byte b = dbFile.read();
  digitalWrite(OnBoard_LED_Pin, LOW);
  return b;
}

// Create an EDB object with the appropriate write and read handlers
EDB db(&writer, &reader);

// Run the demo
void setup()
{
  pinMode(OnBoard_LED_Pin, OUTPUT);
  digitalWrite(OnBoard_LED_Pin, LOW);

  Serial.begin(115200);
  Serial.println(" Extended Database Library + External SD CARD storage demo");
  Serial.println();

  randomSeed(analogRead(0));

  if (!sd.begin(SD_PIN)) {
    Serial.println("No SD-card.");
    return;
  }

  // Check dir for db files
  if (!sd.exists("/db")) {
    Serial.println("Dir for Db files does not exist, creating...");
    sd.mkdir("/db");
  }
  sd.remove(db_name);
  if (sd.exists(db_name)) {

    dbFile = sd.open(db_name, FILE_WRITE);

    // Sometimes it wont open at first attempt, espessialy after cold start
    // Let's try one more time
    if (!dbFile) {
      dbFile = sd.open(db_name, FILE_WRITE);
    }

    if (dbFile) {
      Serial.print("Openning current table... ");
      EDB_Status result = db.open(0);
      if (result == EDB_OK) {
        Serial.println("DONE");
      } else {
        Serial.println("ERROR");
        Serial.println("Did not find database in the file " + String(db_name));
        Serial.print("Creating new table... ");
        db.create(0, TABLE_SIZE, (unsigned int)sizeof(logEvent));
        Serial.println("DONE");
        return;
      }
    } else {
      Serial.println("Could not open file " + String(db_name));
      return;
    }
  } else {
    Serial.print("Creating table... ");
    // create table at with starting address 0
    dbFile = sd.open(db_name, FILE_WRITE);
    db.create(0, TABLE_SIZE, (unsigned int)sizeof(logEvent));
    Serial.println("DONE");
  }
  sd.ls(LS_R | LS_DATE | LS_SIZE);
  recordLimit();
  countRecords();
  createRecords(RECORDS_TO_CREATE);
  countRecords();
  sd.ls(LS_R | LS_DATE | LS_SIZE);
  delay(1000);
  selectAll();
  deleteOneRecord(RECORDS_TO_CREATE / 2);
  countRecords();
  selectAll();
  appendOneRecord(RECORDS_TO_CREATE + 1);
  countRecords();
  selectAll();
  insertOneRecord(RECORDS_TO_CREATE / 2);
  countRecords();
  selectAll();
  updateOneRecord(RECORDS_TO_CREATE);
  selectAll();
  countRecords();
  deleteAll();
  Serial.println("Use insertRec() and deleteRec() carefully, they can be slow");
  countRecords();
  for (int i = 1; i <= 20; i++) insertOneRecord(1);  // inserting from the beginning gets slower and slower
  selectAll();
  countRecords();
  for (int i = 1; i <= 20; i++) deleteOneRecord(1);  // deleting records from the beginning is slower than from the end
  countRecords();
  Serial.println("DONE!");

  dbFile.close();
}

void loop()
{
}

// utility functions

void recordLimit()
{
  Serial.print("Record Limit: ");
  Serial.println(db.limit());
}

void deleteOneRecord(int recno)
{
  Serial.print("Deleting recno: ");
  Serial.println(recno);
  db.deleteRec(recno);
}

void deleteAll()
{
  Serial.print("Truncating table... ");
  db.clear();
  Serial.println("DONE");
}

void countRecords()
{
  Serial.print("Record Count: ");
  Serial.println(db.count());
}

void createRecords(int num_recs)
{
  Serial.print("Creating Records... ");
  for (int recno = 1; recno <= num_recs; recno++)
  {
    logEvent.epoch = now();
    logEvent.sensor_t1 = random(-127, 150);
    logEvent.sensor_t2 = random(-127, 150);
    logEvent.sensor_d1 = random(0, 2);
    EDB_Status result = db.appendRec(EDB_REC logEvent);
    if (result != EDB_OK) printError(result);
  }
  Serial.println("DONE");
}

void selectAll()
{
  for (int recno = 1; recno <= db.count(); recno++)
  {
    EDB_Status result = db.readRec(recno, EDB_REC logEvent);
    if (result == EDB_OK)
    {
      Serial.print("Recno: ");
      Serial.print(recno);
      Serial.print(" epoch: ");
      Serial.print(logEvent.epoch);
      Serial.print(" sensor_t1: ");
      Serial.print(logEvent.sensor_t1);
      Serial.print(" sensor_t2: ");
      Serial.print(logEvent.sensor_t2);
      Serial.print(" sensor_d1: ");
      Serial.println(logEvent.sensor_d1);

    }
    else printError(result);
  }
}

void updateOneRecord(int recno)
{
  Serial.print("Updating record at recno: ");
  Serial.print(recno);
  Serial.print("... ");
  logEvent.epoch = now();
  logEvent.sensor_t1 = 0;
  logEvent.sensor_t2 = 0;
  logEvent.sensor_d1 = 0;
  EDB_Status result = db.updateRec(recno, EDB_REC logEvent);
  if (result != EDB_OK) printError(result);
  Serial.println("DONE");
}

void insertOneRecord(int recno)
{
  Serial.print("Inserting record at recno: ");
  Serial.print(recno);
  Serial.print("... ");
  logEvent.epoch = now();
  logEvent.sensor_t1 = random(-127, 150);
  logEvent.sensor_t2 = random(-127, 150);
  logEvent.sensor_d1 = random(0, 2);
  EDB_Status result = db.insertRec(recno, EDB_REC logEvent);
  if (result != EDB_OK) printError(result);
  Serial.println("DONE");
}

void appendOneRecord(int id)
{
  Serial.print("Appending record... ");
  logEvent.epoch = now();
  logEvent.sensor_t1 = random(-127, 150);
  logEvent.sensor_t2 = random(-127, 150);
  logEvent.sensor_d1 = random(0, 2);
  EDB_Status result = db.appendRec(EDB_REC logEvent);
  if (result != EDB_OK) printError(result);
  Serial.println("DONE");
}

void printError(EDB_Status err)
{
  Serial.print("ERROR: ");
  switch (err)
  {
    case EDB_OUT_OF_RANGE:
      Serial.println("Recno out of range");
      break;
    case EDB_TABLE_FULL:
      Serial.println("Table full");
      break;
    case EDB_OK:
    default:
      Serial.println("OK");
      break;
  }
}
