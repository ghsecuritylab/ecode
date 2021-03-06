#include "pwm2.h"
#include "board_includes.h"
#include "ecode.h"

#define PWM_TIMER_FREQ      16000000

int pwm2_hw_init(void);
void pwm2_period(unsigned int hz);
void pwm2_duty(float duty);
void pwm2_period_us(int us);
void pwm2_pulsewidth_us(int us);


static struct pwm_device pwm2_dev;
static struct pwm_operations pwm2_ops;

int pwm2_init(void)
{
    pwm2_ops.init = pwm2_hw_init;
    pwm2_ops.period = pwm2_period;
    pwm2_ops.pulseduty = pwm2_duty;
    pwm2_ops.period_us = pwm2_period_us;
    pwm2_ops.pulsewidth_us = pwm2_pulsewidth_us;
    
    pwm2_dev.ops = &pwm2_ops;
    
    if(pwm_register(PWM2, &pwm2_dev)<0)
    {
        LOG_ERROR("PWM2 register error!!!");
    }
	
	pwm_period_us(PWM2, 100);
	pwm_write(PWM2, 0);
	return 0;
}

int pwm2_hw_init(void)
{
  /*************************/
  /* GPIO AF configuration */
  /*************************/
  /* Enable the peripheral clock of GPIOs */
  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOC);
  
  /* GPIO TIM3_CH1 configuration */
  LL_GPIO_SetPinMode(GPIOC, LL_GPIO_PIN_7, LL_GPIO_MODE_ALTERNATE);
  LL_GPIO_SetPinPull(GPIOC, LL_GPIO_PIN_7, LL_GPIO_PULL_DOWN);
  LL_GPIO_SetPinSpeed(GPIOC, LL_GPIO_PIN_7, LL_GPIO_SPEED_FREQ_HIGH);
  LL_GPIO_SetAFPin_0_7(GPIOC, LL_GPIO_PIN_7, LL_GPIO_AF_2);
  
  /******************************/
  /* Peripheral clocks enabling */
  /******************************/
  /* Enable the timer peripheral clock */
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM3); 
  
  /***************************/
  /* Time base configuration */
  /***************************/
  /* Set counter mode */
  /* Reset value is LL_TIM_COUNTERMODE_UP */
  LL_TIM_SetCounterMode(TIM3, LL_TIM_COUNTERMODE_UP);
  
  /* Set the pre-scaler value to have TIM3 counter clock equal to 1000 kHz */
  LL_TIM_SetPrescaler(TIM3, __LL_TIM_CALC_PSC(SystemCoreClock, PWM_TIMER_FREQ));
  
  /* Enable TIM3_ARR register preload. Writing to or reading from the         */
  /* auto-reload register accesses the preload register. The content of the   */
  /* preload register are transferred into the shadow register at each update */
  /* event (UEV).                                                             */  
  LL_TIM_EnableARRPreload(TIM3);
  
  /* Set the auto-reload value to have a counter frequency of 100 Hz */
  LL_TIM_SetAutoReload(TIM3, __LL_TIM_CALC_ARR(SystemCoreClock, LL_TIM_GetPrescaler(TIM3), 100));
  
  /*********************************/
  /* Output waveform configuration */
  /*********************************/
  /* Set output mode */
  /* Reset value is LL_TIM_OCMODE_FROZEN */
  LL_TIM_OC_SetMode(TIM3, LL_TIM_CHANNEL_CH2, LL_TIM_OCMODE_PWM1);
  
  /* Set output channel polarity */
  /* Reset value is LL_TIM_OCPOLARITY_HIGH */
  LL_TIM_OC_SetPolarity(TIM3, LL_TIM_CHANNEL_CH2, LL_TIM_OCPOLARITY_HIGH);
  
  /* Set compare value to half of the counter period (50% duty cycle ) */
  LL_TIM_OC_SetCompareCH2(TIM3, 0);
  
  /* Enable TIM3_CCR1 register preload. Read/Write operations access the      */
  /* preload register. TIM3_CCR1 preload value is loaded in the active        */
  /* at each update event.                                                    */
  LL_TIM_OC_EnablePreload(TIM3, LL_TIM_CHANNEL_CH2);
  
  /**********************************/
  /* Start output signal generation */
  /**********************************/
  /* Enable output channel 2 */
  LL_TIM_CC_EnableChannel(TIM3, LL_TIM_CHANNEL_CH2);
  
  /* Enable counter */
  //LL_TIM_DisableCounter(TIM3); 
  LL_TIM_EnableCounter(TIM3);
  
  return 0;
}


void pwm2_period(unsigned int hz)
{
    hz = PWM_TIMER_FREQ/hz;
    LL_TIM_SetAutoReload(TIM3, hz-1);
    LL_TIM_GenerateEvent_UPDATE(TIM3);
}

void pwm2_duty(float duty)
{
    unsigned int reload;
    
    reload = LL_TIM_GetAutoReload(TIM3);
    reload = (int)(reload*duty);
    if(reload!=0)
        reload -= 1;
    LL_TIM_OC_SetCompareCH2(TIM3, reload);
    LL_TIM_EnableCounter(TIM3);
}


void pwm2_period_us(int us)
{
    if(us>0xFFFF)
        us = 0xFFFF;
    LL_TIM_SetAutoReload(TIM3, us-1);
    LL_TIM_GenerateEvent_UPDATE(TIM3);
}

void pwm2_pulsewidth_us(int us)
{
    if(us > 0xFFFF)
        us = 0xFFFF;
    LL_TIM_OC_SetCompareCH2(TIM3, us);
    LL_TIM_EnableCounter(TIM3);
}
