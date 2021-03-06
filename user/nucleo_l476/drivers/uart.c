#include "uart.h"
#include "board_includes.h"
#include "ecode.h"

#define USE_UART2
#define UART2_INSTANCE           USART2
#define UART2_CLK_ENABLE()     LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_USART2)
#define UART2_CLK_SOURCE()     LL_RCC_SetUSARTClockSource(LL_RCC_USART2_CLKSOURCE_PCLK1)
#define UART2_GPIO_CLK_ENABLE()    LL_AHB2_GRP1_EnableClock(LL_AHB2_GRP1_PERIPH_GPIOA)
#define UART2_TX_PIN               LL_GPIO_PIN_2
#define UART2_TX_GPIO_PORT              GPIOA
#define UART2_SET_TX_GPIO_AF()         LL_GPIO_SetAFPin_0_7(UART2_TX_GPIO_PORT, UART2_TX_PIN, LL_GPIO_AF_7)
#define UART2_RX_PIN               LL_GPIO_PIN_3
#define UART2_RX_GPIO_PORT              GPIOA
#define UART2_SET_RX_GPIO_AF()         LL_GPIO_SetAFPin_0_7(UART2_RX_GPIO_PORT, UART2_RX_PIN, LL_GPIO_AF_7)
#define UART2_IRQ_NUM              USART2_IRQn
#define UART2_INTERRUPT              USART2_IRQHandler
#define SERIAL_RX_BUFFER_SIZE       100



#define UART_ENABLE_IRQ(n)            NVIC_EnableIRQ((n))
#define UART_DISABLE_IRQ(n)           NVIC_DisableIRQ((n))

struct stm_uart{
    USART_TypeDef *instance;
    IRQn_Type irq;
};


static e_err_t stm_uart_init(struct serial_dev *serial)
{
    struct stm_uart *uart;
    struct serial_configure *cfg;
    
    ASSERT_PARAM(serial!=NULL);
    cfg = &serial->config;
    
    uart = (struct stm_uart *)serial->private_data;
    
    if(cfg->data_bits==DATA_BITS_8)
    {
        LL_USART_SetDataWidth(uart->instance,LL_USART_DATAWIDTH_8B);
    }else if(cfg->data_bits==DATA_BITS_9){
        LL_USART_SetDataWidth(uart->instance,LL_USART_DATAWIDTH_9B);
    }
    
    if(cfg->stop_bits == STOP_BITS_1){
        LL_USART_SetStopBitsLength(uart->instance, LL_USART_STOPBITS_1);
    }else if(cfg->stop_bits==STOP_BITS_2){
        LL_USART_SetStopBitsLength(uart->instance, LL_USART_STOPBITS_2);
    }
    
    if(cfg->parity==PARITY_NONE)
    {
        LL_USART_SetParity(uart->instance, LL_USART_PARITY_NONE);
    }else if(cfg->parity==PARITY_ODD){
        LL_USART_SetParity(uart->instance, LL_USART_PARITY_ODD);
    }else if(cfg->parity==PARITY_EVEN){
        LL_USART_SetParity(uart->instance, LL_USART_PARITY_EVEN);
    }
        
    /* TX/RX direction */
    LL_USART_SetTransferDirection(uart->instance, LL_USART_DIRECTION_TX_RX);

    /* 8 data bit, 1 start bit, 1 stop bit, no parity */
    //LL_USART_ConfigCharacter(UART2_INSTANCE, LL_USART_DATAWIDTH_8B, LL_USART_PARITY_NONE, LL_USART_STOPBITS_1);

    /* No Hardware Flow control */
    /* Reset value is LL_USART_HWCONTROL_NONE */
    LL_USART_SetHWFlowCtrl(uart->instance, LL_USART_HWCONTROL_NONE);

#ifdef  USE_UART2
    if(uart->instance==UART2_INSTANCE)
        LL_USART_SetBaudRate(uart->instance, SystemCoreClock, LL_USART_OVERSAMPLING_16, cfg->baud_rate); 
#endif

    LL_USART_Enable(uart->instance);
    
    /* Polling USART initialisation */
    while((!(LL_USART_IsActiveFlag_TEACK(uart->instance))) || (!(LL_USART_IsActiveFlag_REACK(uart->instance))))
    { 
    } 
    
    UART_ENABLE_IRQ(uart->irq);
    LL_USART_EnableIT_RXNE(uart->instance);
    LL_USART_ClearFlag_TC(uart->instance);
    
    return E_EOK;
}

static void uart_isr(struct serial_dev *serial)
{
    struct stm_uart * uart= (struct stm_uart *)serial->private_data;
    
    ASSERT_PARAM(uart!=NULL);
    
    if(LL_USART_IsActiveFlag_RXNE(uart->instance))
    {
        serial_hw_isr(serial, SERIAL_EVENT_RX_IND);
    }
    if(LL_USART_IsActiveFlag_TC(uart->instance))
    {
        
    }
}


static int stm_putc(struct serial_dev *serial, char c)
{
    struct stm_uart *uart;

    ASSERT_PARAM(serial != NULL);
    uart = serial->private_data;
    ASSERT_PARAM(uart!=NULL);
    
    while(!LL_USART_IsActiveFlag_TXE(uart->instance));
    LL_USART_TransmitData8(uart->instance, c);
    
    return 1;
}

static int stm_getc(struct serial_dev *serial)
{
    int ch;
    struct stm_uart *uart;

    ASSERT_PARAM(serial != NULL);
    uart = (struct stm_uart *)serial->private_data;

    ch = -1;
    
    if(LL_USART_IsActiveFlag_RXNE(uart->instance))
    {
        ch = LL_USART_ReceiveData8(uart->instance);
    }

    return ch;
}

static const struct serial_operation stm_uart_ops={
    .init = stm_uart_init,
    .fputc = stm_putc,
    .fgetc = stm_getc,
};

#ifdef USE_UART2
struct stm_uart uart2 = {
    .instance = UART2_INSTANCE,
    .irq = UART2_IRQ_NUM,
};

e_uint8_t serial2_rx_buffer[SERIAL_RB_BUFSZ];

struct serial_rx_fifo serial2_rx_fifo={
    .buffer = serial2_rx_buffer,
    .put_index = 0,
    .get_index = 0,
    .bufsz = SERIAL_RB_BUFSZ,
};

static struct serial_dev serial2={
    .flag = SERIAL_FLAG_INT_RX,
    
    .ops = &stm_uart_ops,
    .config = SERIAL_CONFIG_DEFAULT,
    
    .serial_rx = &serial2_rx_fifo,
    
    .private_data = &uart2,
};


void UART2_INTERRUPT(void)
{
    uart_isr(&serial2);
}
#endif

static void uart_rcc_configuration(void)
{
#ifdef USE_UART2
    UART2_CLK_ENABLE();
    UART2_CLK_SOURCE();
    UART2_GPIO_CLK_ENABLE();
#endif
}

static void uart_gpio_configuration(void)
{
#ifdef USE_UART2
  /* Configure Tx Pin as : Alternate function, High Speed, Push pull, Pull up */
  LL_GPIO_SetPinMode(UART2_TX_GPIO_PORT, UART2_TX_PIN, LL_GPIO_MODE_ALTERNATE);
  UART2_SET_TX_GPIO_AF();
  LL_GPIO_SetPinSpeed(UART2_TX_GPIO_PORT, UART2_TX_PIN, LL_GPIO_SPEED_FREQ_HIGH);
  LL_GPIO_SetPinOutputType(UART2_TX_GPIO_PORT, UART2_TX_PIN, LL_GPIO_OUTPUT_PUSHPULL);
  LL_GPIO_SetPinPull(UART2_TX_GPIO_PORT, UART2_TX_PIN, LL_GPIO_PULL_UP);

  /* Configure Rx Pin as : Alternate function, High Speed, Push pull, Pull up */
  LL_GPIO_SetPinMode(UART2_RX_GPIO_PORT, UART2_RX_PIN, LL_GPIO_MODE_ALTERNATE);
  UART2_SET_RX_GPIO_AF();
  LL_GPIO_SetPinSpeed(UART2_RX_GPIO_PORT, UART2_RX_PIN, LL_GPIO_SPEED_FREQ_HIGH);
  LL_GPIO_SetPinOutputType(UART2_RX_GPIO_PORT, UART2_RX_PIN, LL_GPIO_OUTPUT_PUSHPULL);
  LL_GPIO_SetPinPull(UART2_RX_GPIO_PORT, UART2_RX_PIN, LL_GPIO_PULL_UP);
#endif
}

static void uart_nvic_configuration(struct stm_uart *uart)
{
    NVIC_SetPriority(uart->irq, 0);
    NVIC_EnableIRQ(uart->irq);
}

void uart_hw_init(void)
{
    struct stm_uart *uart;
    struct serial_configure config = SERIAL_CONFIG_DEFAULT;
    
    uart_rcc_configuration();
    uart_gpio_configuration();
    
#ifdef USE_UART2
    uart = &uart2;
    
    serial2.ops = &stm_uart_ops;
    serial2.config = config;
    
    uart_nvic_configuration(uart);
    
    serial_register(COM1,
                    &serial2,
                    "COM1");
#endif
}

