#include "zdo_header_for_led.h"

//-----------------------------------------------------------------------

void init_button(void) // BUTTON L & R
{
  GPIO_InitTypeDef b_init_struct;
  NVIC_InitTypeDef b_nvic_init_struct;
  EXTI_InitTypeDef b_exti_init_struct;
  
   /* Enable peripheral clock for buttons port */
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);
  /* Init button */
  b_init_struct.GPIO_Pin = B_LEFT | B_RIGHT;
  b_init_struct.GPIO_Mode = GPIO_Mode_IN;
  b_init_struct.GPIO_OType = GPIO_OType_PP;
  b_init_struct.GPIO_Speed = GPIO_Speed_2MHz;
  b_init_struct.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_Init(GPIOE, &b_init_struct);

  RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
  SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOE, EXTI_PinSource0);
  b_exti_init_struct.EXTI_Line = EXTI_Line0;
  b_exti_init_struct.EXTI_LineCmd = ENABLE;
  b_exti_init_struct.EXTI_Mode = EXTI_Mode_Interrupt;
  b_exti_init_struct.EXTI_Trigger = EXTI_Trigger_Rising;
  EXTI_Init(&b_exti_init_struct);

  SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOE, EXTI_PinSource1);
  b_exti_init_struct.EXTI_Line = EXTI_Line1;
  EXTI_Init(&b_exti_init_struct);
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, DISABLE);
	    
  b_nvic_init_struct.NVIC_IRQChannel = EXTI0_IRQn;
  b_nvic_init_struct.NVIC_IRQChannelPreemptionPriority = 0x00;
  b_nvic_init_struct.NVIC_IRQChannelSubPriority = 0x00;
  b_nvic_init_struct.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&b_nvic_init_struct);

  b_nvic_init_struct.NVIC_IRQChannelSubPriority = 0x01;
  b_nvic_init_struct.NVIC_IRQChannel = EXTI1_IRQn;
  NVIC_Init(&b_nvic_init_struct);
}

//-----------------------------------------------------------------------



