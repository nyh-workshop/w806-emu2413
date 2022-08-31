#ifndef W806_I2S_AUDIO_H
#define W806_I2S_AUDIO_H

#include "wm_hal.h"
#include <cmath>

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "queue.h"
#include "event_groups.h"

constexpr uint32_t I2S_BUFFER_SIZE = 512;

extern uint32_t tx_buf[I2S_BUFFER_SIZE];

// For sine wave test only!
struct preCalculateSineTable1024
{
    constexpr preCalculateSineTable1024() : table()
    {
        for (uint32_t i = 0; i < 1024; i++)
        {
            table[i] = (int16_t)((float)32767.0 * std::sin(((float)i / (float)(1024.0) * 2.0f * M_PI)));
        }
    }
    int16_t table[1024];
};

struct i2sBuffer
{
    volatile uint8_t bEmptyFlag = 0;
    volatile uint8_t bSelect = 0;
};

extern i2sBuffer i2sB;

template <typename T>
class w806_i2sAudio
{
public:
	w806_i2sAudio();
	int16_t getSample();
    void fillBuffer(uint32_t* buffer, uint32_t startIndex, uint32_t endIndex);
	void playSamples();
protected:
	//i2sBuffer i2sB;    
	//static friend void HAL_I2S_TxHalfCpltCallback(I2S_HandleTypeDef *hi2s);
	//static friend void HAL_I2S_TxCpltCallback(I2S_HandleTypeDef *hi2s);
};

template <typename T>
w806_i2sAudio<T>::w806_i2sAudio()
{
	memset(tx_buf, 0x00, sizeof(tx_buf));
	//i2sB.bSelect = 0;
	//i2sB.bEmptyFlag = 0;
}

template <typename T>
int16_t w806_i2sAudio<T>::getSample()
{
	return static_cast<T*>(this)->getSample();
}

template <typename T>
void w806_i2sAudio<T>::fillBuffer(uint32_t *buffer, uint32_t startIndex, uint32_t endIndex)
{
    for (uint32_t i = startIndex; i < endIndex; i++)
    {
        int16_t temp = getSample();
        // buffer[i] = (temp & 0x0000ffff) | ( (temp & 0x0000ffff) << 16 );
        buffer[i] = temp & 0x0000ffff;
    }
}

template <typename T>
void w806_i2sAudio<T>::playSamples()
{
	if(i2sB.bEmptyFlag)
    {
        //printf("buffer empty!\r\n");
        if(!i2sB.bSelect)
        {
            //printf("<");
            fillBuffer(tx_buf, 0, I2S_BUFFER_SIZE / 2);
        }
        else
        {
            //printf(">");
            fillBuffer(tx_buf, I2S_BUFFER_SIZE / 2, I2S_BUFFER_SIZE);
        }
        i2sB.bEmptyFlag = 0;
    }
}

#endif