#include <can2040.h>
#include <hardware/regs/intctrl.h>
#include <stdio.h>
#include <pico/stdlib.h>
#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>

#define CALLBACK_TASK_PRIORITY      ( tskIDLE_PRIORITY + 1UL )
#define CALLBACK_TASK_STACK_SIZE configMINIMAL_STACK_SIZE

static struct can2040 cbus;
QueueHandle_t callbackQueue;

static void can2040_cb(struct can2040 *cd, uint32_t notify, struct can2040_msg *msg)
{

}

static void PIOx_IRQHandler(void)
{
    can2040_pio_irq_handler(&cbus);
}

void canbus_setup(void)
{
    uint32_t pio_num = 0;
    uint32_t sys_clock = 125000000, bitrate = 500000;
    uint32_t gpio_rx = 4, gpio_tx = 5;

    // Setup canbus
    can2040_setup(&cbus, pio_num);
    can2040_callback_config(&cbus, can2040_cb);

    // Enable irqs
    irq_set_exclusive_handler(PIO0_IRQ_0, PIOx_IRQHandler);
    irq_set_priority(PIO0_IRQ_0, PICO_DEFAULT_IRQ_PRIORITY - 1);
    irq_set_enabled(PIO0_IRQ_0, 1);

    // Start canbus
    can2040_start(&cbus, sys_clock, bitrate, gpio_rx, gpio_tx);
}

void callback_thread(void *params)
{
    struct can2040_msg msg;
    for (;;)
    {
        if (xQueueReceive(callbackQueue, &msg, portMAX_DELAY) == pdTRUE)
        {
            //Get CAN msg and print?
        }
    }
}


void transmit_can(void *params)
{
    vTaskDelay(pdMS_TO_TICKS(5000));
    struct can2040_msg transmit_msg;
    transmit_msg.id = 32;
    transmit_msg.dlc = 2;
    transmit_msg.data[0] = 32;
    transmit_msg.data[1] = 43;
    while(true){
        int ret = can2040_transmit(&cbus, &transmit_msg);
        if(ret == 0){    
            printf("success");
        }else {
            printf("failed %d \n", ret);
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}


int main(void)
{
    stdio_init_all();
    hard_assert(cyw43_arch_init() == PICO_OK);
    TaskHandle_t callback;
    canbus_setup();
    xTaskCreate(transmit_can, "transmit_can",
                CALLBACK_TASK_STACK_SIZE, NULL, CALLBACK_TASK_PRIORITY, &callback);
    callbackQueue = xQueueCreate(100, sizeof(struct can2040_msg));
    vTaskStartScheduler();
	return 0;
}