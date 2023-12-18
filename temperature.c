#include "system_sam3x.h"
#include "at91sam3x8.h"
#include "display.h"
#include "delay.h"

volatile int flag = 0, flag2 = 0;
//double voltage = 0;
double temperature = 0;
double minValue = 0;
extern float volt;
int val = -1;
int status = 0, tdel=0, del=0;

char hexa_Key(int key){ //return hexa value
    
  if (key == 1){
      return 0x11;
    }
    if (key == 2){
      return 0x12;
    }
    if (key == 3){
      return 0x13;
    }
    if (key == 4){
      return 0x14;
    }
    if (key == 5){
      return 0x15;
    }
    if (key == 6){
      return 0x16;
    }
    if (key == 7){
      return 0x17;
    }
    if (key == 8){
      return 0x18;
    }
    if (key == 9){
      return 0x19;
    }
    if (key == 10){
      return 0x0A;
    }
    if (key == 11){
      return 0x10;
    }
    if (key == 12){
      return 0x03;
    }
  return 0x00;
}

void write_Number (char key){ //func used to return different excepctions
  Write_Data_2_Display(key);
  Write_Command_2_Display(0xC0);
}

void write_Key_Display(int key){ //func used in printdigit
  val = hexa_Key(key);
  write_Number(hexa_Key(key));
}

void printDigit(double num){ //func to print temp and voltage values
  Write_Data_2_Display(0x00); // columns
  Write_Data_2_Display(0x00); // rows
  Write_Command_2_Display(0x24);
  if (num == 0)
      return;
    //To store the digitof the number N
    //printf("%f\n", num);
    int arr[100];
    int i = 0;
    int j, r;
    
    int N = (int) (num*10000);
   // printf("%i\n", N);
    // Till N becomes 0
    while (N != 0) {
  
        // Extract the last digit of N
        r = N % 10;
  
        // Put the digit in arr[]
        arr[i] = r;
        i++;
  
        // Update N to N/10 to extract
        // next last digit
        N = N / 10;
    }

    // Print the digit of N by traversing arr[] reverse
    int temp = 0;
    if (num < 1){
      write_Key_Display(11);
      write_Number(0x0C); // (,)
    }
    for (j = i - 1; j > -1; j--){
       
        /*if (temp == 2 && num > 1){ // print dec value of temp 
          write_Number(0x0C);         
        }*/
        if (temp == 1 && num > 1){ // print dec value of voltage
            write_Number(0x0C);
        }   
        if (arr[j] == 0)
            write_Key_Display(11);
        else write_Key_Display(arr[j]);  
       // printf("temp: %d\n", temp);
        temp++;       
    }
    write_Number(0x00); // blankt rum 
   
} 

void init_clockTemp(){ // Start the clk for temp
  *AT91C_PMC_PCER = (1 << 12); // Enable PIOB
  *AT91C_PMC_PCER = (1 << 11); // Enable PIOA
  *AT91C_PMC_PCER = (1 << 27); // Enable Time counter 0

  *AT91C_TC0_CMR = (*AT91C_TC0_CMR & 0xFFF0FFF8)|(1 << 17)|(1<<18);  //Select Timer_Clock1 as TCCLK
  
  *AT91C_PMC_PCER = (1 << 27); // Enable Timer Counter 1
  *AT91C_PMC_PCER = (1 << 28); // Enable Timer Counter 2 
  
  *AT91C_TC0_CCR = 0x5; // Enable counter and make a sw_reset in TC0_CCR0  
  
  *AT91C_PIOB_PER = (1 << 25); // Enable pin       
  
  *AT91C_PIOB_PPUDR = (1 << 25); // Disable pull up register
  *AT91C_PIOB_OER = (1 << 25); // Output enable register
  
  *AT91C_PIOB_SODR = (1 << 25); // Check status for pin
  
  *AT91C_PIOB_CODR = (1 << 25); //Clear output for pin      
  del=1;
  while(del);
  
  *AT91C_PIOB_SODR = (1 << 25); // Check status for pin
  
  NVIC_ClearPendingIRQ(TC0_IRQn); //Enable the interrupt (NVIC) with the inline declared function
  NVIC_SetPriority(TC0_IRQn, 2);     
  NVIC_EnableIRQ(TC0_IRQn); 
  *AT91C_TC0_SR;
  
}                         
  
void measure_Temp(){ // measure temp func   
  *AT91C_PIOB_OER     = (1<<25); // StartPulse
  *AT91C_PIOB_SODR =    (1<<25); //Enable the pin
  
  *AT91C_PIOB_CODR = (1 << 25); //Enable the pin
  del=1;
  while(del);
    
  *AT91C_PIOB_SODR = (1 << 25); 
  Delay(25);
  *AT91C_PIOB_CODR = (1 << 25); 
                                         
  Delay(25);                            
  *AT91C_PIOB_SODR = (1 << 25);
  
  *AT91C_PIOB_ODR = (1<<25);   
  
  *AT91C_TC0_CCR = (1 << 2); // sw_reset in TC0_CCR0
  
  *AT91C_TC0_SR; //Clear old interrupt by read statusreg / For B 
  
  *AT91C_TC0_IER = (1<<6); //Enable interrupt for LDRBS
}

void TC0_Handler(){ //Interrupt handler
  *AT91C_TC0_IDR = (1 << 6); // Disable interrupt LDRBS
  flag = 1; 
  
    Delay(5);
 
    int Rb = (*AT91C_TC0_RB);
    int Ra = (*AT91C_TC0_RA);
    *AT91C_TC0_IDR = AT91C_TC_LDRBS; //Disable interrupt TC_IER_LDRBS (1<<6)
 
    AT91C_BASE_TC0->TC_SR;
    int time=Rb-Ra;
    if(time > 0x200){

      temperature=(double)((((time * (1/42.0))/5.0)-273.15)-3);
    }
}

