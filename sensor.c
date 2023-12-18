#include "at91sam3x8.h"
#include "system_sam3x.h"

void init_ADC();
float measure_ADC();

float volt = 0;  

void init_ADC(){ // initilize the ADC
   *AT91C_PMC_PCER1 = (1 << 5); // Enable peripheral for ADC
   *AT91C_PIOA_PER = (1<<4); // enable pin               
   *AT91C_PIOA_PPUDR = (1<<4); // disable pull up register
   *AT91C_PIOA_PER = (1<<3); // enable pin
   *AT91C_PIOA_PPUDR = (1<<3); // disable pull up register
   *AT91C_ADCC_MR = (2 << 8);  //(32000000/((2+1)*2));           
   //*AT91C_ADCC_MR -> MCK / 6;
}

float measure_ADC(){ // func to measure the voltage from sensor
  *AT91C_ADCC_CHER = ( 1 << 1); //Select channel 1
  *AT91C_ADCC_CR= (1 << 1); //Start ADC
  
 // *AT91C_ADCC_MR = (1<<24); 
  while (1)  
    if((*AT91C_ADCC_SR & (1 << 24)) == (1 << 24)){
        //light = (*AT91C_ADCC_LCDR & 0xFFF)*(3.3/4096); //Read value in ADC_LCDR, 3.2991 is max value?
        volt = (*AT91C_ADCC_LCDR & 0xFFF)*(3.3/4096);
        //return volt1;
        break;
    }
  return volt;
}  
//return (volt2/(volt2+volt1))*180;för att servorn ska få värde att tolka