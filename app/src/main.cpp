#include <stdio.h>
#include "wm_hal.h"
#include <cmath>

#include "emu2413_driver.h"

I2S_HandleTypeDef hi2s;
DMA_HandleTypeDef hdma_i2s_tx;
TIM_HandleTypeDef htim0;

const HeapRegion_t xHeapRegions[] = 
{ 
    { ( uint8_t * ) 0x20020000UL, 0x5000 },
    { NULL, 0 }
};

void task1_handle(void *p);
void task2_handle(void *p);
TaskHandle_t htask1 = NULL;
TaskHandle_t htask2 = NULL;

extern "C" void Error_Handler(void);

static void GPIO_Init(void);
static void I2S_Init(void);
static void DMA_Init(void);
static void TIM0_Init(void);

void Error_Handler(void);

SemaphoreHandle_t xSemaphoreGetOPLL;
SemaphoreHandle_t xSemaphoreBufferEmptyFlag;

emu2413driver* _OPLL_ptr = nullptr;

int main(void)
{
    SystemClock_Config(CPU_CLK_240M);
    printf("enter main\r\n");
  
    HAL_Init();
    GPIO_Init();
    DMA_Init();
    I2S_Init();
    TIM0_Init();

    vPortDefineHeapRegions( xHeapRegions );

    xSemaphoreGetOPLL = xSemaphoreCreateBinary();
    xSemaphoreBufferEmptyFlag = xSemaphoreCreateBinary();

    xTaskCreate(task1_handle, "OPLL_task", 512, NULL, 35, &htask1);
    xTaskCreate(task2_handle, "mdt_task", 512, NULL, 37, &htask2);

    vTaskStartScheduler();

    return 0;
}

void task1_handle(void *p)
{
    emu2413driver _OPLL;

    _OPLL_ptr = &_OPLL;

    xSemaphoreGive(xSemaphoreGetOPLL);

    printf("task1 starting!\r\n");
    printf("starting dma!\r\n");

    HAL_I2S_Transmit_DMA(&hi2s, (uint32_t *)tx_buf, I2S_BUFFER_SIZE * 2);

    xSemaphoreGive(xSemaphoreGetOPLL);

    _OPLL.writeReg(0x30, 0xb0);
    _OPLL.writeReg(0x31, 0xb0); 
    _OPLL.writeReg(0x32, 0xb0);
    _OPLL.writeReg(0x33, 0xb0);
    _OPLL.writeReg(0x34, 0xb0);
    _OPLL.writeReg(0x35, 0xb0);
    _OPLL.writeReg(0x36, 0xb0);
    // _OPLL.writeReg(0x10, 0x80); /* set F-Number(L). */
    // _OPLL.writeReg(0x20, 0x15); /* set BLK & F-Number(H) and
    //                              /* keyon. */

    //_OPLL.startPlaying();

    while (1)
    {
        xSemaphoreTake(xSemaphoreBufferEmptyFlag, portMAX_DELAY);
        _OPLL.playSamples();
    }
}

void task2_handle(void *p)
{
   // Wait for task1 to get the OPLL pointer first!
   printf("task2 starting!\r\n");
   printf("waiting for task1 to get OPLL ptr!\r\n");
   xSemaphoreTake(xSemaphoreGetOPLL, portMAX_DELAY);

   configASSERT(_OPLL_ptr != nullptr);

   xSemaphoreTake(xSemaphoreGetOPLL, portMAX_DELAY);

   printf("run stepscore now!\r\n");

   _OPLL_ptr->startPlaying();

   while(1)
   {
        _OPLL_ptr->isPlaying() ? _OPLL_ptr->stepScore() : vTaskSuspend(NULL);
   }
}

static void GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    __HAL_RCC_GPIO_CLK_ENABLE();

    GPIO_InitStruct.Pin = GPIO_PIN_3 | GPIO_PIN_4;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, GPIO_PIN_RESET);    
}

static void I2S_Init(void)
{
    hi2s.Instance = I2S;
    hi2s.Init.Mode = I2S_MODE_MASTER;
    hi2s.Init.Standard = I2S_STANDARD_PHILIPS;
    hi2s.Init.DataFormat = I2S_DATAFORMAT_16B;
    hi2s.Init.MCLKOutput = I2S_MCLKOUTPUT_ENABLE;
    hi2s.Init.AudioFreq = I2S_AUDIOFREQ_22K;
    hi2s.Init.Channel = I2S_CHANNEL_STEREO;
    hi2s.Init.ChannelSel = I2S_CHANNELSEL_LEFT;
    if (HAL_I2S_Init(&hi2s) != HAL_OK)
    {
        Error_Handler();
    }
}

static void DMA_Init(void)
{
    __HAL_RCC_DMA_CLK_ENABLE();
    
    HAL_NVIC_SetPriority(DMA_Channel0_IRQn, 0);
    HAL_NVIC_EnableIRQ(DMA_Channel0_IRQn);
}

static void TIM0_Init(void)
{
    htim0.Instance = TIM0;
    htim0.Init.Unit = TIM_UNIT_MS;
    htim0.Init.Period = 1;
    htim0.Init.AutoReload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    if (HAL_TIM_Base_Init(&htim0) != HAL_OK)
    {
        Error_Handler();
    }
}

void HAL_I2S_ErrorCallback(I2S_HandleTypeDef *hi2s)
{
    printf("err cb\r\n");
}

void Error_Handler(void)
{
    while (1)
    {
        printf("error handler\r\n");
    }
}

void assert_failed(uint8_t *file, uint32_t line)
{
    printf("Wrong parameters value: file %s on line %d\r\n", file, line);
}

void vApplicationStackOverflowHook(TaskHandle_t xTask, char* pcTaskName)
{
    printf("Stack overflow!\r\n");
    while(1);
}

void HAL_TIM_Callback(TIM_HandleTypeDef *htim)
{
    static BaseType_t xHigherPriorityTaskWoken;

    if (htim->Instance == TIM0)
    {
        vTaskNotifyGiveFromISR(htask2, &xHigherPriorityTaskWoken);
    }

    portYIELD_FROM_ISR(&xHigherPriorityTaskWoken);
}