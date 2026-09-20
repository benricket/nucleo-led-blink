
#include <stdint.h>

#define RCC_BASE 0x40023800U
#define TIM2_BASE  0x40000000U
#define GPIOA_BASE 0x40020000U
#define GPIOC_BASE 0x40020800U
#define SYSCFG_BASE 0x40013800U
#define EXTI_BASE 0x40013C00U

#define RCC_AHB1ENR *(volatile uint32_t *)(RCC_BASE + 0x30)
#define RCC_APB1ENR *(volatile uint32_t *)(RCC_BASE + 0x40)
#define RCC_APB2ENR *(volatile uint32_t *)(RCC_BASE + 0x44)

#define TIM2_CR1 *(volatile uint32_t *)(TIM2_BASE  + 0x00)
#define TIM2_CCMR1 *(volatile uint32_t *)(TIM2_BASE  + 0x18)
#define TIM2_CCER *(volatile uint32_t *)(TIM2_BASE + 0x20)
#define TIM2_PSC *(volatile uint32_t *)(TIM2_BASE + 0x28)
#define TIM2_ARR *(volatile uint32_t *)(TIM2_BASE + 0x2C)
#define TIM2_CCR1 *(volatile uint32_t *)(TIM2_BASE + 0x34)
#define TIM2_CNT *(volatile uint32_t *)(TIM2_BASE + 0x24)
#define TIM2_DIER *(volatile uint32_t *)(TIM2_BASE + 0x0C)
#define TIM2_SR *(volatile uint32_t *)(TIM2_BASE + 0x10)

#define GPIOA_MODER *(volatile uint32_t *)(GPIOA_BASE + 0x00)
#define GPIOA_PUPDR *(volatile uint32_t *)(GPIOA_BASE + 0x0C)
#define GPIOA_AFRL *(volatile uint32_t *)(GPIOA_BASE + 0x20)
#define GPIOA_AFRH *(volatile uint32_t *)(GPIOA_BASE + 0x24)
#define GPIOA_ODR *(volatile uint32_t *)(GPIOA_BASE + 0x14)

#define GPIOC_MODER *(volatile uint32_t *)(GPIOC_BASE + 0x00)
#define GPIOC_PUPDR *(volatile uint32_t *)(GPIOC_BASE + 0x0C)
#define GPIOC_AFRL *(volatile uint32_t *)(GPIOC_BASE + 0x20)
#define GPIOC_AFRH *(volatile uint32_t *)(GPIOC_BASE + 0x24)

#define SYSCFG_EXTICR4 *(volatile uint32_t *)(SYSCFG_BASE + 0x14)

#define EXTI_IMR *(volatile uint32_t *)(EXTI_BASE + 0x0)
#define EXTI_RTSR *(volatile uint32_t *)(EXTI_BASE + 0x08)
#define EXTI_FTSR *(volatile uint32_t *)(EXTI_BASE + 0x0C)
#define EXTI_PR *(volatile uint32_t *)(EXTI_BASE + 0x14)

#define NVIC_BASE 0xE000E100
#define NVIC_ISER0 *(volatile uint32_t *)(NVIC_BASE + 0x00)
#define NVIC_ISER1 *(volatile uint32_t *)(NVIC_BASE + 0x04)

#define PSC_VAL 15999UL
#define ARR_VAL 124UL // corresponds to period of 0.25s = 4 Hz

volatile uint32_t count = 0;
volatile uint32_t answer = 0;
volatile uint32_t counter = 0;
volatile uint32_t button_mode = 0;

int main(void)
{
	// Enable things
	RCC_AHB1ENR |= (1U << 0); // GPIOA
	RCC_AHB1ENR |= (1U << 2); // GPIOC
	RCC_APB1ENR |= (1U << 0); // TIM2 EN
	RCC_APB2ENR |= (1U << 14); // SYSCFG EN

	TIM2_CCMR1 &= ~(0x7U << 4);
	TIM2_CCMR1 |= (0x6U << 4); // CCMR1 to PWM1
	TIM2_CCER |= (1U << 0); // CC1
	TIM2_PSC = (PSC_VAL); // PSC fills the whole reg, so we can direct assign
	TIM2_ARR = (ARR_VAL); // ARR fills the whole register, so we can direct assign
	TIM2_CR1 |= (1U << 0); // CEN
	TIM2_DIER |= (1U << 0);

	NVIC_ISER0 |= (1U << 28); // Enable TIM2 interrupt (position 28) in NVIC

//	GPIOA_MODER &= ~(0x3U << 2 * 5);
//	GPIOA_MODER |= (0x2U << 2 * 5); // AF mode on PA5
//	GPIOA_AFRL &= ~(0xFU << 5 * 4);
//	GPIOA_AFRL |= (1U << 5 * 4); // AF mode 1 (TIM2_CH1) for PA5
	GPIOA_MODER &= ~(0x3U << (5 * 2));
	GPIOA_MODER |=  (0x1U << (5 * 2));   // output mode on PA5
	GPIOC_MODER &= ~(0x3U << 13 * 2); // Input mode on PC13

	EXTI_IMR |= (1U << 13); // Stop masking interrupts on EXTI13
	EXTI_RTSR |= (1U << 13); // rising edge interrupts on EXTI13
	EXTI_FTSR &= ~(1U << 13);

	SYSCFG_EXTICR4 &= ~(0xFU << 4);
	SYSCFG_EXTICR4 |= (2U << 4); // EXTI13 should listen to port C

	NVIC_ISER1 |= (1U << 8); // Enable EXTI15_10 interrupt (position 40) in NVIC


  while (1)
  {
	  // Do nothing here; everything is interrupts
  }
}

void TIM2_IRQHandler(void)
{
    if (TIM2_SR & (1U << 0))
    {
        if (button_mode == 2)
        {
            GPIOA_ODR ^= (1U << 5); // Flip LED state
        }
        TIM2_SR &= ~(1U << 0);
    }
}


void EXTI15_10_IRQHandler(void)
{
    if (EXTI_PR & (1U << 13))
    {
        button_mode++;
        if (button_mode > 2) button_mode = 0;
        switch (button_mode) {
        case 0:
        	// Turn off the LED
        	GPIOA_ODR &= ~(1U << 5);
        	break;
        case 1:
        	// Turn on the LED
        	GPIOA_ODR |= (1U << 5);
        	break;
        case 2:
        	// Don't trigger LED (handled by TIM2 interrupt
        	break;
        }
        EXTI_PR |= (1U << 13);
    }
}
