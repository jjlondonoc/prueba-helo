/**
 * @brief     Actividad 1 prueba técnica para Helo
 * @author    Juan José Londoño - Biomedical Engineer M.Sc
 * @date      2025-11-30
 * @copyright (c) 2025 Helo. All rights reserved.
 *
 * @note Target board: ESP32 Dev Module
 *
 * @details
 * Implementé todo el firmware con FreeRTOS para aprovechar
 * al máximo las capacidade sdel ESP e implementé una aqruitectura por eventos, 
 * utilicé dos interrupciones externas
 * una generada por el interruptor magnético y la otra la generé
 * mediante el RTC. Lo configuré para generar una señal
 * cuadrada de 1Hz en el pin SQ, esa señal la usé como mi tick del sistema.
 * Esa señal genera una interrupción cada segundo, de esa forma puedo tener
 * un tick de 1 segundo proveniente del RTC y ese tick lo usé para un contador de 
 * ticks que cuando llega a 60 (1 minuto) lee el DHT22 para sensar temperatura y 
 * humedad.
 * El interruptor magnético lo conecté con el común (COM) conectado
 * a tierra y el Normalmente cerrado (NC) lo conecté a un GPIO con PULLUP interno
 * activado, en el setup hice un digitalRead del pin para saber el estado
 * inicial ya que en la interrupción detecto cambios en el pin
 * y actualizo una variable global que es la que me dice el estado del
 * interruptor.
 * Utilicé FreeRTOS ya que quería optimizar el uso de la CPU (el loop quedó vacío
 * y se podría usar para muchas otras cosas), así que evité hacer polling al loop
 * tontamente y mejor creé una tarea que atiende eventos, esa tarea sólo despierta
 * a la CPU cuando un evento es puesto en la cola de eventos, esa cola soporta
 * hasta 10 eventos por si hay un cuello de botella algún día o algo y se atrasan los
 * eventos, ahí quedan guardados en la cola y se procesan cuando el ESP pueda,
 * esto es una medida de seguridad para no perder datos.
 * Las ISR generan los eventos y la tarea que creé los maneja, es un handler de eventos.
 * Finalmente creé dos funciones para formatear los JSON y escribirlos en la SD
 * como me solicitaorn.
 * Usé patrones de diseño de firmware embebido similares a los que uso cuando
 * desarrollo firmware médico que son bastante exigentes, esos patrones incluyen no
 * usar "número mágicos " en el códifo, por eso hago varios #define y además uso
 * patrones de nombres estándar usados en un estándar llamado MISRA-C muy usado
 * en firmware médico que es de muy alta exigencia y calidad, por eso creé structs
 * y enums con esos patrones de nombres.
 * Para la documentación, en este encabezado del documento usé Doxygen que es
 * el estándar más usado para firmware profeisonal en la industria y espero
 * poder implementar toda esta normativa y buenas prácticas en Helo :)
 */

#include <string.h>
#include <Wire.h>
#include <RTClib.h>
#include <SPI.h>
#include <SD.h>
#include <DHT.h>

#define UART_BAUD_RATE (115200U) /* Baudios para Serial */

#define SECONDS_FOR_MEASURE 60U /* Segundos para realizar las mediciones de Tempreatura y humedad */

#define DHTTYPE DHT22 /* Tipo de swnsor de temperatura y humedad */

#define PIN_RTC_ISR         4U  /* GPIO para interrupción del RTC */
#define PIN_INTERRUPT_ISR  16U  /* GPIO pata interrupción del interruptor */
#define PIN_CS_SD           5U  /* CS para el SPI de la micro SD */
#define PIN_DHT22          17U  /* GPIO para DHT22 */

/* Tipos de variables */
#define TYPE_HUMIDITY     10U
#define TYPE_TEMPERATURE  11U
#define TYPE_STATE         7U

#define TX_BUFF_LEN 512U /* Tamaño buffer para guardar los JSON antes d eescribirlos */
#define DATE_LEN     20U /* YYYY-MM-DD HH:MM:SS + \0 */

#define EVENT_QUEUE_LEN 10U /* Capacidad de la cola de eventos */

#define JSON_STATE_LEN 16U /* Tamaño del campo "estado" para tener márgen */

#define JSON_ARRAY_MEASURES_LEN 2U /* Tamaño de array de JSON para medidas del sensor */

#define DEBOUNCE_TIME_MS 500 /* Tiempo de espera para antirrebotes en ms*/

#define PATH_NAME "/HELO_PRUEBA.txt" /* Nombre para archivo de texto */

static volatile bool s_switch_state = 0;  /* 1 = Abierto, 0 = Cerrado */

/* ===================== Objetos ===================== */

static RTC_DS1307 rtc; /* Objeto RTC */
static DHT dht(PIN_DHT22, DHTTYPE); /* Objeto sensor de temperatura/humedad */

/* ===================== Manejo de JSON ===================== */

/* Estructura del objeto JSON */
struct json_object_t {
  uint8_t  type;
  float    state;
  DateTime date;
};

/* Buffer de TX para serializar los JSON */
static char json_tx_buff[TX_BUFF_LEN];

/* ===================== Helpers ===================== */

static bool format_json_object(char *buff, const size_t buff_size, const json_object_t *json) {

  /* Estado*/
  char format_state[JSON_STATE_LEN];
  int state_str = snprintf(
    format_state,
    sizeof(format_state),
    "%.1f",
    json->state
  );

  if (state_str < 0 || state_str >= (int)sizeof(format_state)) return false;

  /* Fecha "YYYY-MM-DD HH:MM:SS" */
  char format_date[DATE_LEN];
  int str_date = snprintf(
    format_date,
    sizeof(format_date),
    "%04d-%02d-%02d %02d:%02d:%02d",
    json->date.year(),
    json->date.month(),
    json->date.day(),
    json->date.hour(),
    json->date.minute(),
    json->date.second()
  );

  if (str_date < 0 || str_date >= (int)sizeof(format_date)) return false;

  /* Body del JSON */
  int str_body = snprintf(
    buff,
    buff_size,
    "{\"tipo\":\"%u\",\"estado\":\"%s\",\"fecha\":\"%s\"}",
    json->type,
    format_state,
    format_date
  );

  if(str_body < 0 || str_body >= (int)buff_size) return false;

  return true;
}

/* Para un sólo objeto JSON */
static bool format_json(const json_object_t *json) {
  return format_json_object(json_tx_buff, sizeof(json_tx_buff), json);
}

/* Para un array de objetos JSON */
static bool format_json_array(const json_object_t *json_array, size_t count) {
  size_t offset = 0;
  size_t remaining = sizeof(json_tx_buff);

  if(remaining < 2) return false; /* Se necesita al menos [ y \0*/

  json_tx_buff[offset++] = '[';
  remaining--;

  for(size_t i = 0; i < count; i++) {
    if(i > 0) {
      if(remaining < 2) return false;
      json_tx_buff[offset++] = ','; /* Separación entre objetos*/
      remaining--;
    }

    if(!format_json_object(json_tx_buff + offset, remaining, &json_array[i])) {
      return false;
    }

    size_t len = strlen(json_tx_buff + offset);

    offset += len;
    remaining -= len;
  }

  if (remaining < 2) return false;

  json_tx_buff[offset++] = ']';
  json_tx_buff[offset] = '\0';

  return true;

}

static bool sd_append_line(const char *line) {

  File file = SD.open(PATH_NAME, FILE_APPEND);
  if (!file) {
    Serial.println("Error abriendo archivo de texto");
    return false;
  }

  size_t written = file.println(line);

  file.close();

  return (written > 0);
}

/* ===================== Cola y eventos para el sistema operativo =====================*/

/* Eventos del sistema */
enum system_event_t {
  EVENT_TIME_TICK,              /* Evento que se dispara cuando se cumple 1 minuto en el RTC */
  EVENT_INTERRUPT_STATE_CHANGE  /* Evento qus se dispara cuando cambia el estado del intreuptor */
};

/* Cola de eventod */
QueueHandle_t event_queue = nullptr;

/* ===================== Interrupcuones ===================== */

/* Interrupción del RTC */
void IRAM_ATTR isr_rtc() {
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  system_event_t event = EVENT_TIME_TICK;
  xQueueSendFromISR(event_queue, &event, &xHigherPriorityTaskWoken);
  if (xHigherPriorityTaskWoken) portYIELD_FROM_ISR();
}

/* Interrupción del interruptor  */
void IRAM_ATTR isr_switch() {
  static uint32_t last_ms = 0;
  uint32_t now = millis();

  /* Antirrebote */
  if (now - last_ms < DEBOUNCE_TIME_MS) return;
  last_ms = now;

  /* Cambia el estado del switch */
  s_switch_state = !s_switch_state;

  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  system_event_t event = EVENT_INTERRUPT_STATE_CHANGE;
  xQueueSendFromISR(event_queue, &event, &xHigherPriorityTaskWoken);
  if (xHigherPriorityTaskWoken) portYIELD_FROM_ISR();
}

/* ===================== Tarea ===================== */

/* Handler de eventos */
bool state = false;
void event_task(void *pvParams) {
  system_event_t event;
  uint8_t seconds = 0;
  while(1) {
    if(xQueueReceive(event_queue, &event, portMAX_DELAY) == pdTRUE) {
      switch(event) {
        case EVENT_TIME_TICK:
          seconds++;
          if(seconds >= SECONDS_FOR_MEASURE) {
            seconds = 0;

            json_object_t json_array[JSON_ARRAY_MEASURES_LEN];

            /* Leer temperatura y humedad */
            float humidity = dht.readHumidity();
            float temperature = dht.readTemperature();

            /* Si todo se leyó bien comtimuamos */
            if (isnan(humidity) || isnan(temperature)) {
              Serial.println("Error leyendo DHT22");
              break;
            }
            
            DateTime now = rtc.now();

            /* Serializar datos a JSON */
            json_array[0].type = TYPE_HUMIDITY;
            json_array[0].state = humidity;
            json_array[0].date = now;

            json_array[1].type = TYPE_TEMPERATURE;
            json_array[1].state = temperature;
            json_array[1].date = now;

            if (format_json_array(json_array, JSON_ARRAY_MEASURES_LEN)) {
              if(sd_append_line(json_tx_buff)) Serial.println("JSON escrito en SD");
              else Serial.println("Error en SD");
            } 
            
            else Serial.println("Error en JSON");
          }
          break;
        
        case EVENT_INTERRUPT_STATE_CHANGE:
          bool state = s_switch_state;
          json_object_t json_obj;
          json_obj.type  = TYPE_STATE;    
          json_obj.state = state;
          json_obj.date  = rtc.now();

          if (format_json(&json_obj)) {
            if(sd_append_line(json_tx_buff)) Serial.println("JSON escrito en SD");
            else Serial.println("Error en SD");
          } 
            
          else Serial.println("Error en JSON");
          break;
      }
    }
  }
}

void setup() {
  Serial.begin(UART_BAUD_RATE);
  delay(1000);

  /* Esperar al UART dle PC */
  while(!Serial) delay(100);

  /* Iniciar I2C*/
  Wire.begin();

  /* Verificar el RTC */
  Serial.println(rtc.begin() ? "RTC iniciado correctamnte" : "Error con el RTC");
  
  /* Configurar RTC con hora en que se compile este archvio */
  if(!rtc.isrunning()) rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));

  /* Configurar señal cuadrada de RTC para usar como tick */
  rtc.writeSqwPinMode(DS1307_SquareWave1HZ);

  /* Configurar sensor de temperatura y humedad */
  dht.begin();

  /* Configurar microSD*/
  if (SD.begin(PIN_CS_SD)) Serial.println("SD iniciada correctamente");
    
  else {
    Serial.println("Error iniciando SD");
    while(1); /* No se puede escribir en la SD */
  }

  /* Crear cola de eventos */
  event_queue = xQueueCreate(EVENT_QUEUE_LEN, sizeof(system_event_t));

  /* Pines de interrupción */
  pinMode(PIN_RTC_ISR, INPUT);
  pinMode(PIN_INTERRUPT_ISR, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(PIN_RTC_ISR), isr_rtc, RISING);
  attachInterrupt(digitalPinToInterrupt(PIN_INTERRUPT_ISR), isr_switch, CHANGE);

  /* Estado inicial del interruptor */
  s_switch_state = digitalRead(PIN_INTERRUPT_ISR);

  /* Tarea para loe eventos */
  xTaskCreatePinnedToCore(event_task, "event_task", 4096, nullptr, 1, nullptr, 1);
}

void loop() {
  /* Todo lo manejan las interrupciones y el RTOS */
  /* El loop queda libre para cualquier otra cosa */
}
