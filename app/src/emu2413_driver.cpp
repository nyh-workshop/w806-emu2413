#include "emu2413_driver.h"
#include "miditonesFile.h"

extern I2S_HandleTypeDef hi2s;
extern DMA_HandleTypeDef hdma_i2s_tx;
extern TIM_HandleTypeDef htim0;

extern TaskHandle_t htask2;

OPLL emu2413driver::opll;
OPLL_RateConv emu2413driver::opll_conv;

static preCalculateSineTable1024 sine1024table;

emu2413driver::emu2413driver()
{
	_emu2413_assert(hdma_i2s_tx.Instance || hi2s.Instance, "Must load both the i2s and DMA before init this i2sAudio!\n");
	
    OPLL_new(&opll, &opll_conv, true, static_cast<uint32_t>(MSX_CLK), static_cast<uint32_t>(DEFAULT_SAMPLE_RATE));
    
	playScore(mdtFile1);
}

// 440Hz sine wave:
static uint32_t accumulator = 0;
static uint32_t phase = 42852281;

int16_t generateSineWaveSample()
{
    accumulator += phase;
    return sine1024table.table[accumulator >> 22];
}

int16_t emu2413driver::getSample() {
	return OPLL_calc(&opll);
	//return generateSineWaveSample();
}

void emu2413driver::setUpOneShotTimer(uint32_t durationMs)
{
    htim0.Init.Period = durationMs;
    HAL_TIM_Base_Init(&htim0);
    HAL_TIM_Base_Start_IT(&htim0);
}

void emu2413driver::waitTimerEventComplete()
{
	ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
}

void _emu2413_assert(bool condition, const char* str) {
	if(!condition)
	{
		printf(str);
		while(1);
	}
}