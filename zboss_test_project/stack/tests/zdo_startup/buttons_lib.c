#include "zdo_header_for_button.h"
#include "stm32f4xx_tim.h"

void send_led_command(zb_uint8_t param);

zb_callback_t right_button_callback;
zb_callback_t left_button_callback;
zb_callback_t double_click_callback;

volatile zb_bool_t button_state_left = ZB_FALSE;
volatile zb_bool_t button_state_right = ZB_FALSE;

//-----------------------------------------------------------------------

void init_timer(void)
{
  TIM_TimeBaseInitTypeDef  t_init_struct;
  NVIC_InitTypeDef t_nvic_init_struct;
  
  /* Enable peripheral clock for timer */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
  /* Init timer */
  t_init_struct.TIM_Period = 100000 - 1;
  t_init_struct.TIM_Prescaler = 84 - 1;
  t_init_struct.TIM_ClockDivision = 0;
  t_init_struct.TIM_CounterMode = TIM_CounterMode_Up;
  TIM_TimeBaseInit(TIM2, &t_init_struct);
  /*Interruption on up-dating*/
  TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
  /* On */ 
  //TIM_Cmd(TIM2, ENABLE);

  t_nvic_init_struct.NVIC_IRQChannel = TIM2_IRQn;
  t_nvic_init_struct.NVIC_IRQChannelPreemptionPriority = 0;
  t_nvic_init_struct.NVIC_IRQChannelSubPriority = 1;
  t_nvic_init_struct.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&t_nvic_init_struct);
}

void state_tim_cmd(zb_uint8_t state)
{
  switch(state)
    {
    case TIM_CMD_DISABLE:
      TIM_Cmd(TIM2, DISABLE);
      break;
    case TIM_CMD_ENABLE:
      TIM_Cmd(TIM2, ENABLE);
      break;
    }
}

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

void set_double_click_handler(zb_callback_t func)
{
  double_click_callback = func;
}

void set_left_button_handler(zb_callback_t func)
{
  left_button_callback = func;
}

void set_right_button_handler(zb_callback_t func)
{
  right_button_callback = func;
}

void EXTI0_IRQHandler(void)
{
  zb_uint8_t read_pin_0, cycle;
  EXTI_ClearITPendingBit(EXTI_Line0);
  read_pin_0 = GPIO_ReadInputDataBit(GPIOE, B_LEFT);

  while(read_pin_0 == 1)
    {
      read_pin_0 = GPIO_ReadInputDataBit(GPIOE, B_LEFT);
      
      if(cycle == 50)
	{
	  button_state_left = ZB_TRUE;
	  state_tim_cmd(1);
	  break;
	}
      cycle++;
    }
  cycle = 0;
}

void EXTI1_IRQHandler(void)
{  
  zb_uint8_t read_pin_1, cycle;
  EXTI_ClearITPendingBit(EXTI_Line1);
  read_pin_1 = GPIO_ReadInputDataBit(GPIOE, B_RIGHT);
  
  while(read_pin_1 == 1)
    {
      read_pin_1 = GPIO_ReadInputDataBit(GPIOE, B_RIGHT);
      
      if(cycle == 50)
	{
	  button_state_right = ZB_TRUE;
	  state_tim_cmd(1);
   	  break;
	}
      cycle++;
    }
  cycle = 0;
}

void TIM2_IRQHandler(void)
{
  if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET)
    {
      TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
      
      send_led_command(0);
    }
}

void send_led_command(zb_uint8_t param)
{
  if (button_state_left == ZB_TRUE && button_state_right == ZB_TRUE)
    {
      button_state_left = ZB_FALSE;
      button_state_right = ZB_FALSE;
      ZB_SCHEDULE_ALARM(double_click_callback, param, ZB_MILLISECONDS_TO_BEACON_INTERVAL(100));
    }
  
  if (button_state_left == ZB_FALSE && button_state_right == ZB_TRUE)
    {
      button_state_right = ZB_FALSE;
      ZB_SCHEDULE_ALARM(right_button_callback, param, ZB_MILLISECONDS_TO_BEACON_INTERVAL(100));
    }
      
  if (button_state_left == ZB_TRUE && button_state_right == ZB_FALSE)
    {
      button_state_left = ZB_FALSE;
      ZB_SCHEDULE_ALARM(left_button_callback, param, ZB_MILLISECONDS_TO_BEACON_INTERVAL(100));
    }

  state_tim_cmd(0);
}

