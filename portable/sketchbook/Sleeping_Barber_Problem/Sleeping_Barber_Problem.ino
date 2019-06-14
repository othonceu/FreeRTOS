
#include "FreeRTOS_AVR.h"
#include "basic_io_avr.h"

#define cadeira 2
#define cliente 4

const TickType_t umSegundo = 1000 / portTICK_PERIOD_MS;

uint8_t clienteEsperando;

SemaphoreHandle_t semCliente, semBarbeiro, semMutex;

void barbeiroTask(void* barbeiro);

void clienteTask(void* clienteT);

void setup() {
  Serial.begin(9600);

  if (xTaskCreate(barbeiroTask, "Barbeiro", 100, NULL, 1, NULL) == pdFAIL) {
      vPrintString("Houve problema de criação no barbeiroTask.\n");
      for (;;) {}
  }

  for (uint8_t i = 0;  i < cliente; i++) {
    if (xTaskCreate(clienteTask, "cliente", 120, NULL, 1, NULL) == pdFAIL) {
       vPrintString("Houve problema de criação de clienteTask.\n");
    }
  }

  semCliente = xSemaphoreCreateCounting(cadeira, 0);
  semBarbeiro = xSemaphoreCreateBinary();
  semMutex = xSemaphoreCreateMutex();
  clienteEsperando = 0;

  if (semCliente == NULL || semBarbeiro == NULL || semMutex == NULL) {
   // vPrintString("Houve problema na criação do semáfaro.\n");
    for (;;) {}
  }

  randomSeed(analogRead(0));

  vTaskStartScheduler();
  for (;;) {}
}

void barbeiroTask(void* barbeiro) {
  uint8_t flag = 0;
  for (;;) {
    xSemaphoreTake(semCliente, portMAX_DELAY);
    xSemaphoreTake(semMutex, portMAX_DELAY);
   
    if(flag == 0){
      vPrintString("Acordou!!\n");
      flag = 1;
     }
    clienteEsperando -= 1;
    xSemaphoreGive(semBarbeiro);
    xSemaphoreGive(semMutex);
    
   
    vTaskDelay(umSegundo);
  }
}

void clienteTask(void* clienteT) {
  for (;;) {
    vTaskDelay(random(200, 1700) / portTICK_PERIOD_MS);
    
    xSemaphoreTake(semMutex, portMAX_DELAY );
    
    if (clienteEsperando < cadeira) {
       
        clienteEsperando += 1;
        vPrintStringAndNumber("AgoraNº:",clienteEsperando);
        xSemaphoreGive(semCliente);
        xSemaphoreGive(semMutex); //Saindo da região crítica
      xSemaphoreTake(semBarbeiro, portMAX_DELAY);
     
      vPrintStringAndNumber("CortouNº:",clienteEsperando);
     
    } else {
      vPrintString("Cheio indo embora\n");
      xSemaphoreGive(semMutex);
      vTaskDelay(umSegundo);
    }
  }
}
void loop() {}
