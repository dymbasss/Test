#include "zdo_header_for_led.h"
#include "stm32f4xx_tim.h"

void toggle_color(zb_uint8_t color);
void increase_brightness(zb_uint8_t color);
void udpate_led_state();
void delay_mc(zb_uint8_t mc);

static volatile zb_bool_t led_state_red = ZB_FALSE;
static volatile zb_bool_t led_state_green = ZB_FALSE;
static volatile zb_bool_t led_state_blue = ZB_FALSE;

static zb_uint16_t red_pwm_value;
static zb_uint16_t green_pwm_value;
static zb_uint16_t blue_pwm_value;

//------------------------------------------------------------------------

void init_led(void) // RGB_LED
{
  GPIO_InitTypeDef  led_init_struct;
  
  /* Enable peripheral clock for led port */
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
  
  GPIO_PinAFConfig(GPIOB, GPIO_PinSource6, GPIO_AF_TIM4);
  GPIO_PinAFConfig(GPIOB, GPIO_PinSource7, GPIO_AF_TIM4);
  GPIO_PinAFConfig(GPIOB, GPIO_PinSource8, GPIO_AF_TIM4);
  
  /* Init leds */
  led_init_struct.GPIO_Pin = L_PIN_RED | L_PIN_GREEN | L_PIN_BLUE;
  led_init_struct.GPIO_OType = GPIO_OType_PP;
  led_init_struct.GPIO_Mode = GPIO_Mode_AF;
  led_init_struct.GPIO_PuPd = GPIO_PuPd_NOPULL;
  led_init_struct.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_Init(GPIOB, &led_init_struct);

  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
  
  /* Init leds */
  led_init_struct.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14;
  led_init_struct.GPIO_Mode = GPIO_Mode_OUT;
  led_init_struct.GPIO_OType = GPIO_OType_PP;
  led_init_struct.GPIO_PuPd = GPIO_PuPd_NOPULL;
  led_init_struct.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_Init(GPIOD, &led_init_struct);
  // GPIO_SetBits(GPIOD,GPIO_Pin_12 | GPIO_Pin_14 | GPIO_Pin_15);
}

//-----------------------------------------------------------------------

void init_timer_pwm(void)
{
  TIM_TimeBaseInitTypeDef  t_init_struct;
  TIM_OCInitTypeDef oc_init_struct;

  TIM_TimeBaseStructInit(&t_init_struct);

  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
  /* Init timer */
  t_init_struct.TIM_Period = PERIOD;
  t_init_struct.TIM_Prescaler = 10;
  t_init_struct.TIM_CounterMode = TIM_CounterMode_Up;
  t_init_struct.TIM_ClockDivision = TIM_CKD_DIV1;
  // Inictialize TIM4
  TIM_TimeBaseInit(TIM4, &t_init_struct);
  
  oc_init_struct.TIM_Pulse = 0;    // ((timer_period + 1) * DUTY_CYCLE) / 100 - 1
  oc_init_struct.TIM_OCMode = TIM_OCMode_PWM1;
  oc_init_struct.TIM_OutputState = TIM_OutputState_Enable;
  oc_init_struct.TIM_OCPolarity = TIM_OCPolarity_Low;

  TIM_OC1Init(TIM4, &oc_init_struct);
  TIM_OC1PreloadConfig(TIM4, TIM_OCPreload_Enable);
  TIM_OC2Init(TIM4, &oc_init_struct);
  TIM_OC2PreloadConfig(TIM4, TIM_OCPreload_Enable);
  TIM_OC3Init(TIM4, &oc_init_struct);
  TIM_OC1PreloadConfig(TIM4, TIM_OCPreload_Enable);

  // Start TIM4
  TIM_Cmd(TIM4, ENABLE);
}

//-----------------------------------------------------------------------

void delay_ms(uint8_t ms)
{
  volatile uint32_t nCount;
  RCC_ClocksTypeDef RCC_Clocks;
  RCC_GetClocksFreq (&RCC_Clocks);
    
  nCount = (RCC_Clocks.HCLK_Frequency/168000) * ms;
  while(nCount != 0)
    {
      nCount--;
    }
}

void update_led_state()
{
  if (led_state_red == ZB_TRUE)
    {
      TIM4->CCR1 = red_pwm_value;
      delay_ms(1);
    }
  else
    {
      TIM4->CCR1 = 0;
    }

  if (led_state_green == ZB_TRUE)
    {
      TIM4->CCR2 = green_pwm_value;
      delay_ms(1);
    }
  else
    {
      TIM4->CCR2 = 0;
    }
  
  if (led_state_blue == ZB_TRUE)
    {
      TIM4->CCR3 = blue_pwm_value;
      delay_ms(1);
    }
  else 
    {
      TIM4->CCR3 = 0;
    }
}

void toggle_color(zb_uint8_t color)
{
  switch(color)
    {
    case COLOR_RED:
      led_state_red = !led_state_red;
      break;
    case COLOR_GREEN:
      led_state_green = !led_state_green;
      break;
    case COLOR_BLUE:
      led_state_blue = !led_state_blue;
      break;
    }
  
  update_led_state();
}

void increase_brightness(zb_uint8_t color)
{
  if (color == COLOR_RED && led_state_red == ZB_TRUE)
    {
      red_pwm_value += PERIOD * 10 /100;
      if (red_pwm_value >= PERIOD)
	{
	  red_pwm_value = 0;
	}
    }
      
  if (color == COLOR_GREEN && led_state_green == ZB_TRUE)
    {
      green_pwm_value += PERIOD * 10 /100;
      if (green_pwm_value >= PERIOD)
	{
	  green_pwm_value = 0;
	}
    }
      
  if (color == COLOR_BLUE && led_state_blue == ZB_TRUE)
    {
      blue_pwm_value += PERIOD * 10 /100;
      if (blue_pwm_value >= PERIOD)
	{
	  blue_pwm_value = 0;
	}
    }
  
  update_led_state();
}


