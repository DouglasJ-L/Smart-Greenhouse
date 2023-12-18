#include "system_sam3x.h"
#include "at91sam3x8.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>      // Standard dir for string
#include <time.h>        // Standard dir for time
#include "display.h"     // Dir for all display commands (lab2)
#include "temperature.h" // Dir for all temp commands (lab3)
#include "Linked_list.h" // dir for linked_list (Software lab2)
#include "sensor.h"      // dir for sensor (lab3)

#define AT91C_PIOD_AIMER (AT91_CAST(AT91_REG *) 0x400E14B0)
#define AT91C_PIOD_DIFSR (AT91_CAST(AT91_REG *) 0x400E1484)
#define AT91C_PIOD_SCDR (AT91_CAST(AT91_REG *) 0x400E148C)
#define AT91C_PWMC_CMRx (AT91_CAST(AT91_REG *) 0x4009422C)

void SysTick_Handler(void);
void Config();
void timeConfig(void);
void write_date_and_time(void);
char int_to_hex(int i);
char char_to_hex(char c);
int readKeypad(void);
void sysTickOnOff(int onOff);
void fastModeOnOff(void);
void GUI(void);
void timeConfigUser(void);

int sysTickStatus = 0;
extern double temperature; // global variables from temp func
extern volatile int flag;
extern volatile int flag2; 
extern int del, tdel;
double maxValue = 0;
extern double minValue;
extern float volt1;
extern float volt2;
extern float light;
short lastlight = 0;
short minlight;
int bestAngle = 0;
char lastAngle = 0;
unsigned int led = 0;
int sunflag = 0;
int LEDflag = 0;
int darkflag = 0;
double upperTempLimit = 25.0; 
double lowerTempLimit = 20.0;

int year;
int month;
int day;
int hour;
int minute;
int second;

int value;
int column [] = {256, 128, 512}; // R1 = 256 (PC7), R2 = 128 (PC8), R3 = 512 (PC9) 
int row [] = {32, 4, 8, 16}; // R1 = 32 (PC5), R2 = 4 (PC2), R3 = 8 (PC3), R4 = 16 (PC4)

int fastMode;
int sysTickDelay;

void Set_Led(unsigned int nLed){ // turn on/off LED
  if (nLed == 0){              // If int nled is 0
    *AT91C_PIOD_CODR = (1<<1); // Clear output data register
  }
  else{
    *AT91C_PIOD_SODR = (1<<1);  // Set output data register
  }
}

void Config(){  //Configurations
  SysTick_Config(SystemCoreClock/1000); //Interrupt every milliseconds for temp
  timeConfig();
  
  *AT91C_PMC_PCER = (1<<14); // Enables PIOD
  *AT91C_PMC_PCER = (1<<13); // Enables PIOC 
  
  //Enable all pins
  *AT91C_PIOD_OER = (1<<1); // Enable pin output
  *AT91C_PIOD_PER = (1<<1); // Enable Pin 26 led
  *AT91C_PIOC_PER = (1<<4); //R4 (P36)
  *AT91C_PIOC_PER = (1<<2); //R2 (P34)
  *AT91C_PIOC_PER = (1<<3); //R3 (P35)
  *AT91C_PIOC_PER = (1<<5); //R1 (P37)
  *AT91C_PIOC_PER = (1<<7); //C3 (P39)
  *AT91C_PIOC_PER = (1<<8); //C1 (P40)
  *AT91C_PIOC_PER = (1<<9); //C2 (P41)
  
  //Disable internal resistor for all keypad-pins
  *AT91C_PIOD_PPUDR = (1<<1); // Disable pullup register led
  *AT91C_PIOC_PPUDR = (1<<4); //R4 (P36)
  *AT91C_PIOC_PPUDR = (1<<3); //R3 (P35)
  *AT91C_PIOC_PPUDR = (1<<2); //R2 (P34)
  *AT91C_PIOC_PPUDR = (1<<5); //R1 (P37)
  *AT91C_PIOC_PPUDR = (1<<7); //C3 (P39)
  *AT91C_PIOC_PPUDR = (1<<8); //C1 (P40)
  *AT91C_PIOC_PPUDR = (1<<9); //C2 (P41)
  // for LED 
  *AT91C_PIOD_PER = (1<<3);     //Enable pin for OE
  *AT91C_PIOD_OER = (1<<3);     //Enable output for OE
  *AT91C_PIOD_PPUDR = (1<<3); // Disable pullup register
  
 //*((volatile unsigned int *) 0xE000E010) = (0 << 0); //denna fucka upp temp 
}

void timeConfig(void){  //Configures the time to use the acurate real time
struct tm * myTime;   //Struct for usage of time.h. mytime=localtime
  time_t t;           //variabel for time_t
  t = time(NULL);     // null pointer constant
  myTime = localtime(&t); // Sets mytime variabel to localtime which points to t
  
  year = myTime->tm_year + 1900;//Set the year to current year, it counts from when the clk was initiated
  month = myTime->tm_mon + 1;   //Set month to current month
  day = myTime->tm_mday;        //Set day to current day
  hour = myTime->tm_hour + 1;   //Set hour to current hour
  minute = myTime->tm_min;      //Set minute to current minute
  second = myTime->tm_sec;      //Set second to current second
}

void tempAlarm(int lowHigh){ //The alarm for temp
  char str[] = "Press and hold 7 to return to program";
  int length = strlen(str);
  
  char strHigh[] = "The temperature is high!!";
  int lengthHigh = strlen(strHigh);
  
  char strLow[] = "The temperature is low!!";
  int lengthLow = strlen(strLow);
  
  char strTemp1[] = "The latest temperature readingwas: ";
  int lengthTemp1 = strlen(strTemp1);
  
  char strTempMin[] = "The minimum temperature is:   ";
  int lengthTempMin = strlen(strTempMin);
  
  char strTempMax[] = "The maximum temperature is:   ";
  int lengthTempMax = strlen(strTempMax);
  
  if (lowHigh == 0){ //Temperature was to low
    while (1){
      if (readKeypad() == 7){   //Check if 7 is pressed (return)
        temperature = lowerTempLimit;   //Set temperature to lower limit (to ensure we don't trigger alarm emediatly again)
        return;
      }
      Delay(5000000);   //Delay used for blinking of text
      Write_Data_2_Display(0x00);
      Write_Data_2_Display(0x00);
      Write_Command_2_Display(0x24);    //Set cursor
      
      for (int i = 0; i < lengthLow; i++){
        Write_Data_2_Display(char_to_hex(strLow[i]));
        Write_Command_2_Display(0xC0);  //Print: The temperature is low!!
      }
      
      Write_Data_2_Display(0x3C);
      Write_Data_2_Display(0x00);
      Write_Command_2_Display(0x24);    //Set cursor
      
      for (int i = 0; i < lengthTemp1; i++){
        Write_Data_2_Display(char_to_hex(strTemp1[i]));
        Write_Command_2_Display(0xC0);  //Print: The latest temperature reading was: 
      }
      
      printDigit(temperature);  //Print the temperature
      
      Write_Data_2_Display(0x96);
      Write_Data_2_Display(0x00);
      Write_Command_2_Display(0x24);    //Set cursor
      
      for (int i = 0; i < lengthTempMin; i++){
        Write_Data_2_Display(char_to_hex(strTempMin[i]));
        Write_Command_2_Display(0xC0);  //Print: The minimum temperature is:   
      }
      
      printDigit(lowerTempLimit);       //Print the lower temp limit
      
      Write_Data_2_Display(0xF0);
      Write_Data_2_Display(0x00);
      Write_Command_2_Display(0x24);    //Set cursor
      
      for (int i = 0; i < length; i++){
        Write_Data_2_Display(char_to_hex(str[i]));
        Write_Command_2_Display(0xC0);  //Print: Press and hold 7 to return to program
      }
      
      Delay(5000000);   //Delay used for blinking of text
      
      Clear_Display();  //Clear the display
    }
  } else if (lowHigh == 1){ //Temperature was to high
    while (1){
      if (readKeypad() == 7){   //Check if 7 is pressed (return)
        temperature = lowerTempLimit;   //Set temperature to lower limit (to ensure we don't trigger alarm emediatly again)
        return;
      }
      Delay(5000000);   //Delay used for blinking of text
      Write_Data_2_Display(0x00);
      Write_Data_2_Display(0x00);
      Write_Command_2_Display(0x24);    //Set cursor
      
      for (int i = 0; i < lengthHigh; i++){
        Write_Data_2_Display(char_to_hex(strHigh[i]));
        Write_Command_2_Display(0xC0);  //Print:The temperature is high!!
      }
      
      Write_Data_2_Display(0x3C);
      Write_Data_2_Display(0x00);
      Write_Command_2_Display(0x24);    //Set cursor
      
      for (int i = 0; i < lengthTemp1; i++){
        Write_Data_2_Display(char_to_hex(strTemp1[i]));
        Write_Command_2_Display(0xC0);  //Print: The latest temperature reading was: 
      }
      
      printDigit(temperature);  //Print the temperature
      
      Write_Data_2_Display(0x96);
      Write_Data_2_Display(0x00);
      Write_Command_2_Display(0x24);    //Set cursor
      
      for (int i = 0; i < lengthTempMax; i++){
        Write_Data_2_Display(char_to_hex(strTempMax[i]));
        Write_Command_2_Display(0xC0);  //Print: The maximum temperature is:   
      }
      
      printDigit(upperTempLimit);       //Print the upper limit temperature
      
      Write_Data_2_Display(0xF0);
      Write_Data_2_Display(0x00);
      Write_Command_2_Display(0x24);    //Set cursor
      
      for (int i = 0; i < length; i++){
        Write_Data_2_Display(char_to_hex(str[i]));
        Write_Command_2_Display(0xC0);  //Print: Press and hold 7 to return to program
      }
      
      Delay(5000000);   //Delay used for blinking of text
      
      Clear_Display();  //Clear the display
    }
  }
}

void tempAlarmConfig(char strL[], char strH[], int lengthL, int lengthH){ //Configure low and high temp
  int number;
  
  char str[] = "Not accurate limits!";
  int lengthstr = strlen(str);
  
  lowerTempLimit = 0.0;
  upperTempLimit = 0.0;
  
  Clear_Display();
  
  Write_Data_2_Display(0x2C);
  Write_Data_2_Display(0x01);
  Write_Command_2_Display(0x24);        //Set cursor
      
  for (int i = 0; i < lengthL; i++){
    Write_Data_2_Display(char_to_hex(strL[i]));
    Write_Command_2_Display(0xC0);      //Print text to screen
  }
  
  printDigit(lowerTempLimit);   //Print lower temp limit
      
  Write_Data_2_Display(0x4A);
  Write_Data_2_Display(0x01);
  Write_Command_2_Display(0x24);        //Set cursor
      
  for (int i = 0; i < lengthH; i++){
    Write_Data_2_Display(char_to_hex(strH[i]));
    Write_Command_2_Display(0xC0);      //Print text to screen
  }
  
  printDigit(upperTempLimit);   //Print upper temp limit
  
  Delay(2000000);       //Delay to ensure accurate keypad press
  
  while (1){    //Wait untill no button is pressed
    number = readKeypad();
    if (number == 0){
      break;
    }
  }
  
  //Lower limit
  
  for (int i = 0; i < 7; i++){
    number = readKeypad();
    while (number == 0 || number == 12 || number == 10){
      number = readKeypad();
    }
    
    if (i == 0){        //Each case is to know what position of the number we are choosing
      if (number == 11){
        lowerTempLimit = 0.0;
      } else {
        lowerTempLimit = number * 10;
      }
    }
    
    if (i == 1){
      if (number == 11){
        lowerTempLimit += 0.0;
      } else {
        lowerTempLimit += number + 0.0;
      }
    }
    
    if (i == 2){
      if (number == 11){
        lowerTempLimit += 0.0;
      } else {
        lowerTempLimit += number * 0.1;
      }
    }
    
    if (i == 3){
      if (number == 11){
        lowerTempLimit += 0.0;
      } else {
        lowerTempLimit += number * 0.01;
      }
    }
    
    if (i == 4){
      if (number == 11){
        lowerTempLimit += 0.0;
      } else {
        lowerTempLimit += number * 0.001;
      }
    }
    
    if (i == 5){
      if (number == 11){
        lowerTempLimit += 0.0;
      } else {
        lowerTempLimit += number * 0.0001;
      }
    }
    
    if (i == 6){
      if (number == 11){
        lowerTempLimit += 0.0;
      } else {
        lowerTempLimit += number * 0.00001;
      }
    }
    
    Delay(2000000);                     //Delay to ensure we can choose each digit seperatly
    Write_Data_2_Display(0x3E);
    Write_Data_2_Display(0x01);
    Write_Command_2_Display(0x24);      //Set cursor
    printDigit(lowerTempLimit); //Print the lower limit
  }
  
  //Upper limit
  
  for (int i = 0; i < 7; i++){
    number = readKeypad();
    while (number == 0 || number == 12 || number == 10){
      number = readKeypad();
    }
    
    if (i == 0){        //Each case is to know what position of the number we are choosing
      if (number == 11){
        upperTempLimit = 0;
      } else {
        upperTempLimit = number * 10;
      }
    }
    
    if (i == 1){
      if (number == 11){
        upperTempLimit += 0.0;
      } else {
        upperTempLimit += number + 0.0;
      }
    }
    
    if (i == 2){
      if (number == 11){
        upperTempLimit += 0.0;
      } else {
        upperTempLimit += number * 0.1;
      }
    }
    
    if (i == 3){
      if (number == 11){
        upperTempLimit += 0.0;
      } else {
        upperTempLimit += number * 0.01;
      }
    }
    
    if (i == 4){
      if (number == 11){
        upperTempLimit += 0.0;
      } else {
        upperTempLimit += number * 0.001;
      }
    }
    
    if (i == 5){
      if (number == 11){
        upperTempLimit += 0.0;
      } else {
        upperTempLimit += number * 0.0001;
      }
    }
    
    if (i == 6){
      if (number == 11){
        upperTempLimit += 0.0;
      } else {
        upperTempLimit += number * 0.00001;
      }
    }
    
    Delay(2000000);                     //Delay to ensure we can choose each digit seperatly
    Write_Data_2_Display(0x5C);
    Write_Data_2_Display(0x01);
    Write_Command_2_Display(0x24);      //Set cursor
    printDigit(upperTempLimit); //Print the upper limit
  }
  
  if (upperTempLimit < lowerTempLimit || lowerTempLimit > upperTempLimit){      //Check if the limits set can work
    Clear_Display();
    
    Write_Data_2_Display(0x96);
    Write_Data_2_Display(0x00);
    Write_Command_2_Display(0x24);      //Set cursor
    
    for (int i = 0; i < lengthstr; i++){
      Write_Data_2_Display(char_to_hex(str[i]));
      Write_Command_2_Display(0xC0);    //Alarm the user that the limit's wasn't corrently inputted
    }
    
    Delay(50000000);    //Delay to show the user the message for a reasonable period
    
    tempAlarmConfig(strL, strH, lengthL, lengthH);      //Return back to config
  }
  
}

void SysTick_Handler(void){ //SysTick Interrupt handler
  sysTickStatus++;  //SysTick Interrupt counter
 if(del){          // del and tdel varibales for temp and timer
    tdel++;
   }
  if (tdel == 15) {
    del=0;
    tdel=0;
  }
  if (upperTempLimit < temperature && temperature != 0.0){      //Check if the latest temperature reading was high
    tempAlarm(1);
    //Jump to alarm function
  }
  
  if (lowerTempLimit > temperature && temperature != 0.0){      //Check if the latest temperature reading was low
    tempAlarm(0);
    //Jump to alarm function
  } 
}

char int_to_hex(int i){ //Function converts int to hexadecimal (for printing to display)
  switch (i){
  case 0:
    return 0x10;
    break;
    
  case 1:
    return 0x11;
  
    
  case 2:
    return 0x12;
  
    
  case 3:
    return 0x13;
    break;
    
  case 4:
    return 0x14;
    break;
    
  case 5:
    return 0x15;
    break;
    
  case 6:
    return 0x16;
    break;
    
  case 7:
    return 0x17;
    break;
    
  case 8:
    return 0x18;
    break;
    
  case 9:
    return 0x19;
    break;
    
  case 10: // *
    return 0xA0;
    break;
    
 /* case 11: //0
    return 0x10;
    break; */
  
  case 12: // #
    return 0x03;
    break;
  }
  
  return 0x00;
}

char char_to_hex(char c){ // Function converts char (letters) to hex -||-
  switch (c) {
  case 'a':
    return 0x41;
    break;
    
  case 'b':
    return 0x42;
    break;
    
  case 'c':
    return 0x43;
    break;
    
  case 'd':
    return 0x44;
    break;
    
  case 'e':
    return 0x45;
    break;
    
  case 'f':
    return 0x46;
    break;
    
  case 'g':
    return 0x47;
    break;
    
  case 'h':
    return 0x48;
    break;
    
  case 'i':
    return 0x49;
    break;
    
  case 'j':
    return 0x4A;
    break;
    
  case 'k':
    return 0x4B;
    break;
    
  case 'l':
    return 0x4C;
    break;
    
  case 'm':
    return 0x4D;
    break;
    
  case 'n':
    return 0x4E;
    break;
    
  case 'o':
    return 0x4F;
    break;
    
  case 'p':
    return 0x50;
    break;
    
  case 'q':
    return 0x51;
    break;
    
  case 'r':
    return 0x52;
    break;
    
  case 's':
    return 0x53;
    break;
    
  case 't':
    return 0x54;
    break;
    
  case 'u':
    return 0x55;
    break;
    
  case 'v':
    return 0x56;
    break;
    
  case 'w':
    return 0x57;
    break;
    
  case 'x':
    return 0x58;
    break;
    
  case 'y':
    return 0x59;
    break;
    
  case 'z':
    return 0x5A;
    break;
    
  case 'A':
    return 0x21;
    break;
    
  case 'B':
    return 0x22;
    break;
    
  case 'C':
    return 0x23;
    break;
    
  case 'D':
    return 0x24;
    break;
    
  case 'E':
    return 0x25;
    break;
    
  case 'F':
    return 0x26;
    break;
    
  case 'G':
    return 0x27;
    break;
    
  case 'H':
    return 0x28;
    break;
    
  case 'I':
    return 0x29;
    break;
    
  case 'J':
    return 0x2A;
    break;
    
  case 'K':
    return 0x2B;
    break;
    
  case 'L':
    return 0x2C;
    break;
    
  case 'M':
    return 0x2D;
    break;
    
  case 'N':
    return 0x2E;
    break;
    
  case 'O':
    return 0x2F;
    break;
    
  case 'P':
    return 0x30;
    break;
    
  case 'Q':
    return 0x31;
    break;
    
  case 'R':
    return 0x32;
    break;
    
  case 'S':
    return 0x33;
    break;
    
  case 'T':
    return 0x34;
    break;
    
  case 'U':
    return 0x35;
    break;
    
  case 'V':
    return 0x36;
    break;
    
  case 'W':
    return 0x37;
    break;
    
  case 'X':
    return 0x38;
    break;
    
  case 'Y':
    return 0x39;
    break;
    
  case 'Z':
    return 0x3A;
    break;
    
  case '/':
    return 0x0F;
    break;
    
  case ':':
    return 0x1A;
    break;
    
  case ' ':
    return 0x00;
    break;
    
  case ',':
    return 0x0C;
    break;
    
  case '#':
    return 0x03;
    break; 
   
  case '0':
   return 0x10;
   break;  
    
  case '1':
   return 0x11;
   break;
   
   case '2':
    return 0x12;
    break;
    
  case '3':
    return 0x13;
    break;
    
  case '4':
    return 0x14;
    break;
    
  case '5':
    return 0x15;
    break;
    
  case '6':
    return 0x16;
    break;
    
  case '7':
    return 0x17;
    break;
    
  case '8':
    return 0x18;
    break;
    
  case '9':
    return 0x19;
    break;
  }

  return 0x00;
}

void write_date_and_time(void){ //Write date and time to display
  
  //Set cursor 
  Write_Data_2_Display(0xB8); 
  Write_Data_2_Display(0x01);
  Write_Command_2_Display(0x24);
  
  //Print date
  //Day 
  Write_Data_2_Display(int_to_hex((int) (day % 100) / 10)); 
  Write_Command_2_Display(0xC0);
  
  Write_Data_2_Display(int_to_hex((int) day % 10));
  Write_Command_2_Display(0xC0);
  
  //Slash
  Write_Data_2_Display(char_to_hex('/'));
  Write_Command_2_Display(0xC0);
  
  //Month
  Write_Data_2_Display(int_to_hex((int) (month % 100) / 10));
  Write_Command_2_Display(0xC0);
  
  Write_Data_2_Display(int_to_hex((int) month % 10));
  Write_Command_2_Display(0xC0);
  
  //Slash
  Write_Data_2_Display(char_to_hex('/'));
  Write_Command_2_Display(0xC0);
  
  //Year
  Write_Data_2_Display(int_to_hex((int) (year % 10000)/ 1000));
  Write_Command_2_Display(0xC0);
  
  Write_Data_2_Display(int_to_hex((int) (year % 1000) / 100));
  Write_Command_2_Display(0xC0);
  
  Write_Data_2_Display(int_to_hex((int) (year % 100) / 10));
  Write_Command_2_Display(0xC0);
  
  Write_Data_2_Display(int_to_hex((int) year % 10));
  Write_Command_2_Display(0xC0);
  
  
  //Set cursor(down-left corner)
  Write_Data_2_Display(0xD8);
  Write_Data_2_Display(0x01);
  Write_Command_2_Display(0x24);
  
  
  //Time
  //Hour
  Write_Data_2_Display(int_to_hex((int) (hour % 100) / 10));
  Write_Command_2_Display(0xC0);
  
  Write_Data_2_Display(int_to_hex((int) hour % 10));
  Write_Command_2_Display(0xC0);
  
  //Colon
  Write_Data_2_Display(char_to_hex(':'));
  Write_Command_2_Display(0xC0);
  
  //Minute
  Write_Data_2_Display(int_to_hex((int) (minute % 100) / 10));
  Write_Command_2_Display(0xC0);
  
  Write_Data_2_Display(int_to_hex((int) minute % 10));
  Write_Command_2_Display(0xC0);
  
  //Colon
  Write_Data_2_Display(char_to_hex(':'));
  Write_Command_2_Display(0xC0);
  
  //Second
  Write_Data_2_Display(int_to_hex((int) (second % 100) / 10));
  Write_Command_2_Display(0xC0);
  
  Write_Data_2_Display(int_to_hex((int) second % 10));
  Write_Command_2_Display(0xC0);
}
  
void updateTime(void){  //Update the time
  if (fastMode == 1){
    minute++;;
  } 
   else {
    second++;             //Uppdates the Second variable
    
    if (second > 59){     //Checking if second is 60
      second = 0;
      minute++;
    }
  }
  
  if (minute > 59){     //Checking if minute is 60
    
    minute = (minute % 60);
    hour++;
  }
  if (hour > 24){       //Checking if hour is 25
    hour = 1;
    day++;
  }
  // tre st if statements: en för ljus och en för LED som totalt ska va igång 16h
  // den sista är för 8h mörker, då ska LED va släckt tills den tiden har gått
  if (hour == 22){ // från 6 till 22, 16h passerade
    LEDflag = 1; // turn off LED 
  }
  
  if (hour == 06){ // Solen stiger
    sunflag = 1; // Get light from source
  }
  
    
  switch(month){        //Checks what month it is and checks if day has gone to the next month
  case 1:
    if (day > 31){
      day = 1;
      month++;
    }
    break;
      
  case 2:
    if (day > 28){
      day = 1;
      month++;
    }
    break;
    
  case 3:
    if (day > 31){
      day = 1;
      month++;
    }
    break;
    
  case 4:
    if (day > 30){
      day = 1;
      month++;
    }
    break;
    
  case 5:
    if (day > 31){
      day = 1;
      month++;
    }
    break;
    
  case 6:
    if (day > 30){
      day = 1;
      month++;
    }
    break;
    
  case 7:
    if (day > 31){
      day = 1;
      month++;
    }
    break;
    
  case 8:
    if (day > 31){
      day = 1;
      month++;
    }
    break;
    
  case 9:
    if (day > 30){
      day = 1;
      month++;
    }
    break;
    
  case 10:
    if (day > 31){
      day = 1;
      month++;
    }
    break;
    
  case 11:
    if (day > 30){
      day = 1;
      month++;
    }
    break;
    
  case 12:
    if (day > 31){
      day = 1;
      month = 1;
      year++;
    }
    break;
  }
  
  sysTickStatus = 0;            //Reset the SysTick Interrupt counter

  write_date_and_time();        //Print date and time to screen
}

int readKeypad(void){  //Read the keypad

value = 0;

  *AT91C_PIOD_CODR = (1<<2); // Set OE to low

  *AT91C_PIOC_OER = (1<<7);
  *AT91C_PIOC_OER = (1<<8);
  *AT91C_PIOC_OER = (1<<9); // Enable columns as output
  
  *AT91C_PIOC_ODR = (1<<5);
  *AT91C_PIOC_ODR = (1<<4);
  *AT91C_PIOC_ODR = (1<<3);
  *AT91C_PIOC_ODR = (1<<2); // Enable rows as input
  
  *AT91C_PIOC_SODR = (1<<7);
  *AT91C_PIOC_SODR = (1<<8);
  *AT91C_PIOC_SODR = (1<<9); // Enable columns as active high
  
  for (int i = 0; i < 3; i++) { 
    
    *AT91C_PIOC_CODR = column[i]; 
     
     for (int j = 0; j < 4; j++){
       
       if ((*AT91C_PIOC_PDSR & row[j]) == 0){ //Read the row and check if bit is zero
          value = j*3 + i + 1;  
       }
     }
     
     *AT91C_PIOC_SODR = (1<<7);
     *AT91C_PIOC_SODR = (1<<8);
     *AT91C_PIOC_SODR = (1<<9); //Enable columns as active high (again)
  }
  
     *AT91C_PIOC_ODR = (1<<7); 
     *AT91C_PIOC_ODR = (1<<8);
     *AT91C_PIOC_ODR = (1<<9);  // Enable column pins as input
     
     *AT91C_PIOD_SODR = (1 << 2);       //Set OE to high
     return value;
     }

double measureTemp(){ // Read temp func
  int Rb;
  int Ra;
  int time;
  
  while (1){ 
      if (flag2 == 0) { // flags for reading temp
        measure_Temp(); // init the temp
          flag2 = 1;
        
      }else{
        if (flag == 1) {     
          Rb = (*AT91C_TC0_RB);
          Ra = (*AT91C_TC0_RA);
          
          time = Rb-Ra;

          temperature = (double)((((time * (1/42.0))/5.0)-273.15)-3);   
          flag = 0;
          flag2 = 0;
          return temperature;
        }        
      }
  }
}

void sysTickOnOff(int onOff){ //Turn the SysTick on and off
  if (onOff == 0 && fastMode == 0){
    *((volatile unsigned int *) 0xE000E010) = (1 << 0);  //Enable the SysTick
      SysTick_Config(SystemCoreClock * 0.001);  //Configures the SysTick to "tick" at rate factor, 1 ms
      // SysTick has a factor of 0.001 for real-time
  } else if (onOff == 1){
    *((volatile unsigned int *) 0xE000E010) = (0 << 0); //Disable the SysTick
    Clear_Display();                                    //Clear the display
  }
}

void fastModeOnOff(void){ //Enable / Disable fast mode
  if (fastMode == 0){   //Checking if fastMode is off
    fastMode = 1;       //Switch fastMode variable to 1 (on)
    sysTickDelay = 32;   //Set sysTickDelay to 32, bigger then 33 which is 30 min
    
  } else if (fastMode == 1){    //Checking if fastMode is on
    fastMode = 0;               //Switch fastMode variable to 0 (off)
    sysTickDelay = 999;         //Set sysTickDelay to 999
  }
}

void printTempRecorderResults(int day, double average, struct LinkedList *min, struct LinkedList *max, int monthTemp, int yearTemp){
  //Print the result from recording of temperature
  
  char strDay[] = "Day: ";
  int lengthDay = strlen(strDay);
  
  char strAverage[] = "Average temperature: ";
  int lengthAverage = strlen(strAverage);
  
  char strMinTemp[] = "Minimum temperature: ";
  int lengthMinTemp = strlen(strMinTemp);
  
  char strMaxTemp[] = "Maximum temperature: ";
  int lengthMaxTemp = strlen(strMaxTemp);
  
  char strDayMinMax[] = "Achieved: ";
  int lengthDayMinMax = strlen(strDayMinMax);
  
  char strDayReturn[] = "Press # to return to overview";
  int lengthDayReturn = strlen(strDayReturn);
  
  int tempChange = 0;
  
  Clear_Display();      //Clear the display
  Write_Data_2_Display(0x0C);
  Write_Data_2_Display(0x00);
  Write_Command_2_Display(0x24); //Set the cursor
      
  for (int i = 0; i < lengthDay; i++){ //Print: Day:
    Write_Data_2_Display(char_to_hex(strDay[i]));
    Write_Command_2_Display(0xC0);
  }
      
  Write_Data_2_Display(int_to_hex(day)); //Print the day
  Write_Command_2_Display(0xC0);
      
  //Minimum value/day
      
  Write_Data_2_Display(0x3C);
  Write_Data_2_Display(0x00);
  Write_Command_2_Display(0x24); //Set the cursor
      
  for (int i = 0; i < lengthMinTemp; i++){ //Print: Minimum temperature: 
    Write_Data_2_Display(char_to_hex(strMinTemp[i]));
    Write_Command_2_Display(0xC0);
  }
      
  printDigit(min->sensorData); //Print the temperature
      
  Write_Data_2_Display(0x5A);
  Write_Data_2_Display(0x00);
  Write_Command_2_Display(0x24); //Set the cursor
      
  for (int i = 0; i < lengthDayMinMax; i++){ //Print: Achieved: 
    Write_Data_2_Display(char_to_hex(strDayMinMax[i]));
    Write_Command_2_Display(0xC0);
  }
      
  Write_Data_2_Display(int_to_hex(((min->id) % 1000000) / 100000));
  Write_Command_2_Display(0xC0); //First digit of hour
      
  Write_Data_2_Display(int_to_hex(((min->id) % 100000) / 10000));
  Write_Command_2_Display(0xC0); //Second digit of hour
      
  Write_Data_2_Display(char_to_hex(':'));
  Write_Command_2_Display(0xC0); //(:)
      
  Write_Data_2_Display(int_to_hex(((min->id) % 10000) / 1000));
  Write_Command_2_Display(0xC0); //First digit of minute
      
  Write_Data_2_Display(int_to_hex(((min->id) % 1000) / 100));
  Write_Command_2_Display(0xC0); //Second digit of minute
      
  Write_Data_2_Display(char_to_hex(':'));
  Write_Command_2_Display(0xC0); //(:)
      
  Write_Data_2_Display(int_to_hex(((min->id) % 100) / 10));
  Write_Command_2_Display(0xC0); //First digit of second
      
  Write_Data_2_Display(int_to_hex(((min->id) % 10)));
  Write_Command_2_Display(0xC0); //Second digit of second
      
  Write_Data_2_Display(char_to_hex(' '));
  Write_Command_2_Display(0xC0); //(Space)
      
  Write_Data_2_Display(int_to_hex(((min->id) % 100000000) / 10000000));
  Write_Command_2_Display(0xC0); //First digit of day
      
  Write_Data_2_Display(int_to_hex(((min->id) % 10000000) / 1000000));
  Write_Command_2_Display(0xC0); //Second digit of day
      
  Write_Data_2_Display(char_to_hex('/'));
  Write_Command_2_Display(0xC0); //(/)
      
  Write_Data_2_Display(int_to_hex((min->id) / 1000000000));
  Write_Command_2_Display(0xC0); //First digit of month
      
  Write_Data_2_Display(int_to_hex(((min->id) / 100000000) % 10));
  Write_Command_2_Display(0xC0); //Second digit of month
      
  Write_Data_2_Display(char_to_hex('/'));
  Write_Command_2_Display(0xC0); //(/)
      
  if ((monthTemp != ((((int)(min->id) / 1000000000) * 10) + (((int)(min->id) / 100000000) % 10))) && monthTemp == 12){
    yearTemp++;
    tempChange = 1;
  }
      
  Write_Data_2_Display(int_to_hex(yearTemp / 1000));
  Write_Command_2_Display(0xC0); //First digit of year
      
  Write_Data_2_Display(int_to_hex((yearTemp / 100) % 10));
  Write_Command_2_Display(0xC0); //Second digit of year
      
  Write_Data_2_Display(int_to_hex((yearTemp / 1000) % 10));
  Write_Command_2_Display(0xC0); //Third digit of year
      
  Write_Data_2_Display(int_to_hex((yearTemp % 10)));
  Write_Command_2_Display(0xC0); //Fourth digit of year
      
  if (tempChange == 1){
    yearTemp--;
    tempChange = 0;
  }
      
  //Maximum value/day
      
  Write_Data_2_Display(0x96);
  Write_Data_2_Display(0x00);
  Write_Command_2_Display(0x24); //Set the cursor
      
  for (int i = 0; i < lengthMaxTemp; i++){ //Print: Maximum temperature: 
    Write_Data_2_Display(char_to_hex(strMaxTemp[i]));
    Write_Command_2_Display(0xC0);
  }
      
  printDigit(max->sensorData); //Print the temperature
      
  Write_Data_2_Display(0xB4);
  Write_Data_2_Display(0x00);
  Write_Command_2_Display(0x24); //Set the cursor
      
  for (int i = 0; i < lengthDayMinMax; i++){ //Print: Achieved: 
    Write_Data_2_Display(char_to_hex(strDayMinMax[i]));
    Write_Command_2_Display(0xC0);
  }
      
  Write_Data_2_Display(int_to_hex(((max->id) % 1000000) / 100000));
  Write_Command_2_Display(0xC0); //First digit of hour
      
  Write_Data_2_Display(int_to_hex(((max->id) % 100000) / 10000));
  Write_Command_2_Display(0xC0); //Second digit of hour
      
  Write_Data_2_Display(char_to_hex(':'));
  Write_Command_2_Display(0xC0); //(:)
      
  Write_Data_2_Display(int_to_hex(((max->id) % 10000) / 1000));
  Write_Command_2_Display(0xC0); //First digit of minute
      
  Write_Data_2_Display(int_to_hex(((max->id) % 1000) / 100));
  Write_Command_2_Display(0xC0); //Second digit of minute
      
  Write_Data_2_Display(char_to_hex(':'));
  Write_Command_2_Display(0xC0); //(:)
      
  Write_Data_2_Display(int_to_hex(((max->id) % 100) / 10));
  Write_Command_2_Display(0xC0); //First digit of second
      
  Write_Data_2_Display(int_to_hex(((max->id) % 10)));
  Write_Command_2_Display(0xC0); //Second digit of second
      
  Write_Data_2_Display(char_to_hex(' '));
  Write_Command_2_Display(0xC0); //(Space)
      
  Write_Data_2_Display(int_to_hex(((max->id) % 100000000) / 10000000));
  Write_Command_2_Display(0xC0); //First digit of day
      
  Write_Data_2_Display(int_to_hex(((max->id) % 10000000) / 1000000));
  Write_Command_2_Display(0xC0); //Second digit of day
      
  Write_Data_2_Display(char_to_hex('/'));
  Write_Command_2_Display(0xC0); //(/)
      
  Write_Data_2_Display(int_to_hex((max->id) / 1000000000));
  Write_Command_2_Display(0xC0); //First digit of month
      
  Write_Data_2_Display(int_to_hex(((max->id) / 100000000) % 10));
  Write_Command_2_Display(0xC0); //Second digit of month
      
  Write_Data_2_Display(char_to_hex('/'));
  Write_Command_2_Display(0xC0); //(/)
      
  if ((monthTemp != ((((int)(max->id) / 1000000000) * 10) + (((int)(max->id) / 100000000) % 10))) && monthTemp == 12){
    yearTemp++;
    tempChange = 1;
  }
      
  Write_Data_2_Display(int_to_hex(yearTemp / 1000));
  Write_Command_2_Display(0xC0); //First digit of year
      
  Write_Data_2_Display(int_to_hex((yearTemp / 100) % 10));
  Write_Command_2_Display(0xC0); //Second digit of year
      
  Write_Data_2_Display(int_to_hex((yearTemp / 10) % 10));
  Write_Command_2_Display(0xC0); //Third digit of year
      
  Write_Data_2_Display(int_to_hex((yearTemp % 10)));
  Write_Command_2_Display(0xC0); //Fourth digit of year
      
  if (tempChange == 1){
    yearTemp--;
    tempChange = 0;
  }
      
  //Average
      
  Write_Data_2_Display(0xF0);
  Write_Data_2_Display(0x00);
  Write_Command_2_Display(0x24); //Set the cursor
      
  for (int i = 0; i < lengthAverage; i++){ //Print: Average temperature: 
    Write_Data_2_Display(char_to_hex(strAverage[i]));
    Write_Command_2_Display(0xC0);
  }
      
  printDigit(average); //Print the temperature
      
  //Press to exit
      
  Write_Data_2_Display(0xA4);
  Write_Data_2_Display(0x01);
  Write_Command_2_Display(0x24); //Set the cursor
      
  for (int i = 0; i < lengthDayReturn; i++){ //Print: Press # to return to overview
    Write_Data_2_Display(char_to_hex(strDayReturn[i]));
    Write_Command_2_Display(0xC0);
  }
}

void tempRecorder(void){ //Function for recording temperature during 7 days
  
  struct LinkedList *firstNode;         //The first node in the list
  struct LinkedList *tempNode;          //The second first node in the list
  struct LinkedList *currentNode;       //The current node we are creating
  
  struct LinkedList *minNode1;
  struct LinkedList *minNode2;
  struct LinkedList *minNode3;
  struct LinkedList *minNode4;
  struct LinkedList *minNode5;
  struct LinkedList *minNode6;
  struct LinkedList *minNode7;  //Nodes for min value for each day
  
  struct LinkedList *maxNode1;
  struct LinkedList *maxNode2;
  struct LinkedList *maxNode3;
  struct LinkedList *maxNode4;
  struct LinkedList *maxNode5;
  struct LinkedList *maxNode6;
  struct LinkedList *maxNode7;  //Nodes for max value for each day
  
  double averageDay1 = 0;
  double averageDay2 = 0;
  double averageDay3 = 0;
  double averageDay4 = 0;
  double averageDay5 = 0;
  double averageDay6 = 0;
  double averageDay7 = 0;       //Variables for average of each day
  
  int secondTemp = second;      //Second variable
  int monthTemp = month;        //Month variable
  int yearTemp = year;          //Year variable
  
  int minuteCreation = minute;  //A second minute variable, used for knowing when a minute has passed
  
  double temp;  //Temperature variable
  
  int nodes = 0;        //Node counter
  
  int number;   //Variable for the keypad
  
  char strMenu1[] = "Temperature recording done!";
  int lengthMenu1 = strlen(strMenu1);
  
  char strMenu2[] = "Press the corresponding numberon the keypad to display the  results for the day. Press 0  to return to the home menu";
  int lengthMenu2 = strlen(strMenu2);
  
  char strDay[] = "Day: ";
  int lengthDay = strlen(strDay);
  
  char strRecording[] = "Recording temperature for";
  int lengthRecording = strlen(strRecording);
  
  //Day 1
  
  Clear_Display();      //Clear the display
  
  Write_Data_2_Display(0x7B);
  Write_Data_2_Display(0x00);
  Write_Command_2_Display(0x24);        //Set cursor
  
  for (int i = 0; i < lengthRecording; i++){
    Write_Data_2_Display(char_to_hex(strRecording[i]));
    Write_Command_2_Display(0xC0);      //Print text to screen
  }
  
  Write_Data_2_Display(0xA2);
  Write_Data_2_Display(0x00);
  Write_Command_2_Display(0x24);        //Set cursor
  
  for (int i = 0; i < lengthDay; i++){
    Write_Data_2_Display(char_to_hex(strDay[i]));
    Write_Command_2_Display(0xC0);      //Print text to screen
  }
  
  Write_Data_2_Display(int_to_hex(1));
  Write_Command_2_Display(0xC0);        //Print 1 to screen
  

  while (nodes != 1440){
    if (sysTickStatus > sysTickDelay){     //Checking if a SysTick interrupt has occured, use 32 for 30 min
      updateTime();             // simulation, use 999 for real-time simulation
    }
    
    
    if (secondTemp == second && minuteCreation != minute){
      temp = measureTemp();     //Measuring the temperature
      currentNode = createNode((second + (100 * minute) + (10000 * hour) + (1000000 * day) + (100000000 * month)), temp);
      //Allocating memory to currentNode and adding a timestamp and temp to it
      
      if (nodes == 0){  //Check if this is the first node we create
        minNode1 = currentNode; //Setting the min node to the first node we create
        maxNode1 = currentNode; //Setting the max node to the first node we create
        firstNode = currentNode;//Setting the firstNode node to the first node we create
      }
      
      if (currentNode == NULL){ //Checking if the malloc() returned a NULL, meaning the memory is full
        
        while ((firstNode == minNode1) || (firstNode == maxNode1)){     //Checking so we don't delete the min or max node
          firstNode = firstNode->next;  //Setting firstNode to it's next
          tempNode = tempNode->next;    //Setting tempNode to it's next (the node after firstNode)
        }
          
        remover(&firstNode, firstNode); //Removing the node from the list (not deleting it from memory)
        free(firstNode);                //Removing the node from memory
        firstNode = tempNode;           //Setting firstNode to tempNode (tempNode is firstNode's next node)

        currentNode = createNode((second + (100 * minute) + (10000 * hour) + (1000000 * day) + (100000000 * month)), temp);
        //Allocating memory to currentNode (again) and adding a timestamp and temp to it
        
      }  
      
        insertFirst(&firstNode, currentNode);   //Insert the new node first in the list
        minuteCreation = minute;                //Set the minuteCreation to the current minute (used for knowing when a minute has passed) 
        tempNode = firstNode;                   //Setting tempNode to firstNode (the second node in the list)
        firstNode = currentNode;                //Setting firstNode to currentNode (the first node in the list)
      
      if ((firstNode->sensorData) < (minNode1->sensorData)){//Checking if the created node's temperature is smaller then the prevoius smallest
        minNode1 = firstNode;   //Setting the smallest temperature node (minNode) to the created node
      }
      
      if ((firstNode->sensorData) > (maxNode1->sensorData)){//Checking if the created node's temperature is bigger then the previous biggest
        maxNode1 = firstNode;   //Setting the biggest temperature node (maxNode) to the created node
      }
      
      averageDay1 += firstNode->sensorData;     //Adding on the created nodes temperature to an average variable
      nodes++;  //Uppdating the total nodes variable (used for knowing if the node created is the first one created)
    }
  }
  
  averageDay1 = (averageDay1 / 1440.0); //Calculating the average of the day
  
  //Day 2
  
  nodes = 0;
  
  while (firstNode->next != NULL){
    if ((firstNode != minNode1) && (firstNode != maxNode1)){
      free(firstNode);
    }   //Clear the list but not deleating any min or max nodes
    
    firstNode = tempNode;
    tempNode = tempNode->next;
  }
  
  Write_Data_2_Display(0xA2);
  Write_Data_2_Display(0x00);
  Write_Command_2_Display(0x24);
  
  for (int i = 0; i < lengthDay; i++){
    Write_Data_2_Display(char_to_hex(strDay[i]));
    Write_Command_2_Display(0xC0);
  }
  
  Write_Data_2_Display(int_to_hex(2));
  Write_Command_2_Display(0xC0);
  
  while (nodes != 1440){
    if (sysTickStatus > sysTickDelay){     //Checking if a SysTick interrupt has occured, use 32 for 30 min
      updateTime();             // simulation, use 999 for real-time simulation
    }
    
    
    if (secondTemp == second && minuteCreation != minute){
      temp = measureTemp();     //Measuring the temperature
      currentNode = createNode((second + (100 * minute) + (10000 * hour) + (1000000 * day) + (100000000 * month)), temp);
      //Allocating memory to currentNode and adding a timestamp and temp to it
      
      if (nodes == 0){  //Check if this is the first node we create
        minNode2 = currentNode; //Setting the min node to the first node we create
        maxNode2 = currentNode; //Setting the max node to the first node we create
        firstNode = currentNode;//Setting the firstNode node to the first node we create
      }
      
      if (currentNode == NULL){ //Checking if the malloc() returned a NULL, meaning the memory is full
       
        while ((firstNode == minNode1) || (firstNode == maxNode1) 
               || (firstNode == minNode2) || (firstNode == maxNode2)){     //Checking so we don't delete the min or max node
          firstNode = firstNode->next;  //Setting firstNode to it's next
          tempNode = tempNode->next;    //Setting tempNode to it's next (the node after firstNode)
        }
          
        remover(&firstNode, firstNode); //Removing the node from the list (not deleting it from memory)
        free(firstNode);                //Removing the node from memory
        firstNode = tempNode;           //Setting firstNode to tempNode (tempNode is firstNode's next node)

        currentNode = createNode((second + (100 * minute) + (10000 * hour) + (1000000 * day) + (100000000 * month)), temp);
        //Allocating memory to currentNode (again) and adding a timestamp and temp to it
        
      } 
      
        insertFirst(&firstNode, currentNode);   //Insert the new node first in the list
        minuteCreation = minute;                //Set the minuteCreation to the current minute (used for knowing when a minute has passed) 
        tempNode = firstNode;                   //Setting tempNode to firstNode (the second node in the list)
        firstNode = currentNode;                //Setting firstNode to currentNode (the first node in the list)
      
      
      if ((firstNode->sensorData) < (minNode2->sensorData)){//Checking if the created node's temperature is smaller then the prevoius smallest
        minNode2 = firstNode;   //Setting the smallest temperature node (minNode) to the created node
      }
      
      if ((firstNode->sensorData) > (maxNode2->sensorData)){//Checking if the created node's temperature is bigger then the previous biggest
        maxNode2 = firstNode;   //Setting the biggest temperature node (maxNode) to the created node
      }
      
      averageDay2 += firstNode->sensorData;     //Adding on the created nodes temperature to an average variable
      nodes++;  //Uppdating the total nodes variable (used for knowing if the node created is the first one created)
    }
  }
  
  averageDay2 = (averageDay2 / 1440.0); //Calculating the average of the day
  
  //Day 3
  
  nodes = 0;
  
  while (firstNode->next != NULL){
    if ((firstNode != minNode1) && (firstNode != maxNode1)
        && (firstNode != minNode2) && (firstNode != maxNode2)){
      free(firstNode);
    }   //Clear the list but not deleating any min or max nodes
    
    firstNode = tempNode;
    tempNode = tempNode->next;
  }
  
  Write_Data_2_Display(0xA2);
  Write_Data_2_Display(0x00);
  Write_Command_2_Display(0x24);
  
  for (int i = 0; i < lengthDay; i++){
    Write_Data_2_Display(char_to_hex(strDay[i]));
    Write_Command_2_Display(0xC0);
  }
  
  Write_Data_2_Display(int_to_hex(3));
  Write_Command_2_Display(0xC0);
  
  while (nodes != 1440){
    if (sysTickStatus > sysTickDelay){     //Checking if a SysTick interrupt has occured, use 32 for 30 min
      updateTime();             // simulation, use 999 for real-time simulation
    }
    
    
    if (secondTemp == second && minuteCreation != minute){
      temp = measureTemp();     //Measuring the temperature
      currentNode = createNode((second + (100 * minute) + (10000 * hour) + (1000000 * day) + (100000000 * month)), temp);
      //Allocating memory to currentNode and adding a timestamp and temp to it
      
      if (nodes == 0){  //Check if this is the first node we create
        minNode3 = currentNode; //Setting the min node to the first node we create
        maxNode3 = currentNode; //Setting the max node to the first node we create
        firstNode = currentNode;//Setting the firstNode node to the first node we create
      }
      
      if (currentNode == NULL){ //Checking if the malloc() returned a NULL, meaning the memory is full
        
        while ((firstNode == minNode1) || (firstNode == maxNode1) 
               || (firstNode == minNode2) || (firstNode == maxNode2)
                 || (firstNode == minNode3) || (firstNode == maxNode3)){     //Checking so we don't delete the min or max node
          firstNode = firstNode->next;  //Setting firstNode to it's next
          tempNode = tempNode->next;    //Setting tempNode to it's next (the node after firstNode)
        }
          
        remover(&firstNode, firstNode); //Removing the node from the list (not deleting it from memory)
        free(firstNode);                //Removing the node from memory
        firstNode = tempNode;           //Setting firstNode to tempNode (tempNode is firstNode's next node)

        currentNode = createNode((second + (100 * minute) + (10000 * hour) + (1000000 * day) + (100000000 * month)), temp);
        //Allocating memory to currentNode (again) and adding a timestamp and temp to it
        
      } 
      
        insertFirst(&firstNode, currentNode);   //Insert the new node first in the list
        minuteCreation = minute;                //Set the minuteCreation to the current minute (used for knowing when a minute has passed) 
        tempNode = firstNode;                   //Setting tempNode to firstNode (the second node in the list)
        firstNode = currentNode;                //Setting firstNode to currentNode (the first node in the list)
      
      
      if ((firstNode->sensorData) < (minNode3->sensorData)){//Checking if the created node's temperature is smaller then the prevoius smallest
        minNode3 = firstNode;   //Setting the smallest temperature node (minNode) to the created node
      }
      
      if ((firstNode->sensorData) > (maxNode3->sensorData)){//Checking if the created node's temperature is bigger then the previous biggest
        maxNode3 = firstNode;   //Setting the biggest temperature node (maxNode) to the created node
      }
      
      averageDay3 += firstNode->sensorData;     //Adding on the created nodes temperature to an average variable
      nodes++;  //Uppdating the total nodes variable (used for knowing if the node created is the first one created)
    }
  }
  
  averageDay3 = (averageDay3 / 1440.0); //Calculating the average of the day
  
  //Day 4
  
  nodes = 0;
  
  while (firstNode->next != NULL){
    if ((firstNode != minNode1) && (firstNode != maxNode1)
        && (firstNode != minNode2) && (firstNode != maxNode2)
          && (firstNode != minNode3) && (firstNode != maxNode3)){
      free(firstNode);
    }   //Clear the list but not deleating any min or max nodes
    
    firstNode = tempNode;
    tempNode = tempNode->next;
  }
  
  Write_Data_2_Display(0xA2);
  Write_Data_2_Display(0x00);
  Write_Command_2_Display(0x24);
  
  for (int i = 0; i < lengthDay; i++){
    Write_Data_2_Display(char_to_hex(strDay[i]));
    Write_Command_2_Display(0xC0);
  }
  
  Write_Data_2_Display(int_to_hex(4));
  Write_Command_2_Display(0xC0);
  
  while (nodes != 1440){
    if (sysTickStatus > sysTickDelay){     //Checking if a SysTick interrupt has occured, use 32 for 30 min
      updateTime();             // simulation, use 999 for real-time simulation
    }
    
    
    if (secondTemp == second && minuteCreation != minute){
      temp = measureTemp();     //Measuring the temperature
      currentNode = createNode((second + (100 * minute) + (10000 * hour) + (1000000 * day) + (100000000 * month)), temp);
      //Allocating memory to currentNode and adding a timestamp and temp to it
      
      if (nodes == 0){  //Check if this is the first node we create
        minNode4 = currentNode; //Setting the min node to the first node we create
        maxNode4 = currentNode; //Setting the max node to the first node we create
        firstNode = currentNode;//Setting the firstNode node to the first node we create
      }
      
      if (currentNode == NULL){ //Checking if the malloc() returned a NULL, meaning the memory is full
        
        while ((firstNode == minNode1) || (firstNode == maxNode1) 
               || (firstNode == minNode2) || (firstNode == maxNode2)
                 || (firstNode == minNode3) || (firstNode == maxNode3) 
                   || (firstNode == minNode4)|| (firstNode == maxNode4)){     //Checking so we don't delete the min or max node
          firstNode = firstNode->next;  //Setting firstNode to it's next
          tempNode = tempNode->next;    //Setting tempNode to it's next (the node after firstNode)
        }
          
        remover(&firstNode, firstNode); //Removing the node from the list (not deleting it from memory)
        free(firstNode);                //Removing the node from memory
        firstNode = tempNode;           //Setting firstNode to tempNode (tempNode is firstNode's next node)

        currentNode = createNode((second + (100 * minute) + (10000 * hour) + (1000000 * day) + (100000000 * month)), temp);
        //Allocating memory to currentNode (again) and adding a timestamp and temp to it
        
      } 
      
        insertFirst(&firstNode, currentNode);   //Insert the new node first in the list
        minuteCreation = minute;                //Set the minuteCreation to the current minute (used for knowing when a minute has passed) 
        tempNode = firstNode;                   //Setting tempNode to firstNode (the second node in the list)
        firstNode = currentNode;                //Setting firstNode to currentNode (the first node in the list)
      
      
      if ((firstNode->sensorData) < (minNode4->sensorData)){//Checking if the created node's temperature is smaller then the prevoius smallest
        minNode4 = firstNode;   //Setting the smallest temperature node (minNode) to the created node
      }
      
      if ((firstNode->sensorData) > (maxNode4->sensorData)){//Checking if the created node's temperature is bigger then the previous biggest
        maxNode4 = firstNode;   //Setting the biggest temperature node (maxNode) to the created node
      }
      
      averageDay4 += firstNode->sensorData;     //Adding on the created nodes temperature to an average variable
      nodes++;  //Uppdating the total nodes variable (used for knowing if the node created is the first one created)
    }
  }
  
  averageDay4 = (averageDay4 / 1440.0); //Calculating the average of the day
  
  //Day 5
  
  nodes = 0;
  
  while (firstNode->next != NULL){
    if ((firstNode != minNode1) && (firstNode != maxNode1)
        && (firstNode != minNode2) && (firstNode != maxNode2)
          && (firstNode != minNode3) && (firstNode != maxNode3)
            && (firstNode != minNode4) && (firstNode != maxNode4)){
      free(firstNode);
    }   //Clear the list but not deleating any min or max nodes
    
    firstNode = tempNode;
    tempNode = tempNode->next;
  }
  
  Write_Data_2_Display(0xA2);
  Write_Data_2_Display(0x00);
  Write_Command_2_Display(0x24);
  
  for (int i = 0; i < lengthDay; i++){
    Write_Data_2_Display(char_to_hex(strDay[i]));
    Write_Command_2_Display(0xC0);
  }
  
  Write_Data_2_Display(int_to_hex(5));
  Write_Command_2_Display(0xC0);
  
  while (nodes != 1440){
    if (sysTickStatus > sysTickDelay){     //Checking if a SysTick interrupt has occured, use 32 for 30 min
      updateTime();             // simulation, use 999 for real-time simulation
    }
    
    
    if (secondTemp == second && minuteCreation != minute){
      temp = measureTemp();     //Measuring the temperature
      currentNode = createNode((second + (100 * minute) + (10000 * hour) + (1000000 * day) + (100000000 * month)), temp);
      //Allocating memory to currentNode and adding a timestamp and temp to it
      
      if (nodes == 0){  //Check if this is the first node we create
        minNode5 = currentNode; //Setting the min node to the first node we create
        maxNode5 = currentNode; //Setting the max node to the first node we create
        firstNode = currentNode;//Setting the firstNode node to the first node we create
      }
      
      if (currentNode == NULL){ //Checking if the malloc() returned a NULL, meaning the memory is full
        
        while ((firstNode == minNode1) || (firstNode == maxNode1)
               || (firstNode == minNode2) || (firstNode == maxNode2)
                 || (firstNode == minNode3) || (firstNode == maxNode3)
                   || (firstNode == minNode4)|| (firstNode == maxNode4) 
                     || (firstNode == minNode5) || (firstNode == maxNode5)){     //Checking so we don't delete the min or max node
          firstNode = firstNode->next;  //Setting firstNode to it's next
          tempNode = tempNode->next;    //Setting tempNode to it's next (the node after firstNode)
        }
          
        remover(&firstNode, firstNode); //Removing the node from the list (not deleting it from memory)
        free(firstNode);                //Removing the node from memory
        firstNode = tempNode;           //Setting firstNode to tempNode (tempNode is firstNode's next node)

        currentNode = createNode((second + (100 * minute) + (10000 * hour) + (1000000 * day) + (100000000 * month)), temp);
        //Allocating memory to currentNode (again) and adding a timestamp and temp to it
        
      } 
      
        insertFirst(&firstNode, currentNode);   //Insert the new node first in the list
        minuteCreation = minute;                //Set the minuteCreation to the current minute (used for knowing when a minute has passed) 
        tempNode = firstNode;                   //Setting tempNode to firstNode (the second node in the list)
        firstNode = currentNode;                //Setting firstNode to currentNode (the first node in the list)
      
      
      if ((firstNode->sensorData) < (minNode5->sensorData)){//Checking if the created node's temperature is smaller then the prevoius smallest
        minNode5 = firstNode;   //Setting the smallest temperature node (minNode) to the created node
      }
      
      if ((firstNode->sensorData) > (maxNode5->sensorData)){//Checking if the created node's temperature is bigger then the previous biggest
        maxNode5 = firstNode;   //Setting the biggest temperature node (maxNode) to the created node
      }
      
      averageDay5 += firstNode->sensorData;     //Adding on the created nodes temperature to an average variable
      nodes++;  //Uppdating the total nodes variable (used for knowing if the node created is the first one created)
    }
  }
  
  averageDay5 = (averageDay5 / 1440.0); //Calculating the average of the day
  
  //Day 6
  
  nodes = 0;
  
  while (firstNode->next != NULL){
    if ((firstNode != minNode1) && (firstNode != maxNode1)
        && (firstNode != minNode2) && (firstNode != maxNode2)
          && (firstNode != minNode3) && (firstNode != maxNode3)
            && (firstNode != minNode4) && (firstNode != maxNode4)
              && (firstNode != minNode5) && (firstNode != maxNode5)){
      free(firstNode);
    }   //Clear the list but not deleating any min or max nodes
    
    firstNode = tempNode;
    tempNode = tempNode->next;
  }
  
  Write_Data_2_Display(0xA2);
  Write_Data_2_Display(0x00);
  Write_Command_2_Display(0x24);
  
  for (int i = 0; i < lengthDay; i++){
    Write_Data_2_Display(char_to_hex(strDay[i]));
    Write_Command_2_Display(0xC0);
  }
  
  Write_Data_2_Display(int_to_hex(6));
  Write_Command_2_Display(0xC0);
  
  while (nodes != 1440){
    if (sysTickStatus > sysTickDelay){     //Checking if a SysTick interrupt has occured, use 32 for 30 min
      updateTime();             // simulation, use 999 for real-time simulation
    }
    
    
    if (secondTemp == second && minuteCreation != minute){
      temp = measureTemp();     //Measuring the temperature
      currentNode = createNode((second + (100 * minute) + (10000 * hour) + (1000000 * day) + (100000000 * month)), temp);
      //Allocating memory to currentNode and adding a timestamp and temp to it
      
      if (nodes == 0){  //Check if this is the first node we create
        minNode6 = currentNode; //Setting the min node to the first node we create
        maxNode6 = currentNode; //Setting the max node to the first node we create
        firstNode = currentNode;//Setting the firstNode node to the first node we create
      }
      
      if (currentNode == NULL){ //Checking if the malloc() returned a NULL, meaning the memory is full
       
        while ((firstNode == minNode1) || (firstNode == maxNode1) 
               || (firstNode == minNode2) || (firstNode == maxNode2)
                 || (firstNode == minNode3) || (firstNode == maxNode3) 
                   || (firstNode == minNode4)|| (firstNode == maxNode4) 
                     || (firstNode == minNode5) || (firstNode == maxNode5)
                       || (firstNode == minNode6) || (firstNode == maxNode6)){     //Checking so we don't delete the min or max node
          firstNode = firstNode->next;  //Setting firstNode to it's next
          tempNode = tempNode->next;    //Setting tempNode to it's next (the node after firstNode)
        }
          
        remover(&firstNode, firstNode); //Removing the node from the list (not deleting it from memory)
        free(firstNode);                //Removing the node from memory
        firstNode = tempNode;           //Setting firstNode to tempNode (tempNode is firstNode's next node)

        currentNode = createNode((second + (100 * minute) + (10000 * hour) + (1000000 * day) + (100000000 * month)), temp);
        //Allocating memory to currentNode (again) and adding a timestamp and temp to it
        
      } 
      
        insertFirst(&firstNode, currentNode);   //Insert the new node first in the list
        minuteCreation = minute;                //Set the minuteCreation to the current minute (used for knowing when a minute has passed) 
        tempNode = firstNode;                   //Setting tempNode to firstNode (the second node in the list)
        firstNode = currentNode;                //Setting firstNode to currentNode (the first node in the list)
      
      
      if ((firstNode->sensorData) < (minNode6->sensorData)){//Checking if the created node's temperature is smaller then the prevoius smallest
        minNode6 = firstNode;   //Setting the smallest temperature node (minNode) to the created node
      }
      
      if ((firstNode->sensorData) > (maxNode6->sensorData)){//Checking if the created node's temperature is bigger then the previous biggest
        maxNode6 = firstNode;   //Setting the biggest temperature node (maxNode) to the created node
      }
      
      averageDay6 += firstNode->sensorData;     //Adding on the created nodes temperature to an average variable
      nodes++;  //Uppdating the total nodes variable (used for knowing if the node created is the first one created)
    }
  }
  
  averageDay6 = (averageDay6 / 1440.0); //Calculating the average of the day
  
  //Day 7
  
  nodes = 0;
  
  while (firstNode->next != NULL){
    if ((firstNode != minNode1) && (firstNode != maxNode1)
        && (firstNode != minNode2) && (firstNode != maxNode2)
          && (firstNode != minNode3) && (firstNode != maxNode3)
            && (firstNode != minNode4) && (firstNode != maxNode4)
              && (firstNode != minNode5) && (firstNode != maxNode5)
                && (firstNode != minNode6) && (firstNode != maxNode6)){
      free(firstNode);
    }   //Clear the list but not deleating any min or max nodes
    
    firstNode = tempNode;
    tempNode = tempNode->next;
  }
  
  Write_Data_2_Display(0xA2);
  Write_Data_2_Display(0x00);
  Write_Command_2_Display(0x24);
  
  for (int i = 0; i < lengthDay; i++){
    Write_Data_2_Display(char_to_hex(strDay[i]));
    Write_Command_2_Display(0xC0);
  }
  
  Write_Data_2_Display(int_to_hex(7));
  Write_Command_2_Display(0xC0);
  
  while (nodes != 1440){
    if (sysTickStatus > sysTickDelay){     //Checking if a SysTick interrupt has occured, use 32 for 30 min
      updateTime();             // simulation, use 999 for real-time simulation
    }
    
    
    if (secondTemp == second && minuteCreation != minute){
      temp = measureTemp();     //Measuring the temperature
      currentNode = createNode((second + (100 * minute) + (10000 * hour) + (1000000 * day) + (100000000 * month)), temp);
      //Allocating memory to currentNode and adding a timestamp and temp to it
      
      if (nodes == 0){  //Check if this is the first node we create
        minNode7 = currentNode; //Setting the min node to the first node we create
        maxNode7 = currentNode; //Setting the max node to the first node we create
        firstNode = currentNode;//Setting the firstNode node to the first node we create
      }
      
      if (currentNode == NULL){ //Checking if the malloc() returned a NULL, meaning the memory is full
        
        while ((firstNode == minNode1) || (firstNode == maxNode1) 
               || (firstNode == minNode2) || (firstNode == maxNode2)
                 || (firstNode == minNode3) || (firstNode == maxNode3) 
                   || (firstNode == minNode4)|| (firstNode == maxNode4) 
                     || (firstNode == minNode5) || (firstNode == maxNode5)
                       || (firstNode == minNode6) || (firstNode == maxNode6)
                         || (firstNode == minNode7) || (firstNode == maxNode7)){     //Checking so we don't delete the min or max node
          firstNode = firstNode->next;  //Setting firstNode to it's next
          tempNode = tempNode->next;    //Setting tempNode to it's next (the node after firstNode)
        }
          
        remover(&firstNode, firstNode); //Removing the node from the list (not deleting it from memory)
        free(firstNode);                //Removing the node from memory
        firstNode = tempNode;           //Setting firstNode to tempNode (tempNode is firstNode's next node)

        currentNode = createNode((second + (100 * minute) + (10000 * hour) + (1000000 * day) + (100000000 * month)), temp);
        //Allocating memory to currentNode (again) and adding a timestamp and temp to it
        
      } 
      
        insertFirst(&firstNode, currentNode);   //Insert the new node first in the list
        minuteCreation = minute;                //Set the minuteCreation to the current minute (used for knowing when a minute has passed) 
        tempNode = firstNode;                   //Setting tempNode to firstNode (the second node in the list)
        firstNode = currentNode;                //Setting firstNode to currentNode (the first node in the list)
      
      
      if ((firstNode->sensorData) < (minNode7->sensorData)){//Checking if the created node's temperature is smaller then the prevoius smallest
        minNode7 = firstNode;   //Setting the smallest temperature node (minNode) to the created node
      }
      
      if ((firstNode->sensorData) > (maxNode7->sensorData)){//Checking if the created node's temperature is bigger then the previous biggest
        maxNode7 = firstNode;   //Setting the biggest temperature node (maxNode) to the created node
      }
      
      averageDay7 += firstNode->sensorData;     //Adding on the created nodes temperature to an average variable
      nodes++;  //Uppdating the total nodes variable (used for knowing if the node created is the first one created)
    }
  }
  
  averageDay7 = (averageDay7 / 1440.0); //Calculating the average of the day
  
  Clear_Display();      //Clear display
  
  number = readKeypad();
  
  
  while (1){
    
    Write_Data_2_Display(0x00);
    Write_Data_2_Display(0x00);
    Write_Command_2_Display(0x24); //Set the cursor
      
    for (int i = 0; i < lengthMenu1; i++){ //Print: (Menu1)
      Write_Data_2_Display(char_to_hex(strMenu1[i]));
      Write_Command_2_Display(0xC0);
    }
    
    Write_Data_2_Display(0x5A);
    Write_Data_2_Display(0x00);
    Write_Command_2_Display(0x24); //Set the cursor
      
    for (int i = 0; i < lengthMenu2; i++){ //Print: (Menu2)
      Write_Data_2_Display(char_to_hex(strMenu2[i]));
      Write_Command_2_Display(0xC0);
    }
    
    while (number == 0 || number == 8 || number == 9 || number == 10 || number == 12){
      number = readKeypad();    //Wait until usable button is pressed
    }
    
    if (number == 1){   //Print results from day 1
      printTempRecorderResults(number, averageDay1, minNode1, maxNode1, monthTemp, yearTemp);   //Print the results
       number = readKeypad();
       
       while (number != 12){    //Wait until # is pressed
         number = readKeypad();
      }
      
      Clear_Display();  //Clear display
    }
    
    if (number == 2){   //Print results from day 2
      printTempRecorderResults(number, averageDay2, minNode2, maxNode2, monthTemp, yearTemp);   //Print the results
       number = readKeypad();
       
       while (number != 12){    //Wait until # is pressed
         number = readKeypad();
      }
      
      Clear_Display();  //Clear display
    }
    
    if (number == 3){   //Print results from day 3
      printTempRecorderResults(number, averageDay3, minNode3, maxNode3, monthTemp, yearTemp);   //Print the results
       number = readKeypad();
       
       while (number != 12){    //Wait until # is pressed
         number = readKeypad();
      }
      
      Clear_Display();  //Clear display
    }
    
    if (number == 4){   //Print results from day 4
      printTempRecorderResults(number, averageDay4, minNode4, maxNode4, monthTemp, yearTemp);   //Print the results
       number = readKeypad();
       
       while (number != 12){    //Wait until # is pressed
         number = readKeypad();
      }
      
      Clear_Display();  //Clear display
    }
    
    if (number == 5){   //Print results from day 5
      printTempRecorderResults(number, averageDay5, minNode5, maxNode5, monthTemp, yearTemp);   //Print the results
       number = readKeypad();
       
       while (number != 12){    //Wait until # is pressed
         number = readKeypad();
      }
      
      Clear_Display();  //Clear display
    }
    
    if (number == 6){   //Print results from day 6
      printTempRecorderResults(number, averageDay6, minNode6, maxNode6, monthTemp, yearTemp);   //Print the results
       number = readKeypad();
       
       while (number != 12){    //Wait until # is pressed
         number = readKeypad();
      }
      
      Clear_Display();  //Clear display
    }
    
    if (number == 7){   //Print results from day 7
      printTempRecorderResults(number, averageDay7, minNode7, maxNode7, monthTemp, yearTemp);   //Print the results
       number = readKeypad();
       
       while (number != 12){    //Wait until # is pressed
         number = readKeypad();
      }
      
      Clear_Display();  //Clear display
    }
    
    if (number == 11){  //If 0 is pressed, return to GUI
      break;
    }
    
    number = readKeypad();
  }
  
  release(&firstNode);  //Deleate all nodes
  Clear_Display();      //Clear display
}

void timeConfigUser(void){ // Time and calendar config for user
  // Sätt kalendern och klockan till 0 när denna slås på
  year = 0;
  month = 0;
  day = 0;
  hour = 0;
  minute = 0;
  second = 0;
  
  int number;
  //Set cursor
  Write_Data_2_Display(0xB8);
  Write_Data_2_Display(0x01);
  Write_Command_2_Display(0x24);
  
  for (int i = 0; i < 2; i++){
    number = readKeypad();
    while (number == 0 || number == 10 || number == 12){
      number = readKeypad();
    }
    if (i == 0 && number != 11){
      day += number * 10;
      Write_Data_2_Display(int_to_hex((number)));
      Write_Command_2_Display(0xC0);
    }
    
    if (i == 1 && number != 11){
      day += number;
      Write_Data_2_Display(int_to_hex((number)));
      Write_Command_2_Display(0xC0);
    }
    
    Delay(2000000); 
  }
  
  Write_Data_2_Display(char_to_hex('/'));
  Write_Command_2_Display(0xC0);
  
  for (int i = 0; i < 2; i++){
    number = readKeypad();
    while (number == 0 || number == 10 || number == 12){
      number = readKeypad();
    }
    
    if (i == 0 && number != 11){
      month += number * 10;
      Write_Data_2_Display(int_to_hex((number)));
      Write_Command_2_Display(0xC0);
    }
    
    if (i == 1 && number != 11){
      month += number;
      Write_Data_2_Display(int_to_hex((number)));
      Write_Command_2_Display(0xC0);
    }
    
    Delay(2000000);
  }
  Write_Data_2_Display(char_to_hex('/'));
  Write_Command_2_Display(0xC0);
   for (int i = 0; i < 4; i++){
    number = readKeypad();
    while (number == 0 || number == 10 || number == 12){
      number = readKeypad();
    }
   
    if (i == 0 && number != 11){
      year += number * 1000;
      Write_Data_2_Display(int_to_hex((number)));
      Write_Command_2_Display(0xC0);
    }
   
    
    if (i == 1 && number != 11){
      year += number * 100;
      Write_Data_2_Display(int_to_hex((number)));
      Write_Command_2_Display(0xC0);
    }
   
    
    if (i == 2 && number != 11){
      year += number * 10;
      Write_Data_2_Display(int_to_hex((number)));
      Write_Command_2_Display(0xC0);
    }
    
    
    if (i == 3 && number != 11){
      year += number;
      Write_Data_2_Display(int_to_hex((number)));
      Write_Command_2_Display(0xC0);
    }
    Delay(2000000);
  }
  Write_Data_2_Display(0xD8);
  Write_Data_2_Display(0x01);
  Write_Command_2_Display(0x24);
  
  for (int i = 0; i < 2; i++){
    number = readKeypad();
    while (number == 0 || number == 10 || number == 12){
      number = readKeypad();
    }
    
    if (i == 0 && number != 11){
      hour += number * 10;
      Write_Data_2_Display(int_to_hex((number)));
      Write_Command_2_Display(0xC0);
    }
    
    if (i == 1 && number != 11){
      hour += number;
      Write_Data_2_Display(int_to_hex((number)));
      Write_Command_2_Display(0xC0);
    }
    
    Delay(2000000);
  }
  Write_Data_2_Display(char_to_hex(':'));
  Write_Command_2_Display(0xC0);
  
  for (int i = 0; i < 2; i++){
    number = readKeypad();
    while (number == 0 || number == 10 || number == 12){
      number = readKeypad();
    }
    
    if (i == 0 && number != 11){
      minute += number * 10;
      Write_Data_2_Display(int_to_hex((number)));
      Write_Command_2_Display(0xC0);
    }
    
    if (i == 1 && number != 11){
      minute += number;
      Write_Data_2_Display(int_to_hex((number)));
      Write_Command_2_Display(0xC0);
    }
    
    Delay(2000000);
  }
  Write_Data_2_Display(char_to_hex(':'));
  Write_Command_2_Display(0xC0);
  
  for (int i = 0; i < 2; i++){
    number = readKeypad();
    while (number == 0 || number == 10 || number == 12){
      number = readKeypad();
    }
    
    if (i == 0 && number != 11){
      second += number * 10;
      Write_Data_2_Display(int_to_hex((number)));
      Write_Command_2_Display(0xC0);
    }
    
    if (i == 1 && number != 11){
      second += number;
      Write_Data_2_Display(int_to_hex((number)));
      Write_Command_2_Display(0xC0);
    }
    
    Delay(2000000);
  }
 
}

void init_PWM(){ // Initilize the pulse width modulation
 *AT91C_PMC_PCER = (1 << 12);  // Peripheral enable PIOB
 *AT91C_PMC_PCER1 = (1 << 4);  // Peripheral enable PWM
 *AT91C_PIOB_PDR = (1 << 17);    // Disable PDR (Enable peripheral)
 *AT91C_PIOB_ABMR = (1 << 17);   // Peripheral Select Register
 *AT91C_PWMC_ENA = (1 << 1);  // Enable PWM-channel for CHID1
 *AT91C_PWMC_CH1_CMR = 5;  // Set to Master_CLK/32  
 *AT91C_PWMC_CH1_CPRDR = (int)(20.0 * 2625);  // Set the period to 20ms
 *AT91C_PWMC_CH1_CDTYR = (int)(2625); // Set the cycle to 1ms (2 * 2625 för rak)
}

void turn_Servor(int key){  // Func to turn the servo with pressed key
  int degrees = 2625.0 * ((((key * 10) * 1.933) / 180)+ 0.7); // (t*mck)/32=2625
  (*AT91C_PWMC_CH1_CDTYR) = degrees; // CDTYR controls the servo
}

void greenHouseControll(void){ //Controll the greenhouse enviourment
  
  char strGHC1[] = "Controlling the greenhouse    enviorment";
  int lengthGHC1 = strlen(strGHC1);
  
  char strGHC2[] = "Press 8 to cancel/return";
  int lengthGHC2 = strlen(strGHC2);
  
  char strGHCServo[] = "Servo turning degree: ";
  int lengthGHCServo = strlen(strGHCServo);
  
  char strGHCShades[] = "Shades are currently: ";
  int lengthGHCShades = strlen(strGHCShades);
  
  char strGHCUserStop[] = "Program has stopped";
  int lengthGHCUserStop = strlen(strGHCUserStop);
  
  char strGHCDayStop[] = "A day has concluded";
  int lengthGHCDayStop = strlen(strGHCDayStop);
  
  char strGHCHoursOfSun[] = "Total hours of sunlight: ";
  int lengthGHCHoursOfSun = strlen(strGHCHoursOfSun);
  
  char strGHCHoursOfDark[] = "Total hours of darkness: ";
  int lengthGHCHoursOfDark = strlen(strGHCHoursOfDark);
  
  char strGHCHoursOfLED[] = "Total hours of LED: ";
  int lengthGHCHoursOfLED = strlen(strGHCHoursOfLED);

  Clear_Display();      //Clear the display
  
  init_ADC();   //Initialise the light sensor
  init_PWM();   //Initialist the servo
  
  float ambientLight = 0.0;     //Ambient light is the "normal" light of the room
  
  for (int i = 0; i < 10000; i++){
    ambientLight += measure_ADC();
  }
  ambientLight = ambientLight / 10000;  //Getting a mean value of the room light
  
  int totalMinutes = 0;
  int totalMinutesOfSun = 0;
  int totalMinutesOfLED = 0;
  int minutesRemaining = 1440;
  int prevMinute = minute;      //Variables for knowing minutes
  
  int flag = 0;
  int shadesFlag = 0;
  int sunFlag = 0;      //Flag variables
  
  float light = 0.0;    //Light reading
  
  while (1){
    if (sysTickStatus > sysTickDelay){     //Checking if a SysTick interrupt has occured, use 32 for 30 min
      updateTime();             // simulation, use 999 for real-time simulation
    }
    
    Write_Data_2_Display(0x00);
    Write_Data_2_Display(0x00);
    Write_Command_2_Display(0x24);      //Set cursor
  
    for (int i = 0; i < lengthGHC1; i++){
      Write_Data_2_Display(char_to_hex(strGHC1[i]));
      Write_Command_2_Display(0xC0);    //Print text to screen
    }
  
    Write_Data_2_Display(0x5A);
    Write_Data_2_Display(0x00);
    Write_Command_2_Display(0x24);      //Set cursor
  
    for (int i = 0; i < lengthGHCShades; i++){
      Write_Data_2_Display(char_to_hex(strGHCShades[i]));
      Write_Command_2_Display(0xC0);    //Print text to screen
    }
   
    Write_Data_2_Display(0x96);
    Write_Data_2_Display(0x00);
    Write_Command_2_Display(0x24);      //Set cursor
  
    for (int i = 0; i < lengthGHCServo; i++){
      Write_Data_2_Display(char_to_hex(strGHCServo[i]));
      Write_Command_2_Display(0xC0);    //Print text to screen
    }
  
    Write_Data_2_Display(0xD2);
    Write_Data_2_Display(0x00);
    Write_Command_2_Display(0x24);      //Set cursor
  
    for (int i = 0; i < lengthGHC2; i++){
      Write_Data_2_Display(char_to_hex(strGHC2[i]));
      Write_Command_2_Display(0xC0);    //Print text to screen
    }
    
    double temp = measureTemp();        //Measure the temperature (to know if it's to hot or to cold for the alarm)
    
    if (prevMinute != minute){  //Check if a minute has passed
      totalMinutes++;   //Count up total minutes
      minutesRemaining--;       //Count down minutes remaining
      prevMinute = minute;      //Update the previous minute
      if (((*AT91C_PIOD_ODSR & (1<<3)) == (1<<3))){     //Check if the LED is on
        totalMinutesOfLED++;    //Count up total minutes of LED shine
      }
      if (sunFlag == 1){        //Check if the "sun" is shining
        totalMinutesOfSun++;    //Count up total minutes of suntime
        if (totalMinutesOfSun == 960){  //Check if the sun has been shining for 16h
          shadesFlag = 1;       //Set shades flag to 1
        }
      } else if (sunFlag == 0 && (960 - totalMinutesOfSun) == minutesRemaining){ //Check if the LED needs to engare to ensure 16h of light
        Set_Led(1);     //Turn on the LED
      }
    }
    
    if (shadesFlag == 1){       //Check if the shades flag is 1
      Write_Data_2_Display(0x70);
      Write_Data_2_Display(0x00);
      Write_Command_2_Display(0x24);    //Set cursor
      
      Write_Data_2_Display(char_to_hex('O'));
      Write_Command_2_Display(0xC0);    //Print to screen
      
      Write_Data_2_Display(char_to_hex('N'));
      Write_Command_2_Display(0xC0);    //Print to screen
      
      Write_Data_2_Display(char_to_hex(' '));
      Write_Command_2_Display(0xC0);    //Print to screen
    } else {    //Shades flag was 0
      Write_Data_2_Display(0x70);
      Write_Data_2_Display(0x00);
      Write_Command_2_Display(0x24);    //Set cursor
  
      Write_Data_2_Display(char_to_hex('O'));
      Write_Command_2_Display(0xC0);    //Print to screen
  
      Write_Data_2_Display(char_to_hex('F'));
      Write_Command_2_Display(0xC0);    //Print to screen
  
      Write_Data_2_Display(char_to_hex('F'));
      Write_Command_2_Display(0xC0);    //Print to screen
    }

    Write_Data_2_Display(0xAC);
    Write_Data_2_Display(0x00);
    Write_Command_2_Display(0x24);      //Set cursor
    
    light = measure_ADC();      //Measure the light sensor
    
    if (light > ambientLight - 2.0){    //Check if the "sun" is shining or not
      sunFlag = 0;      //Set the sunflag to 0 (sun is not shining)
    }
    
    if (light < (ambientLight / 2) + 6.0 && shadesFlag == 0){   //Check if the sun is to the right (45 degrees)
      flag++;   //Count up flag
      sunFlag = 1;      //Set sunflag to 1 (sun is shining)
      if (flag > 15){   //Check if flag has incremented 15 times (to make the servo motor more "smooth")
        turn_Servor(3937);       //Turn the servo for 1,5 ms
        Write_Data_2_Display(int_to_hex(4));
        Write_Command_2_Display(0xC0);  //Print to screen
      
        Write_Data_2_Display(int_to_hex(5));
        Write_Command_2_Display(0xC0);  //Print to screen
        
        Write_Data_2_Display(char_to_hex(' '));
        Write_Command_2_Display(0xC0);  //Print to screen
        flag = 0;       //Set flag to 0 to count up again
      }
    }
    
    if (light < (ambientLight / 2) + 8.0 && light > (ambientLight / 2) + 6.0 && shadesFlag == 0){
      //Middle Right (67 degrees), a "deadzone" where the servo won't turn but we still recognise that the sun is shining
      sunFlag = 1;      //Set the sun flag to 1
    }
    
    if (light < (ambientLight / 2) + 11.0 && light > (ambientLight / 2) + 8.0 && shadesFlag == 0){
      //Middle (90 degrees)
      flag++;   //Count up flag
      sunFlag = 1;
      if (flag > 15){   //Check if flag has incremented 15 times (to make the servo motor more "smooth")
        turn_Servor(5250);       //Turn the servo for 2 ms
        Write_Data_2_Display(int_to_hex(9));
        Write_Command_2_Display(0xC0);  //Print to screen
      
        Write_Data_2_Display(int_to_hex(0));
        Write_Command_2_Display(0xC0);  //Print to screen
        
        Write_Data_2_Display(char_to_hex(' '));
        Write_Command_2_Display(0xC0);  //Print to screen
        flag = 0;       //Set flag to 0 to count up again
      }
    }

    if (light > (ambientLight / 2) + 11.0 && light < (ambientLight / 2) + 13.0 && shadesFlag == 0){
      //Middle left (112 degrees), a "deadzone" where the servo won't turn but we still recognise that the sun is shining
      sunFlag = 1;      //Set sunflag to 1
    }
    
    if (light > (ambientLight / 2) + 13.0 && light < ambientLight - 2.0 && shadesFlag == 0){
      //Left (135 degrees)
      flag++;   //Count up flag
      sunFlag = 1;      //Set sunflag to 1
      if (flag > 15){   //Check if flag has incremented 15 times (to make the servo motor more "smooth")
        turn_Servor(6562);       //Turn the servo for 2,5 ms
        Write_Data_2_Display(int_to_hex(1));
        Write_Command_2_Display(0xC0);  //Print to screen
      
        Write_Data_2_Display(int_to_hex(3));
        Write_Command_2_Display(0xC0);  //Print to screen
      
        Write_Data_2_Display(int_to_hex(5));
        Write_Command_2_Display(0xC0);  //Print to screen
        flag = 0;       //Set flag to 0 to count up again
      }
    }
    
    if (readKeypad() == 8){     //Check if 8 is pressed
      Set_Led(0);       //Turn of the LED
      Clear_Display();  //Clear the display

      Write_Data_2_Display(0x00);
      Write_Data_2_Display(0x00);
      Write_Command_2_Display(0x24);    //Set cursor
      
      for (int i = 0; i < lengthGHCUserStop; i++){
        Write_Data_2_Display(char_to_hex(strGHCUserStop[i]));
        Write_Command_2_Display(0xC0);  //Print text to screen                 
      }
      
      Write_Data_2_Display(0x3C);
      Write_Data_2_Display(0x00);
      Write_Command_2_Display(0x24);    //Set cursor
      
      for (int i = 0; i < lengthGHCHoursOfSun; i++){
        Write_Data_2_Display(char_to_hex(strGHCHoursOfSun[i]));
        Write_Command_2_Display(0xC0);  //Print text to screen               
      }
      
      Write_Data_2_Display(0x5A);
      Write_Data_2_Display(0x00);
      Write_Command_2_Display(0x24);    //Set the cursor
      
      printDigit((float) totalMinutesOfSun / 60.0);     //Print hours of suntime
      
      Write_Data_2_Display(0x96);
      Write_Data_2_Display(0x00);
      Write_Command_2_Display(0x24);    //Set the cursor
      
      for (int i = 0; i < lengthGHCHoursOfDark; i++){
        Write_Data_2_Display(char_to_hex(strGHCHoursOfDark[i]));
        Write_Command_2_Display(0xC0);  //Print text to screen                  
      }
      
      Write_Data_2_Display(0xB4);
      Write_Data_2_Display(0x00);
      Write_Command_2_Display(0x24);    //Set the cursor
      
      printDigit((float) (totalMinutes - totalMinutesOfSun - totalMinutesOfLED) / 60.0); //Print hours of darkness
      
      Write_Data_2_Display(0xF0);
      Write_Data_2_Display(0x00);
      Write_Command_2_Display(0x24);    //Set the cursor
      
      for (int i = 0; i < lengthGHCHoursOfLED; i++){
        Write_Data_2_Display(char_to_hex(strGHCHoursOfLED[i]));
        Write_Command_2_Display(0xC0);  //Print text to screen                   
      }
      
      Write_Data_2_Display(0x0E);
      Write_Data_2_Display(0x01);
      Write_Command_2_Display(0x24);    //Set the cursor
      
      printDigit((float) totalMinutesOfLED / 60.0);     //Print hours of LED light
      
      Write_Data_2_Display(0x4A);
      Write_Data_2_Display(0x01);
      Write_Command_2_Display(0x24);    //Set the cursor
  
      for (int i = 0; i < lengthGHC2; i++){
        Write_Data_2_Display(char_to_hex(strGHC2[i]));
        Write_Command_2_Display(0xC0);  //Print text to screen 
      }
      
      Delay(2000000);   //Delay to ensure acurate keypresses
        
      while (readKeypad() != 8){        //Wait untill 8 is pressed
      }
      
      Clear_Display();  //Clear the display
      return;
    }
    
    if (totalMinutes == 1440){  //Check if 24h has passed
      Set_Led(0);       //Turn off the LED
      Clear_Display();  //Clear the display
      
      Write_Data_2_Display(0x00);
      Write_Data_2_Display(0x00);
      Write_Command_2_Display(0x24);    //Set the cursor
      
      for (int i = 0; i < lengthGHCDayStop; i++){
        Write_Data_2_Display(char_to_hex(strGHCDayStop[i]));
        Write_Command_2_Display(0xC0);  //Print text to screen                   
      }
      
      Write_Data_2_Display(0x3C);
      Write_Data_2_Display(0x00);
      Write_Command_2_Display(0x24);    //Set the cursor            
      
      for (int i = 0; i < lengthGHCHoursOfSun; i++){
        Write_Data_2_Display(char_to_hex(strGHCHoursOfSun[i]));
        Write_Command_2_Display(0xC0);  //Print text to screen                   
      }
      
      Write_Data_2_Display(0x5A);
      Write_Data_2_Display(0x00);
      Write_Command_2_Display(0x24);    //Set the cursor
      
      printDigit((float) totalMinutesOfSun / 60.0);     //Print hours of sunlight
      
      Write_Data_2_Display(0x96);
      Write_Data_2_Display(0x00);
      Write_Command_2_Display(0x24);    //Set the cursor
      
      for (int i = 0; i < lengthGHCHoursOfDark; i++){
        Write_Data_2_Display(char_to_hex(strGHCHoursOfDark[i]));
        Write_Command_2_Display(0xC0);  //Print text to screen                   
      }
      
      Write_Data_2_Display(0xB4);
      Write_Data_2_Display(0x00);
      Write_Command_2_Display(0x24);    //Set the cursor
      
      printDigit((float) (totalMinutes - totalMinutesOfSun - totalMinutesOfLED) / 60.0); //Print hours of darkness
      
      Write_Data_2_Display(0xF0);
      Write_Data_2_Display(0x00);
      Write_Command_2_Display(0x24);    //Set the cursor
      
      for (int i = 0; i < lengthGHCHoursOfLED; i++){
        Write_Data_2_Display(char_to_hex(strGHCHoursOfLED[i]));
        Write_Command_2_Display(0xC0);  //Print text to screen                   
      }
      
      Write_Data_2_Display(0x0E);
      Write_Data_2_Display(0x01);
      Write_Command_2_Display(0x24);    //Set the cursor
      
      printDigit((float) totalMinutesOfLED / 60.0);     //Print hours of LED light
      
      Write_Data_2_Display(0x4A);
      Write_Data_2_Display(0x01);
      Write_Command_2_Display(0x24);    //Set the cursor
  
      for (int i = 0; i < lengthGHC2; i++){
        Write_Data_2_Display(char_to_hex(strGHC2[i]));
        Write_Command_2_Display(0xC0);  //Print text to screen 
      }
      
      while (readKeypad() != 8){        //Wait until 8 is pressed
      }
      
      Clear_Display();  //Clear the display
      return;   //Return
    }
  }
}

void GUI(){ // Graphical user interface
   char strGUI1[] = "Smart Greenhouse";
  int lengthGUI1 = strlen(strGUI1);
  
  char strGUI2[] = "Made by: Jacob Käki and       Douglas Jonsson Lundqvist";
  int lengthGUI2 = strlen(strGUI2);
  
  char strGUI3[] = "1: Configure date and time";
  int lengthGUI3 = strlen(strGUI3);
  
  char strGUI4[] = "2: Fast-Mode On/Off";
  int lengthGUI4 = strlen(strGUI4);
  
  char strGUI5[] = "3: Temperature recording";
  int lengthGUI5 = strlen(strGUI5);
  
  char strGUI6[] = "4: Greenhouse Controll";
  int lengthGUI6 = strlen(strGUI6);
  
  char strGUI7[] = "5: Configure temp limits";
  int lengthGUI7 = strlen(strGUI7);
  
  char strGUI8[] = "Lower temp limit: ";
  int lengthGUI8 = strlen(strGUI8);
  
  char strGUI9[] = "Upper temp limit: ";
  int lengthGUI9 = strlen(strGUI9);
  
  Clear_Display();
  
  while (1){
    int number = readKeypad();
    
    if (sysTickStatus > sysTickDelay){     //Checking if a SysTick interrupt has occured, use 32 for 30 min
      updateTime();             // simulation, use 999 for real-time simulation
    }
    
    while (number == 0){
      number = readKeypad();
      
      if (sysTickStatus > sysTickDelay){     //Checking if a SysTick interrupt has occured, use 32 for 30 min
      updateTime();             // simulation, use 999 for real-time simulation
      }
      
      //Set cursor
      Write_Data_2_Display(0x07);
      Write_Data_2_Display(0x00);
      Write_Command_2_Display(0x24);
  
      for (int i = 0; i < lengthGUI1; i++){
        Write_Data_2_Display(char_to_hex(strGUI1[i]));
        Write_Command_2_Display(0xC0);  //Print text to screen
      }
  
      Write_Data_2_Display(0x1E);
      Write_Data_2_Display(0x00);
      Write_Command_2_Display(0x24);    //Set cursor
  
      for (int i = 0; i < lengthGUI2; i++){
        Write_Data_2_Display(char_to_hex(strGUI2[i]));
        Write_Command_2_Display(0xC0);  //Print text to screen
      }
  
      Write_Data_2_Display(0x5A);
      Write_Data_2_Display(0x00);
      Write_Command_2_Display(0x24);    //Set cursor
  
      for (int i = 0; i < 30; i++){
        Write_Data_2_Display(char_to_hex('-'));
        Write_Command_2_Display(0xC0);  //Print text to screen
      }
  
      Write_Data_2_Display(0x78);
      Write_Data_2_Display(0x00);
      Write_Command_2_Display(0x24);    //Set cursor
      
      for (int i = 0; i < lengthGUI3; i++){
        Write_Data_2_Display(char_to_hex(strGUI3[i]));
        Write_Command_2_Display(0xC0);  //Print text to screen
      }
  
      Write_Data_2_Display(0x96);
      Write_Data_2_Display(0x00);
      Write_Command_2_Display(0x24);    //Set cursor
      
      for (int i = 0; i < lengthGUI4; i++){
        Write_Data_2_Display(char_to_hex(strGUI4[i]));
        Write_Command_2_Display(0xC0);  //Print text to screen
      }
      
      Write_Data_2_Display(0xB4);
      Write_Data_2_Display(0x00);
      Write_Command_2_Display(0x24);    //Set cursor
  
      for (int i = 0; i < lengthGUI5; i++){
        Write_Data_2_Display(char_to_hex(strGUI5[i]));
        Write_Command_2_Display(0xC0);  //Print text to screen
      }
      
      Write_Data_2_Display(0xD2);
      Write_Data_2_Display(0x00);
      Write_Command_2_Display(0x24);    //Set cursor
      
      for (int i = 0; i < lengthGUI6; i++){
        Write_Data_2_Display(char_to_hex(strGUI6[i]));
        Write_Command_2_Display(0xC0);  //Print text to screen
      }
  
      Write_Data_2_Display(0xF0);
      Write_Data_2_Display(0x00);
      Write_Command_2_Display(0x24);    //Set cursor
      
      for (int i = 0; i < lengthGUI7; i++){
        Write_Data_2_Display(char_to_hex(strGUI7[i]));
        Write_Command_2_Display(0xC0);  //Print text to screen
      }
      
      Write_Data_2_Display(0x2C);
      Write_Data_2_Display(0x01);
      Write_Command_2_Display(0x24);    //Set cursor
      
      for (int i = 0; i < lengthGUI8; i++){
        Write_Data_2_Display(char_to_hex(strGUI8[i]));
        Write_Command_2_Display(0xC0);  //Print text to screen
      }
      
      printDigit(lowerTempLimit);       //Print the lower temperature limit
      
      Write_Data_2_Display(0x4A);
      Write_Data_2_Display(0x01);
      Write_Command_2_Display(0x24);    //Set cursor
      
      for (int i = 0; i < lengthGUI9; i++){
        Write_Data_2_Display(char_to_hex(strGUI9[i]));
        Write_Command_2_Display(0xC0);  //Print text to screen
      }
      
      printDigit(upperTempLimit);       //Print the upper temperature limit
      
      if (fastMode == 1){       //Check if fastmode is engaged
        Write_Data_2_Display(0xB1);
        Write_Data_2_Display(0x00);
        Write_Command_2_Display(0x24);  //Set cursor
        
        Write_Data_2_Display(char_to_hex(' '));
        Write_Command_2_Display(0xC0);  //Print to screen
        
        Write_Data_2_Display(char_to_hex('O'));
        Write_Command_2_Display(0xC0);  //Print to screen
        
        Write_Data_2_Display(char_to_hex('N'));
        Write_Command_2_Display(0xC0);  //Print to screen
        
      } else if (fastMode == 0){        //Check if fastmode isn't engaged
        Write_Data_2_Display(0xB1);
        Write_Data_2_Display(0x00);
        Write_Command_2_Display(0x24);  //Set cursor
        
        Write_Data_2_Display(char_to_hex('O'));
        Write_Command_2_Display(0xC0);  //Print to screen
        
        Write_Data_2_Display(char_to_hex('F'));
        Write_Command_2_Display(0xC0);  //Print to screen
        
        Write_Data_2_Display(char_to_hex('F'));
        Write_Command_2_Display(0xC0);  //Print to screen
      }
    }
    
    if (number == 1){   //Check if 1 is pressed
      timeConfigUser(); //Goto time config user function
    }
    
    if (number == 2){   //Check if 2 is pressed
      fastModeOnOff();  //Goto fastmode switcher function
    }
    
    if (number == 3){   //Check if 3 is pressed
      tempRecorder();   //Goto temperature recorder function
    }
    
    if (number == 4){   //Check if 4 is pressed
      greenHouseControll();     //Goto greenhouse controll function
    }
    
    if (number == 5){   //Check if 5 is pressed
      tempAlarmConfig(strGUI8, strGUI9, lengthGUI8, lengthGUI9);        //Goto configure temperature alarm function
    }    
    
    Delay(2000000);     //Delay to ensure acurate button presses
  }

}

int main(){
  SystemInit(); // Disabales the Watchdog and setup the MCK
  
  NVIC_ClearPendingIRQ(sysTickStatus);                  //Clear any pending interrupts
  NVIC_SetPriority(1, 1);                               //Setup the priority for SysTick interrupt
  NVIC_EnableIRQ(2);                                    //Setup how many interrupts we handle
  
  Config();             //Configurations
  Config_Display();     //Configurations for the display
  Init_Display();       //Initialise the display
  Clear_Display();      //Clear the display
  init_clockTemp();     //Initialise the temperature sensor
  
  sysTickDelay = 999;   //Set default Sys Tick Delay
  fastMode = 0; //Fast mode is off by default
 
  GUI();        //Enter the GUI
  
  return 0;
}