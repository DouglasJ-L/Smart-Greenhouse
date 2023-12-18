#include "at91sam3x8.h"
#include "system_sam3x.h"

void Delay(int Value){
    for (int i=0;i<Value;i++)
         asm("nop");
}