/*
   ESP32-SimpleCLI-teraterm.ino
   This is an example code for Arduino Command Interpreter
   based on SimpleCLI by spacehuhn at https://github.com/spacehuhn/SimpleCLI

   When ESP32devkitC is used as an embeded device, USB serial port is 
   connected to Tera Term without echo back mode. 
   This program echo backs key input and waits for CR or LF to process CLI. 

    Only three commands are supported.
   ?  (for help)
  - sleep -s 3 ( wait 3 seconds)
  - reboot

  backspace key works.

   MIT License
   Copyright by coniferconifer
*/
#define SERIAL_SPEED 115200
#include <SimpleCLI.h>
SimpleCLI cli;
// Commands
Command cmdSleep;
Command cmdMAC;
Command cmdHelp;
Command cmdReboot;
#define BACKSPACE 0x08

void printHelp() {
  Serial.println();
  Serial.println("SimpleCLI demo for Tera Term");
  Serial.println(" ? for help");
  Serial.println(" sleep -s 3 ...to sleep 3sec");
  Serial.println(" reboot");
}
void setup() {
  Serial.begin(SERIAL_SPEED);
  doInitializeCLI();
}

void doInitializeCLI() {
  printHelp();
  cmdSleep = cli.addCmd("sleep");// set sleep time in sec
  cmdSleep.addArg("s"); //  sleep -s 10

  cmdReboot = cli.addCmd("reboot");//reboot
  cmdHelp = cli.addCommand("?");
  Serial.print("# ");
}
#define KEY_BUFFERSIZE 80
char inputBuffer[KEY_BUFFERSIZE];
int  inputBufferPointer = 0;
void checkCLI() {
  String input;
  if (Serial.available()) {// CR=\r LF=\n  teraterm's default is CR by return key
    char c = Serial.read();
    Serial.print(c);//echo back
    if ( inputBufferPointer < KEY_BUFFERSIZE - 2 ) { //prevent buffer overflow
      inputBuffer[inputBufferPointer++] = c;
    }
    if (c == BACKSPACE) { //Backspace key
      inputBufferPointer = inputBufferPointer - 2;
      if (inputBufferPointer < 0 ) inputBufferPointer = 0;//prevent buffer underflow
    }
    if (c == '\r' || c == '\n') {
      inputBuffer[inputBufferPointer++] = 0x00;
      input = inputBuffer;//char array to String 
      inputBufferPointer = 0;
      if (input.length() > 0) {
        Serial.print("# ");
        Serial.println(input);
        cli.parse(input);
      }
    }

    if (cli.available()) {
      Command c = cli.getCmd();
      int argNum = c.countArgs();
      Serial.print("> ");
      Serial.print(c.getName());
      Serial.print(' ');

      for (int i = 0; i < argNum; ++i) {
        Argument arg = c.getArgument(i);
        // if(arg.isSet()) {
        Serial.print(arg.toString());
        Serial.print(' ');
        // }
      }

      Serial.println();

      if (c == cmdSleep) {
        int sleepSecond = c.getArgument("s").getValue().toInt();
        Serial.println("sleep " + String(sleepSecond) );
        delay(1000 * sleepSecond);
        Serial.print("# ");
      } else if (c == cmdHelp) {
        Serial.println("Help:");
        printHelp();
        Serial.print("# ");
      } else if (c == cmdReboot) {
        Serial.println("Rebooting now");
        delay(3000);
        ESP.restart();
      }
    }

    if (cli.errored()) {
      CommandError cmdError = cli.getError();

      Serial.print("ERROR: ");
      Serial.println(cmdError.toString());

      if (cmdError.hasCommand()) {
        Serial.print("Did you mean \"");
        Serial.print(cmdError.getCommand().toString());
        Serial.println("\"?");
      }
      Serial.print("# ");
    }
  }
}
void loop() {
  // put your main code here, to run repeatedly:

  // check USB serial and command line interpreter works here
  checkCLI();
}
