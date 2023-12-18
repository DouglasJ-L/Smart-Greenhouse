#include "system_sam3x.h"
#include "at91sam3x8.h"
#include "display.h"
#include "delay.h"

void Config_Display(){
  *AT91C_PMC_PCER = (3 << 13);  // Enable Clock ( C and D)
  *AT91C_PIOC_PER = 0x3BC;      // Enable PIOC 2 - 9
  *AT91C_PIOC_ODR = 0x3BC;      // Set pins as input
  *AT91C_PIOD_PER = (1<<2);
  *AT91C_PIOD_OER = (1<<2);
  *AT91C_PIOD_SODR = (1<<2);
  //display
  *AT91C_PMC_PCER = (3<<13);   
  *AT91C_PIOC_PER = (0xFF << 2);       
  *AT91C_PIOC_PER = (0xFF << 12);
  *AT91C_PIOC_ODR = (0xFF << 2); 
  *AT91C_PIOC_OER = (0xFF << 12);
  *AT91C_PIOC_SODR = (0x9 << 14);
  *AT91C_PIOC_SODR = (1 << 12);
  *AT91C_PIOC_PER  = 1;
  *AT91C_PIOC_OER  = 1;
}

unsigned char Read_Status_Display(void){
  unsigned char Temp;
  *AT91C_PIOC_ODR = (255 << 2);
  *AT91C_PIOC_SODR = (1 << 13);
  *AT91C_PIOC_CODR = (1 << 12);
  *AT91C_PIOC_SODR = (1 << 14);
  *AT91C_PIOC_CODR = (1 << 15);
  *AT91C_PIOC_CODR = (1 << 16);
  Delay(25);
  Temp = (*AT91C_PIOC_PDSR >> 2);
  *AT91C_PIOC_SODR = (1 << 15);
  *AT91C_PIOC_SODR = (1 << 16);
  *AT91C_PIOC_SODR = (1 << 12);
  *AT91C_PIOC_CODR = (1 << 13);

  return Temp;
}

void Write_Command_2_Display(unsigned char Command){
 while(1){ 
      if((Read_Status_Display() & (3 << 0)) == (3 << 0)){     
        *AT91C_PIOC_CODR = (255 << 2);
        *AT91C_PIOC_SODR = (Command << 2);
        *AT91C_PIOC_CODR = (1 << 13);
        *AT91C_PIOC_CODR = (1 << 12);
        *AT91C_PIOC_OER = (255 << 2);
        *AT91C_PIOC_SODR = (1 << 14);
        *AT91C_PIOC_CODR = (1 << 15);
        *AT91C_PIOC_CODR = (1 << 17);
        Delay(25);
        *AT91C_PIOC_SODR = (1 << 15);
        *AT91C_PIOC_SODR = (1 << 17);
        *AT91C_PIOC_SODR = (1 << 12);
        *AT91C_PIOC_ODR = (255 << 2);
        break; 
    }
  }
}

void Write_Data_2_Display(unsigned char Data){
   while(1){
    if((Read_Status_Display() & (3 << 0)) == (3 << 0)){
      *AT91C_PIOC_CODR = (255 << 2);
      *AT91C_PIOC_SODR = (Data << 2);
      *AT91C_PIOC_CODR = (1 << 13);
      *AT91C_PIOC_CODR = (1 << 12);
      *AT91C_PIOC_OER = (255 << 2);
      *AT91C_PIOC_CODR = (1 << 14);
      *AT91C_PIOC_CODR = (1 << 15);
      *AT91C_PIOC_CODR = (1 << 17);
      Delay(25);
      *AT91C_PIOC_SODR = (1 << 15);
      *AT91C_PIOC_SODR = (1 << 17);
      *AT91C_PIOC_SODR = (1 << 12);
      *AT91C_PIOC_ODR = (255 << 2);
      break;
    }
  }
}

void Init_Display(void){ 
  *AT91C_PIOD_CODR = (1<<0);            
  Delay(10);                            
  *AT91C_PIOD_SODR = (1<<0);           

  Write_Data_2_Display(0x00);
  Write_Data_2_Display(0x00);
  Write_Command_2_Display(0x40);//Set text home address
  Write_Data_2_Display(0x00);
  Write_Data_2_Display(0x40);
  Write_Command_2_Display(0x42); //Set graphic home address
  Write_Data_2_Display(0x1e);
  Write_Data_2_Display(0x00);
  Write_Command_2_Display(0x41); // Set text area
  Write_Data_2_Display(0x1e);
  Write_Data_2_Display(0x00);
  Write_Command_2_Display(0x43); // Set graphic area
  Write_Command_2_Display(0x80); // text mode
  Write_Command_2_Display(0x94); // Text on graphic o
}

void Clear_Display(){
  Write_Data_2_Display(0x00);
  Write_Command_2_Display(0x24);
 
  for (int i = 0; i < (16*40); i++){ // halva skärmen
    Write_Data_2_Display(0x00);
    Write_Command_2_Display(0xC0);
  }
  Write_Data_2_Display(0x00);
  Write_Command_2_Display(0x24);
}

void Clear_Display1(){
  Write_Data_2_Display(0x00);
  Write_Command_2_Display(0x24);
 
  for (int i = 0; i < (321); i++){ // halva skärmen
    Write_Data_2_Display(0x00);
    Write_Command_2_Display(0xC0);
  }
  Write_Data_2_Display(0x00);
  Write_Command_2_Display(0x24);
  
}

void Clear_Display2(){
  Write_Data_2_Display(0x00);
  Write_Command_2_Display(0x24);
 
  for (int i = 0; i < (321); i++){ // halva skärmen
    Write_Data_2_Display(0x00);
    Write_Command_2_Display(0xC0);
  }
  Write_Data_2_Display(0x00);
  Write_Command_2_Display(0x24);
  
  
}

void Auto_Write(unsigned char Data){
	while((Read_Status_Display()&8)!=8)asm("NOP"); //wait for an OK
	*AT91C_PIOC_CODR = 0xFF<<2; // Clear databus
	*AT91C_PIOC_SODR = Data<<2; // Set Data to databus
	setBus(3); //Set dir as output & Enable output (74chip)
	*AT91C_PIOC_OER = 0xFF<<2; // Set databus as output
	*AT91C_PIOC_CODR = 1<<17; // Set C/D signal Low (0 = Data)
	*AT91C_PIOC_CODR = 1<<16|1<<14; // Clear chip select display & write display
	Delay(2);
	*AT91C_PIOC_SODR = 1<<16|1<<14; // Set chip enable display & write display
	*AT91C_PIOC_ODR = 0xFF<<2; // Make databus as input
	setBus(4); // Disable output (74chip)
}

int Write_Text(unsigned char text[]){
	int blanks = 0;
	int i = 0;
	Write_Command_2_Display(0xB0); // Auto Write
	while(text[i]!= '\0'){
		if(text[i]== '\n'){
			for(int temp=30-(i+blanks)%30; temp>0; temp--){
				blanks++;
			}
			blanks--;	// since \n counts as a char
		}
		i++;
	}
	Write_Command_2_Display(0xB2); // auto reset
	return i+blanks;
}

int Write_Display(unsigned char text[]){
	//Set_Adress Pointer(x,y);
        Write_Data_2_Display(0x00); // set cursor
        Write_Data_2_Display(0x00);
        Write_Command_2_Display(0x24);
        
	return Write_Text(text);
}
