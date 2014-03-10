/**************************************
 * stm32 random numbers example main.c
 **************************************/
 
#include "stm32f10x.h"
#include "entropy.h"

#define NULL (0)
 
/* User defined function prototypes */
void USART1_Init(void);
void USART1_PrintStr(const char *str);
void USART1_PrintHex(uint32_t val);
void RTC_UserInit(void);
void SysTick_UserInit(void);
static inline void Delay_us(uint32_t us);

#define Delay_ms(x) Delay_us(1000*x)
#define Delay_s(x) Delay_us(1000000*x)

static EntropyPool EP;

int main(void)
{
    /* Init peripherals */
    USART1_Init();
    RTC_UserInit();
    SysTick_UserInit();

    /* Init entropy pool */
    Entropy_Init(&EP);

    /* Allow pool to fill */
    Delay_ms(65);
    
    while (1)
    {
	uint32_t rand;
        uint32_t ret = Entropy_GetRandom(&EP, &rand);

	if (ret == 0)
	    USART1_PrintHex(rand);
	else
	    USART1_PrintStr("Could not get random number.\n");

	Delay_ms(10);
    }
}   

/*****************************************************
 * Initialize RTC
 *****************************************************/
void RTC_UserInit(void)
{
    NVIC_InitTypeDef NVIC_InitStructure;

    /* Configure one bit for preemption priority */
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
    
    /* Enable the RTC Interrupt */
    NVIC_InitStructure.NVIC_IRQChannel = RTC_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    /* Enable clock for RTC */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_BKP | RCC_APB1Periph_PWR, ENABLE);

    /* Enable RTC register access */
    PWR_BackupAccessCmd(ENABLE);

    /* Reset Backup Domain */
    BKP_DeInit();

    /* Enable LSI clock */
    RCC_LSICmd(ENABLE);

    /* Wait till LSI is ready */
    while (RCC_GetFlagStatus(RCC_FLAG_LSIRDY) == RESET);

    /* Select LSI as RTC clock */
    RCC_RTCCLKConfig(RCC_RTCCLKSource_LSI);

    /* Enable RTC */
    RCC_RTCCLKCmd(ENABLE);

    /* Wait for RTC registers synchronization */
    RTC_WaitForSynchro();

    /* 1ms pre-scalar */
    RTC_WaitForLastTask();
    RTC_SetPrescaler(39);

    /* Zero out counter */
    RTC_WaitForLastTask();
    RTC_SetCounter(0);
    RTC_WaitForLastTask();

    /* Pre-scaler overflow interrupt */
    RTC_WaitForLastTask();
    RTC_ITConfig(RTC_IT_SEC, ENABLE);
    
    /* Enable RTC interrupt */
    NVIC_EnableIRQ(RTC_IRQn);
}

/*****************************************************
 * SysTick config
 *****************************************************/
void SysTick_UserInit(void)
{
    /* Select HCLK as SysTick clock source */
    SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK);
    
    /* set reload register */
    SysTick->LOAD  = (0xFFFFFF & SysTick_LOAD_RELOAD_Msk) - 1;
    
    /* Load the SysTick Counter Value */
    SysTick->VAL   = 0;
    
    /* Enable SysTick Timer */
    SysTick->CTRL  = SysTick_CTRL_CLKSOURCE_Msk | 
	             SysTick_CTRL_ENABLE_Msk;
}

/*****************************************************
 * Print helper functions
 *****************************************************/
void USART1_PrintHex(uint32_t val)
{
    int chunk = 8;
    uint32_t digit;

    USART1_PrintStr("0x");
    
    while (chunk > 0)
    {
        digit = val >> ((chunk-1) * 4);
        digit &= 0xF;
        
        while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
        if (digit < 10)
            USART_SendData(USART1, digit + '0');
        else
            USART_SendData(USART1, digit-10 + 'a');
        
        chunk--;
    }

    while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);    
    USART_SendData(USART1, '\n');
}

void USART1_PrintStr(const char *str)
{
    char *ptr = (char*)str;
    while (ptr != NULL && *ptr != '\0')
    {
        while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
        USART_SendData(USART1, (uint16_t)(*ptr));
        ptr++;
    }
}

/*****************************************************
 * Initialize USART1 for transmission only
 *****************************************************/
void USART1_Init(void)
{
    USART_InitTypeDef usart1_init_struct;
    GPIO_InitTypeDef gpioa_init_struct;
     
    /* Enalbe clock for USART1, AFIO and GPIOA */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_AFIO | 
                           RCC_APB2Periph_GPIOA, ENABLE);
                            
    /* Tx Pin */
    gpioa_init_struct.GPIO_Pin	 = GPIO_Pin_9;
    gpioa_init_struct.GPIO_Speed = GPIO_Speed_50MHz;
    gpioa_init_struct.GPIO_Mode	 = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOA, &gpioa_init_struct);

    /* Rx Pin */
    gpioa_init_struct.GPIO_Pin	 = GPIO_Pin_10;
    gpioa_init_struct.GPIO_Speed = GPIO_Speed_50MHz;
    gpioa_init_struct.GPIO_Mode	 = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &gpioa_init_struct);
 
    /* Enable USART1 */
    USART_Cmd(USART1, ENABLE);  

    /* Baud rate 115200, 8-bit data, One stop bit
     * No parity, Tx only, No HW flow control
     */
    usart1_init_struct.USART_BaudRate		 = 115200;   
    usart1_init_struct.USART_WordLength		 = USART_WordLength_8b;  
    usart1_init_struct.USART_StopBits		 = USART_StopBits_1;   
    usart1_init_struct.USART_Parity		 = USART_Parity_No ;
    usart1_init_struct.USART_Mode		 = USART_Mode_Tx;
    usart1_init_struct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;

    /* Configure USART1 */
    USART_Init(USART1, &usart1_init_struct);    
}

/**********************************************************
 * RTC interrupt request handler (interrupt on pre-scalar
 * overflow)
 *
 * This handler samples the SysTick clock and uses it as
 * input into the entropy pool. Given that RTC and SysTick
 * are clocked off of independant clocks, the jitter
 * present should create a source of randomness.
 *********************************************************/
void RTC_IRQHandler(void)
{
    // Clear interrupt
    RTC_ClearFlag(RTC_FLAG_SEC);
    Entropy_AppendToPool(&EP, SysTick->VAL);
}

/**********************************************************
 * Microseconds delay
 *********************************************************/
static inline void Delay_us(uint32_t us)
{
    us *= 8;

    /* fudge for function call overhead  */
    us--;
    asm volatile ("   mov r0, %[us]          \n\t"
		  "1: subs r0, #1            \n\t"
		  "   bhi 1b                 \n\t"::[us] "r" (us):"r0");
}
