
#include "wm_hal.h"
#include "FreeRTOS.h"
#include "task.h"
#include "FreeRTOSConfig.h"

extern I2S_HandleTypeDef hi2s;
extern DMA_HandleTypeDef hdma_i2s_tx;
extern TIM_HandleTypeDef htim0;

#define readl(addr) ({unsigned int __v = (*(volatile unsigned int *) (addr)); __v;})
__attribute__((isr)) void CORET_IRQHandler(void)
{
    readl(0xE000E010);
    HAL_IncTick();

#if (INCLUDE_xTaskGetSchedulerState == 1)
	if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED)
	{
#endif
	xPortSysTickHandler();
#if (INCLUDE_xTaskGetSchedulerState == 1)
	}
#endif
}

__attribute__((isr)) void I2S_IRQHandler(void)
{
    HAL_I2S_IRQHandler(&hi2s);
}

__attribute__((isr)) void DMA_Channel0_IRQHandler(void)
{
    HAL_DMA_IRQHandler(&hdma_i2s_tx);
}

__attribute__((isr)) void TIM0_5_IRQHandler(void)
{
    HAL_TIM_IRQHandler(&htim0);
}