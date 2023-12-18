#include "system_sam3x.h"
#include "at91sam3x8.h"

int value;
int column [] = {256, 128, 512}; // R1 = 256 (PC7), R2 = 128 (PC8), R3 = 512 (PC9)
int row [] = {32, 4, 8, 16}; // R1 = 32 (PC5), R2 = 4 (PC2), R3 = 8 (PC3), R4 = 16 (PC4)

int readKeypad(void){   //Read the keypad
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
