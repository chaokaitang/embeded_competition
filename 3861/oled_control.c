#include <stdio.h>
#include <unistd.h>
#include "hi_timer.h"
#include "hi_time.h"
#include "iot_watchdog.h"
#include "iot_errno.h"
#include "hi_errno.h"

#include "iot_gpio_ex.h"
#include "ohos_init.h"
#include "cmsis_os2.h"
#include "iot_gpio.h"
#include "ssd1306_fonts.h"
#include "ssd1306.h"
#define delay_time 1000
#define IOT_I2C_IDX_BAUDRATE 40000
#define SSD1306_I2C_IDX 0
void oled_init(void)
{
    ssd1306_Init();
    ssd1306_ClearOLED();
    printf("oled ready!\n");
}
