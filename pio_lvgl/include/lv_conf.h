#ifndef LV_CONF_H
#define LV_CONF_H

#include <stdint.h>

#ifndef LV_CONF_INCLUDE_SIMPLE
#define LV_CONF_INCLUDE_SIMPLE
#endif

#define LV_COLOR_DEPTH 16
#define LV_COLOR_16_SWAP 0
#define LV_USE_LOG 0
#define LV_USE_PERF_MONITOR 0
#define LV_USE_MEM_MONITOR 0
#define LV_USE_DEMO_WIDGETS 0
#define LV_USE_DEMO_BENCHMARK 0
#define LV_USE_DEMO_STRESS 0
#define LV_USE_DEMO_MUSIC 0

#define LV_FONT_MONTSERRAT_14 1
#define LV_FONT_MONTSERRAT_16 1
#define LV_FONT_MONTSERRAT_18 1
#define LV_FONT_DEFAULT &lv_font_montserrat_16

#define LV_USE_GPU 0
#define LV_USE_GPU_STM32_DMA2D 0

#endif // LV_CONF_H
