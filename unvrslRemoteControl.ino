#include <SPI.h>
#include <SD.h>
#include <IRremote.h>
#include <Arduino.h>

#define LED A0
#define IR A1
#define SET 2
#define POWER 3
#define BUTTON1 4
#define BUTTON2 5
#define BUTTON3 6
#define BUTTON4 7
#define CS 10

// Variabel untuk menyimpan letak baris dari perangkat yang diinginkan
String codeDevice = "";
String codePrev = "";
String protocol;
String address;
String position;

bool haveDevice = false;

// File data
File file;

void setup() {
  // put your setup code here, to run once:
  pinMode(SET, INPUT);
  pinMode(POWER, INPUT);
  pinMode(BUTTON1, INPUT);
  pinMode(BUTTON2, INPUT);
  pinMode(BUTTON3, INPUT);
  pinMode(BUTTON4, INPUT);
  pinMode(LED, OUTPUT);
  
  IrSender.begin(IR);
  Serial.begin(9600);

  if (!SD.begin(10)) {
    Serial.println("initialization failed!");
    return;
  }
  Serial.println("initialization done.");

}

void loop() {
  // put your main code here, to run repeatedly:
  int setState = digitalRead(SET);
  if(setState == HIGH){
    getDevice();  // Kode yang dicari
    haveDevice = true;
  }
  if(codeDevice.length() == 4 && haveDevice == true){
    getPosition();
    haveDevice = false;
  }
  if(position.length() != 0){
    String cmd = whatInput();
    if(cmd != ""){
      sendInfraRed(protocol, cmd);
      delay(200);
    }
  }
}

void getDevice() {

  codeDevice = "";
  bool getChar = false;
  codePrev = codeDevice;

  for (int i = 0; i < 4; i++) {
    getChar = false;
    digitalWrite(LED, HIGH);

    while (getChar == false) {
      int bs1 = digitalRead(BUTTON1);
      int bs2 = digitalRead(BUTTON2);
      int bs3 = digitalRead(BUTTON3);
      int bs4 = digitalRead(BUTTON4);

      if (bs1 == HIGH){
        codeDevice += "A";
        getChar = true;
      }
      if (bs2 == HIGH){
        codeDevice += "B";
        getChar = true;
      } 
      if (bs3 == HIGH){
        codeDevice += "C";
        getChar = true;
      } 
      if (bs4 == HIGH){
        codeDevice += "D";
        getChar = true;
      }
      delay(300);
    }
    digitalWrite(LED, LOW);
    delay(200);
  }
  Serial.println(codeDevice);
  delay(200);
}

void getPosition(){
  bool codeFound = false;  // Variabel untuk melacak apakah kode ditemukan

  // Buka file CSV
  file = SD.open("data1.csv");

  // Periksa apakah file berhasil dibuka
  if (file) {
    Serial.println("File berhasil dibuka...");

    // Baca file baris per baris
    while (file.available()) {
      // Baca satu baris
      String line = file.readStringUntil('\n');

      // Pisahkan data dalam baris
      String code = getValue(line, ',', 0);

      // Periksa apakah kode ditemukan
      if (code.equals(codeDevice)) {
        protocol = getValue(line, ',', 3); // Indeks 3 untuk kolom protocol
        address = getValue(line, ',', 4);  // Indeks 4 untuk kolom address
        position = line;

        Serial.println("Protocol used: " + protocol);
        Serial.println("Address of Device: " + address);

        // Setel variabel codeFound menjadi true dan keluar dari loop
        codeFound = true;
        break;
      }
    }

    // Tutup file setelah selesai
    file.close();

    // Periksa apakah kode tidak ditemukan
    if (!codeFound) {
      Serial.println("Kode tidak ditemukan dalam file data1.csv");
      codeDevice = codePrev;
      codePrev = "";
    }

    delay(1000);
  } else {
    Serial.println("Gagal membuka file CSV.");
  }
}

String whatInput(){
  int ps = digitalRead(POWER);
  int bs1 = digitalRead(BUTTON1);
  int bs2 = digitalRead(BUTTON2);
  int bs3 = digitalRead(BUTTON3);
  int bs4 = digitalRead(BUTTON4);

  int column = 0;

  if (ps == HIGH){
    column = 6;
  } else if (bs1 == HIGH){
    column = 9;
  } else if (bs2 == HIGH){
    column = 10;
  } else if (bs3 == HIGH){
    column = 7;
  } else if (bs4 == HIGH){
    column = 8;
  }
  if(column != 0){
    Serial.println(column);
    String code = getCommand(column);
    column = 0;
    return code;
  } else {
    return "";
  }
}

String getCommand(int column) {
  // Buka file CSV
  File file = SD.open("data1.csv");

  // Periksa apakah file berhasil dibuka
  if (!file) {
    Serial.println("Gagal membuka file CSV untuk command");
    return "";
  }

  // Mengembalikan perintah sesuai input
  String command = getValue(position, ',', column);

  file.close();

  return command;
}

void sendInfraRed(String protocol, String command){
  uint16_t addrs = strtol(address .c_str(), NULL, 0);
  uint8_t cmd = strtol(command.c_str(), NULL, 0);

  if (protocol == "NEC"){
    IrSender.sendNEC(addrs,cmd, 2);
  } else if (protocol == "Sony"){
    IrSender.sendSony(addrs,cmd, 2, 15);
  } else if (protocol ==  "NEC2"){
    IrSender.sendNEC2(addrs,cmd, 2);
  } else if (protocol ==  "RC5"){
    IrSender.sendRC5(addrs, cmd, 2);
  } else {
    Serial.println("protocol tidak ditemukan");
  }
  Serial.println("Sinyal dikirimkan...");
}

// Fungsi untuk mendapatkan nilai dari string CSV berdasarkan indeks kolom
String getValue(String data, char separator, int index) {
  int found = 0;
  int strIndex[] = { 0, -1 };
  int maxIndex = data.length() - 1;

  for (int i = 0; i <= maxIndex && found <= index; i++) {
    if (data.charAt(i) == separator || i == maxIndex) {
      found++;
      strIndex[0] = strIndex[1] + 1;
      strIndex[1] = (i == maxIndex) ? i + 1 : i;
    }
  }

  return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}
