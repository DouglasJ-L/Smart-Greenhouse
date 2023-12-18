#include "at91sam3x8.h"
#include "system_sam3x.h"
#include "delay.h"

void init_PWM(){ // Initilize the pulse width modulation
  
 *AT91C_PMC_PCER = (1 << 12);  // Peripheral enable PIOB
 *AT91C_PMC_PCER1 = (1 << 4);  // Peripheral enable PWM

 *AT91C_PIOB_PDR = (1 << 17);    // Disable PDR (Enable peripheral)
 *AT91C_PIOB_ABMR = (1 << 17);   // Peripheral Select Register
 *AT91C_PWMC_ENA = (1 << 1);  // Enable PWM-channel for CHID1

 *AT91C_PWMC_CH1_CMR = 5;  // Set to Master_CLK/32

    
 *AT91C_PWMC_CH1_CPRDR = (int)(20.0 * 2625);  // Set the period to 20ms
    
 *AT91C_PWMC_CH1_CDTYR = (int)(0.7 * 2625); // Set the cycle to 1ms
}

void servo_Init(){ // Initilize the servomotor
  *AT91C_PMC_PCER = (1<<12);  //enable PMC port B
  *AT91C_PMC_PCER1 = (1<<4);  //PCER1 is peripheral clock enable register 1 used to activate PWM clock on bit 36. page 583
  *AT91C_PIOB_PER = (1<<17);  //peripheral enable register pin 62
  *AT91C_PIOB_PDR = (1<<17);  //the peripheral control the pin REG_PIOB_PDR
  *AT91C_PIOB_ABMR = (1<<17); //activatte the peripheral B to be able to control the pin in REG_PIOB_ABSR
  *AT91C_PWMC_ENA = (1<<1); //enabling PWM_channel in pin 1
  *AT91C_PWMC_CH1_CMR = (0x5); //setting the pre-scaler to Master_CLK/32 
  *AT91C_PWMC_CH1_CPRDR = (0xCD14); //writing a value t PWM_CPRD which is 20 ms, then calculating (0,02*84000000)/32=52500 = 0xCD14
  *AT91C_PWMC_CH1_CDTYR = (180); //write a value to PWM_CDTY that is 1ms;
}

void turn_Servor(int key){  // Func to turn the servo with pressed key
  int degrees = 2625.0 * ((((key * 10) * 1.933) / 180)+ 0.7); // (t*mck)/32=2625
  (*AT91C_PWMC_CH1_CDTYR) = degrees; // CDTYR controls the servo
}

void turn_Servor1(){ // Func to turn servo with keys
  
  int key = readkeypad(); 

  if (key == 0){
    (*AT91C_PWMC_CH1_CDTYR) = (int)(1.0 / (32.0 / 84000.0));
    return;
  }
  if (key == 1){      
    turn_Servor(1);               
  }
  if (key == 2){   
    turn_Servor(2);
  }
  if (key == 3){   
    turn_Servor(3); 
  }
  if (key == 4){   
    turn_Servor(4);
  }
  if (key == 5){   
    turn_Servor(5);
  }
  if (key == 6){   
    turn_Servor(6);
  }
  if (key == 7){   
    turn_Servor(7);
  }
  if (key == 8){   
    turn_Servor(8);
  }
  if (key == 9){   
    turn_Servor(9);
  }
  if (key == 10){
    turn_Servor(10);    
  }
  if (key == 11){
    turn_Servor(11);    
  }
  if (key == 12){
    turn_Servor(12);    
  }
 // Delay(50);
 
}

