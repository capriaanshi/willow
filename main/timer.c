#include "driver/timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

#include "tasks.h"
#include "timer.h"

#define TIMER_DIV   (16)
#define TIMER_SCALE (TIMER_BASE_CLK / TIMER_DIV)

xQueueHandle hdl_q_timer = NULL;

static bool IRAM_ATTR cb_timer_isr(void *data)
{
    BaseType_t hiprio_woken = pdFALSE;
    int value = 10;
    xQueueSendFromISR(hdl_q_timer, &value, &hiprio_woken);
    return hiprio_woken == pdTRUE;
}

void init_timer(void)
{
    timer_config_t cfg_timer = {
        .alarm_en = true,
        .auto_reload = true,
        .counter_dir = TIMER_COUNT_UP,
        .counter_en = TIMER_PAUSE,
        .divider = TIMER_DIV,
    };

    hdl_q_timer = xQueueCreate(2, sizeof(int));

    ESP_ERROR_CHECK_WITHOUT_ABORT(timer_init(TIMER_GROUP_0, TIMER_0, &cfg_timer));
    ESP_ERROR_CHECK_WITHOUT_ABORT(timer_set_counter_value(TIMER_GROUP_0, TIMER_0, 0));
    ESP_ERROR_CHECK_WITHOUT_ABORT(timer_set_alarm_value(TIMER_GROUP_0, TIMER_0, TIMER_SCALE * 10));
    ESP_ERROR_CHECK_WITHOUT_ABORT(timer_enable_intr(TIMER_GROUP_0, TIMER_0));
    ESP_ERROR_CHECK_WITHOUT_ABORT(timer_isr_callback_add(TIMER_GROUP_0, TIMER_0, cb_timer_isr, NULL, 0));

    xTaskCreate(&task_timer, "task_timer", 4 * 1024, NULL, 5, NULL);
}

void reset_timer(bool pause)
{
    if (pause) {
        timer_pause(TIMER_GROUP_0, TIMER_0);
    }
    timer_set_counter_value(TIMER_GROUP_0, TIMER_0, 0);
}