#include "w806_i2sAudio.h"

uint32_t tx_buf[I2S_BUFFER_SIZE];

i2sBuffer i2sB { 0, 0 };

extern SemaphoreHandle_t xSemaphoreBufferEmptyFlag;

extern "C" void HAL_I2S_TxHalfCpltCallback(I2S_HandleTypeDef *hi2s)
{
    static BaseType_t xHigherPriorityTaskWoken;
    //printf("tx halfcplt\r\n");
    i2sB.bSelect = 0;
    i2sB.bEmptyFlag = 1;
    xSemaphoreGiveFromISR(xSemaphoreBufferEmptyFlag, &xHigherPriorityTaskWoken);

    portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
}

extern "C" void HAL_I2S_TxCpltCallback(I2S_HandleTypeDef *hi2s)
{
    static BaseType_t xHigherPriorityTaskWoken;
    //printf("tx cplt\r\n");
    i2sB.bSelect = 1;
    i2sB.bEmptyFlag = 1;
    xSemaphoreGiveFromISR(xSemaphoreBufferEmptyFlag, &xHigherPriorityTaskWoken);

    portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
}
