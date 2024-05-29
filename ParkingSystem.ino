#include <SPI.h>
#include <MFRC522.h>
#include <Servo.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27,16,2); 
// Pin definitions
#define RST_PIN 9
#define SS_PIN 10
#define SERVO_PIN1 8
#define SERVO_PIN2 7
#define IR1 6
#define IR2 5


// Instances
MFRC522 mfrc522(SS_PIN, RST_PIN); // Create MFRC522 instance
Servo myservo1;
Servo myservo2;

// Key for RFID authentication
MFRC522::MIFARE_Key key;

// Block number to read/write
int blockNum = 2;
int value;

// Buffers for RFID data
byte blockData[16] = {'1'}; // Data to write (initialize with '1')
byte readBlockData[18]; // Buffer to read data into
byte bufferLen = 18; // Buffer length for reading data

MFRC522::StatusCode status;

void setup() {
  lcd.init();                    
  lcd.backlight();
  lcd.setCursor(2,0);
  lcd.print("Slot remain");
  ///
  pinMode(IR1, INPUT); // IR Sensor pin INPUT
  pinMode(IR2, INPUT); // IR Sensor pin INPUT

  // Initialize servo
  myservo1.attach(SERVO_PIN1);
  myservo2.attach(SERVO_PIN2);

  myservo1.write(60); // Initial position
  myservo2.write(150); // Initial position

  // Initialize Serial
  Serial.begin(9600);
  while (!Serial); // Wait for Serial to be ready (for ATMEGA32U4 boards)

  // Initialize SPI and MFRC522
  SPI.begin();
  mfrc522.PCD_Init();
  delay(4); // Optional delay after initialization

  //


}

void loop() {
  display();
  // Initialize RFID key
  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;
  }
  // Look for new cards

  // Check for new card presence and read its serial number
  if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
    Serial.println("\nReading from Data Block...");
    ReadDataFromBlock(blockNum, readBlockData);

    // Print the data read from block
    Serial.println();
    for (int j = 0; j < 16; j++) {
      Serial.write(readBlockData[j]);
    }
    Serial.println();

    // Convert the first byte to an integer
    memcpy(&value, &readBlockData[0], sizeof(byte));
    Serial.println(value);

    // Check specific data in the block and open gate if condition met
    if (value == 49) { // ASCII code for '1'
      opengate(SERVO_PIN1);
      blockData[0] = '0'; // Set to '0'
      WriteDataToBlock(blockNum, blockData);
    }
    if (value == 48) { // ASCII code for '0'
      opengate(SERVO_PIN2);
      blockData[0] = '1'; // Set to '1'
      WriteDataToBlock(blockNum, blockData);
    }

    mfrc522.PCD_Reset();
    delay(3000);
    SPI.begin();
    mfrc522.PCD_Init();
  }
  else{
    return;
  }
}

void entrygate()
{
  
}

void opengate(int Servopin) {
  if (Servopin == SERVO_PIN1) {
    myservo1.write(150);
    delay(1000); // Delay to prevent multiple reads
    myservo1.write(60);
    delay(1000); // Delay to prevent multiple reads
  } else {
    myservo2.write(60);
    delay(1000); // Delay to prevent multiple reads
    myservo2.write(150);
    delay(1000); // Delay to prevent multiple reads
  }
}

void ReadDataFromBlock(int blockNum, byte readBlockData[]) {
  // Authenticate desired block using Key A
  byte status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, blockNum, &key, &(mfrc522.uid));
  if (status != MFRC522::STATUS_OK) {
    Serial.print("Authentication failed for Read: ");
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }
  Serial.println("Authentication success");

  // Read data from the block
  status = mfrc522.MIFARE_Read(blockNum, readBlockData, &bufferLen);
  if (status != MFRC522::STATUS_OK) {
    Serial.print("Reading failed: ");
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }
  Serial.println("Block was read successfully");
}

void WriteDataToBlock(int blockNum, byte blockData[]) {
  // Authenticate desired block using Key A
  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, blockNum, &key, &(mfrc522.uid));
  if (status != MFRC522::STATUS_OK) {
    Serial.print("Authentication failed for Write: ");
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  } else {
    Serial.println("Authentication success");
  }

  // Write data to the block
  status = mfrc522.MIFARE_Write(blockNum, blockData, 16);
  if (status != MFRC522::STATUS_OK) {
    Serial.print("Writing to Block failed: ");
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  } else {
    Serial.println("Data was written into Block successfully");
  }
}

void display() {
  int slot = 2; // Initial slot number

  // Check the state of IR sensors
  int sensor1 = digitalRead(IR1); // Read the state of IR sensor 1
  int sensor2 = digitalRead(IR2); // Read the state of IR sensor 2

  // Update slot number based on sensor inputs
  if (sensor1 == LOW && sensor2 == HIGH) {
    // Both sensors detect an object, decrement the slot number
    slot--;
    }
  if (sensor1 == HIGH && sensor2 == LOW) {
    // Both sensors detect an object, decrement the slot number
    slot--;
  } else if (sensor1 == LOW && sensor2 == LOW) {
    // Both sensors do not detect an object, increment the slot number
    slot = 0;
  }

  // Print the updated slot number on the LCD display
  lcd.setCursor(2, 1); // Set cursor position to the second row
  lcd.print(slot); // Print the slot number
}






