
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
    xSemaphoreTake(semCliente, portMAX_DELAY);
    xSemaphoreTake(semMutex, portMAX_DELAY);
    vPrintString("O barbeiro tem clientes esperando!!.\n");
    clienteEsperando -= 1;
    xSemaphoreGive(semBarbeiro);
    xSemaphoreGive(semMutex);
    vPrintString("O barbeiro está cortando \n");
   
    vTaskDelay(umSegundo);
  }
}

void clienteTask(void* clienteT) {
  for (;;) {
    vTaskDelay(random(200, 1700) / portTICK_PERIOD_MS);
    
    xSemaphoreTake(semMutex, portMAX_DELAY );
    
    if (clienteEsperando < cadeira) {
        vPrintString("Vou sentar!!\n");
        clienteEsperando += 1;
        xSemaphoreGive(semCliente);
        xSemaphoreGive(semMutex); //Saindo da região crítica
      xSemaphoreTake(semBarbeiro, portMAX_DELAY);
      vPrintString("O barbeiro cortou meu cabelo.\n");
      vPrintString("\n--------------------------------------------\n");
    } else {
      vPrintString("A barbearia está cheia, o cliente está saindo.\n");
      xSemaphoreGive(semMutex);
      vTaskDelay(umSegundo);
    }
  }
}
void loop() {}
