/**
 * @brief     Actividad 2 prueba técnica para Helo
 * @author    Juan José Londoño - Biomedical Engineer M.Sc
 * @date      2025-11-30
 * @copyright (c) 2025 Helo. All rights reserved.
 *
 * @note Target board: ESP32 Dev Module
 *
 * @details
 * Hago las 3 peticiones POST al backend que me dieron en el documento.
 * De una vez en el setup hago las 3 peticiones para generar los 3 tipos de
 * mensajes de respuesta del servidor, no hago más nadad ene l loop.
 * 
 */

#include <WiFi.h>
#include <HTTPClient.h>

/* Conexión a red wifi*/
const char* ssid = "CLARO_WIFI180";
const char* password = "CLAROI180";

/* API de Helo */
const char* server_url = "https://api.helo.v1.intelmotics.com/services/prueba_tecnica/hola.php";

/* Body del JSON correcto */
String id_ok = "1212";
String token_ok = "1010101010";

/* Body del JSON con valores incorrecros  */
String id_error = "1211";
String token_error = "1010101011";

void setup() {
  Serial.begin(115200);
  delay(1000);

  /* Conectar a la red */
  WiFi.begin(ssid, password);
  Serial.println("Conectando a la red ...");

  while(WiFi.status() != WL_CONNECTED) delay(500);

  Serial.print("Conectado a IP: ");
  Serial.println(WiFi.localIP());

  /* Hago de una vez las 3 peticiones para recibir los 3 mensajes */
  post_request();
}

void loop() {
  /* Ya se hicieton las peticiones */
}

void post_request() {

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("No hay conexión WiFi");
    return;
  }

  HTTPClient http;

  http.begin(server_url);

  /* Header según el documento */
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");

  String json_body;
  int http_response;
  String payload;

  Serial.println("PETICIÓN 1: Body correcto");
  Serial.println("======================================");

  /* Body correcto */
  json_body = "id=" + id_ok + "&token=" + token_ok;

  Serial.print("Body: ");
  Serial.println(json_body);

  http_response = http.POST(json_body);

  Serial.print("Código de respuesta HTTP: ");
  Serial.println(http_response);

  /* Body obtenido */
  payload = http.getString();
  Serial.println("Respuesta del servidor:");
  Serial.println(payload);

  Serial.println("PETICIÓN 2: Body inccorrecto");
  Serial.println("======================================");

  /* Body con campos mal formados */
  json_body = "idd=" + id_ok + "&token=" + token_ok;

  Serial.print("Body: ");
  Serial.println(json_body);

  http_response = http.POST(json_body);

  Serial.print("Código de respuesta HTTP: ");
  Serial.println(http_response);

  /* Body obtenido */
  payload = http.getString();
  Serial.println("Respuesta del servidor:");
  Serial.println(payload);

  Serial.println("PETICIÓN 3: Body con valores incorrectos");
  Serial.println("======================================");

  /* Body con valores incorrectos */
  json_body = "id=" + id_error + "&token=" + token_error;

  Serial.print("Body: ");
  Serial.println(json_body);

  http_response = http.POST(json_body);

  Serial.print("Código de respuesta HTTP: ");
  Serial.println(http_response);

  /* Body obtenido */
  payload = http.getString();
  Serial.println("Respuesta del servidor:");
  Serial.println(payload);

  http.end();
}

