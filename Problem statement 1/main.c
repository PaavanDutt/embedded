#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include <stdio.h>
#include <stdint.h>

/* --------------------------------------------------------------------------
   Global Variables
   -------------------------------------------------------------------------- */
uint8_t  G_DataID    = 0;   // Updated elsewhere
int32_t  G_DataValue = 0;   // Updated elsewhere

/* --------------------------------------------------------------------------
   Type Definition
   -------------------------------------------------------------------------- */
typedef struct {
    uint8_t dataID;
    int32_t DataValue;
} Data_t;

/* --------------------------------------------------------------------------
   Queue Handle & Task Handles
   -------------------------------------------------------------------------- */
QueueHandle_t Queue1;
TaskHandle_t  TaskHandle_1;
TaskHandle_t  TaskHandle_2;

/* --------------------------------------------------------------------------
   Task Prototypes
   -------------------------------------------------------------------------- */
void ExampleTask1(void *pV);
void ExampleTask2(void *pV);

/* --------------------------------------------------------------------------
   Main Function
   -------------------------------------------------------------------------- */
int main(void)
{
    /* Create Queue with size 5 and element type Data_t */
    Queue1 = xQueueCreate(5, sizeof(Data_t));
    if (Queue1 == NULL) {
        printf("Queue creation failed!\n");
        while(1);
    }

    /* Create Tasks */
    xTaskCreate(ExampleTask1, "ExampleTask1", 1000, NULL, 2, &TaskHandle_1);
    xTaskCreate(ExampleTask2, "ExampleTask2", 1000, NULL, 1, &TaskHandle_2);

    /* Start Scheduler */
    vTaskStartScheduler();

    /* Should never reach here */
    for(;;);
}

/* --------------------------------------------------------------------------
   ExampleTask1: Sends Data to Queue Every 500ms Exactly
   -------------------------------------------------------------------------- */
void ExampleTask1(void *pV)
{
    TickType_t xLastWakeTime;
    const TickType_t xFrequency = pdMS_TO_TICKS(500);
    Data_t sendData;

    /* Initialize xLastWakeTime for precise periodic execution */
    xLastWakeTime = xTaskGetTickCount();

    for(;;)
    {
        /* Prepare data */
        sendData.dataID    = G_DataID;
        sendData.DataValue = G_DataValue;

        /* Send to queue */
        if (xQueueSend(Queue1, &sendData, 0) == pdPASS)
        {
            printf("[Task1] Sent → dataID: %u | DataValue: %ld\n", 
                    sendData.dataID, sendData.DataValue);
        }

        /* Delay exactly 500ms from previous wake time */
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
}

/* --------------------------------------------------------------------------
   ExampleTask2: Receives Data and Performs Actions
   -------------------------------------------------------------------------- */
void ExampleTask2(void *pV)
{
    Data_t recvData;
    UBaseType_t initialPriority, newPriority;

    /* Store initial priority to adjust dynamically later */
    initialPriority = uxTaskPriorityGet(TaskHandle_2);
    newPriority     = initialPriority;

    for(;;)
    {
        /* Wait indefinitely for data from queue */
        if (xQueueReceive(Queue1, &recvData, portMAX_DELAY) == pdPASS)
        {
            printf("[Task2] Received → dataID: %u | DataValue: %ld\n",
                   recvData.dataID, recvData.DataValue);

            /* -------------------------
               Apply Conditions & Actions
               ------------------------- */
            if (recvData.dataID == 0)
            {
                printf("[Task2] dataID == 0 → Deleting ExampleTask2\n");
                vTaskDelete(TaskHandle_2);
            }
            else if (recvData.dataID == 1)
            {
                /* Process DataValue member */
                if (recvData.DataValue == 0)
                {
                    /* Increase priority by 2 */
                    newPriority = initialPriority + 2;
                    vTaskPrioritySet(TaskHandle_2, newPriority);
                    printf("[Task2] Priority Increased to %lu\n", (unsigned long)newPriority);
                }
                else if (recvData.DataValue == 1)
                {
                    /* Restore priority if previously increased */
                    if (newPriority > initialPriority)
                    {
                        vTaskPrioritySet(TaskHandle_2, initialPriority);
                        printf("[Task2] Priority Decreased to %lu\n", (unsigned long)initialPriority);
                    }
                }
                else if (recvData.DataValue == 2)
                {
                    printf("[Task2] DataValue == 2 → Deleting ExampleTask2\n");
                    vTaskDelete(TaskHandle_2);
                }
            }
        }
    }
}
