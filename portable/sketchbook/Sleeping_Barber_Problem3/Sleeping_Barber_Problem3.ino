
#include "FreeRTOS_AVR.h"
#include "basic_io_avr.h"

#define cadeira 2
#define cliente 4

const TickType_t umSegundo = 1000 / portTICK_PERIOD_MS;

uint8_t clienteEsperando;

SemaphoreHandle_t semCliente, semBarbeiro, semMutex;

TaskHandle_t barbeiro1;

void barbeiroTask(void* barbeiro);

void clienteTask(void* clienteT);

void setup() {
  Serial.begin(9600);

  if (xTaskCreate(barbeiroTask, "Barbeiro", 100, NULL, 1, NULL) == pdFAIL) {
    vPrintString("Houve problema de criação no barbeiroTask.\n");
    for (;;) {}
  }

  for (uint8_t i = 0;  i < cliente; i++) {
    if (xTaskCreate(clienteTask, "cliente", 100, NULL, 1, NULL) == pdFAIL) {
      vPrintString("Houve problema de criação de clienteTask.\n");
    }
  }

  semCliente = xSemaphoreCreateCounting(cadeira, 0);
  semBarbeiro = xSemaphoreCreateBinary();
  semMutex = xSemaphoreCreateMutex();
  clienteEsperando = 0;

  if (semCliente == NULL || semBarbeiro == NULL || semMutex == NULL) {
    vPrintString("Houve problema na criação do semáfaro.\n");
    for (;;) {}
  }

  randomSeed(analogRead(0));

  vTaskStartScheduler();
  for (;;) {}
}

void barbeiroTask(void* barbeiro) {
  for (;;) {

    
    xSemaphoreTake(semCliente, portMAX_DELAY / portTICK_PERIOD_MS);
    xSemaphoreTake(semMutex, portMAX_DELAY / portTICK_PERIOD_MS);
    
    vPrintString("O barbeiro tem clientes esperando!!.\n");
    clienteEsperando -= 1;
    xSemaphoreGive(semBarbeiro);
    
    xSemaphoreGive(semMutex);
    ulTaskNotifyTake(pdFALSE, portMAX_DELAY);
    vPrintString("O barbeiro está cortando o cabelo do cliente.\n");
    vTaskDelay(umSegundo);
    
  }
}

void clienteTask(void* clienteT) {
  for (;;) {
    vTaskDelay(random(100, 1500) / portTICK_PERIOD_MS);
    
    xSemaphoreTake(semMutex, portMAX_DELAY / portTICK_PERIOD_MS);
    
    if (xSemaphoreGive(semCliente) == pdTRUE) {
      if (clienteEsperando > 0) {
        vPrintString("O barbeiro está cortando cabelo, o cliente vai esperar.\n");
      } else {
        vPrintString("A barbearia está vazia, vou acordar o barbeiro.\n");
      }
      clienteEsperando += 1;
      
      xSemaphoreGive(semMutex);
      xSemaphoreTake(semBarbeiro, portMAX_DELAY / portTICK_PERIOD_MS);
      
      vPrintString("O barbeiro cortou cabelo.\n");
    } else {
      vPrintString("A barbearia está cheia, o cliente está saindo.\n");
      xTaskNotifyGive(barbeiro1);
      xSemaphoreGive(semMutex);
      
    }
  }
}

void vApplicationIdleHook( void ) {
  vPrintString("Rodando\n");
}

void loop() {}
