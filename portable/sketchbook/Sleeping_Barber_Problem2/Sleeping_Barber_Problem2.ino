
#include <Arduino_FreeRTOS.h>
#include "event_groups.h"
#include "semphr.h"

#define cadeira 2
#define cliente 3



void vPrintString( const char *pcString )
{
  /* Print the string, suspending the scheduler as method of mutual
  exclusion. */
  vTaskSuspendAll();
  {
      Serial.print(pcString);
      Serial.flush();
  //  printf( "%s", pcString );
  //  fflush( stdout );
  }
  xTaskResumeAll();

    /* Allow any key to stop the application running.  A real application that
    actually used the key value should protect access to the keyboard too. */
  if( Serial.available() )
  {
    vTaskEndScheduler();
  }
}

void vPrintStringAndNumber( const char *pcString, unsigned portLONG ulValue )
{
  /* Print the string, suspending the scheduler as method of mutual
  exclusion. */
  vTaskSuspendAll();
  {
//    printf( "%s %lu\r\n", pcString, ulValue );
//    fflush( stdout );
      Serial.print(pcString);
      Serial.write(' ');
      Serial.println(ulValue);
      Serial.flush();
  }
  xTaskResumeAll();

  /* Allow any key to stop the application running. */
  if( Serial.available() )
  {
    vTaskEndScheduler();
  }
}


const TickType_t umSegundo = 1000 / portTICK_PERIOD_MS;

uint8_t clienteEsperando;

SemaphoreHandle_t semBarbeiro, semMutex;
EventGroupHandle_t semCliente;

void eventGroupGive(EventGroupHandle_t sem) {
  EventBits_t bits = xEventGroupGetBits(sem);
  xEventGroupClearBits(sem, bits);
  //bits = (EventBits_t)((int)bits + 1);
  if (!bits) {
    bits = 1;
  } else {
    bits <<= 1;
  }
  xEventGroupSetBits(sem, bits);
}

void eventGroupTake(EventGroupHandle_t sem) {
  EventBits_t bits = xEventGroupWaitBits(sem, 0xff, pdTRUE, pdFALSE, portMAX_DELAY);
  //bits = (EventBits_t)max(0, (int)bits - 1);
  bits >>= 1;
  xEventGroupSetBits(sem, bits);
}

void barbeiroTask(void* barbeiro);

void clienteTask(void* clienteT);

void setup() {
  Serial.begin(9600);

  if (xTaskCreate(barbeiroTask, "Barbeiro",100, NULL, 2, NULL) == pdFAIL) {
    vPrintString("Houve problema de criação no barbeiroTask.\n");
    for (;;) {}
  }

  for (uint8_t i = 0;  i < cliente; i++) {
    if (xTaskCreate(clienteTask, "cliente", 120, NULL, 1, NULL) == pdFAIL) {
      vPrintString("Houve problema de criação de clienteTask.\n");
    }
  }

  semCliente = xEventGroupCreate();
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
    eventGroupTake(semCliente);

   
    xSemaphoreTake(semMutex, portMAX_DELAY);
    
    clienteEsperando -= 1;
    if(clienteEsperando == 0){
    vPrintStringAndNumber("Barbeiro acordando !! |clientes esperando!! |Nº cadeira : ",clienteEsperando+1);
    }
    xSemaphoreGive(semBarbeiro);
    xSemaphoreGive(semMutex);
    vPrintString("O barbeiro está cortando \n");
   
    vTaskDelay(umSegundo);
  }
}

void clienteTask(void* clienteT) {
  for (;;) {
    vTaskDelay(random(200, 1500) / portTICK_PERIOD_MS);
    
    xSemaphoreTake(semMutex, portMAX_DELAY );
    
    if (clienteEsperando <= cadeira) {
       
        clienteEsperando += 1;
        //xSemaphoreGive(semCliente);
        eventGroupGive(semCliente);
        
        xSemaphoreGive(semMutex); //Saindo da região crítica
      xSemaphoreTake(semBarbeiro, portMAX_DELAY);
      
      vPrintStringAndNumber("Cortou meu cabelo Nº cadeira: ",clienteEsperando);
       
      
      
      vPrintString("\n--------------------------------------------\n");
    } else {
      vPrintString("A barbearia está cheia, o cliente está saindo.\n");
      xSemaphoreGive(semMutex);
      vTaskDelay(umSegundo);
    }
  }
}

void loop() {}
