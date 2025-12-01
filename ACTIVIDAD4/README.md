# Actividad 4

## 1. Hipótesis técnicas

1. Error en el diseño de la PCB: el reboot puede ser debido a que un mecanismo de reset (el pin o un botón o lo que se haya usado en el diseño de la tarjeta) o el arreglo de transistores usados para entrar en modo bootloader tienen alguna falla de diseño o en la PCB como tal por un error en la fabricación por parte del fabricante.

2. Error en el firmware: el firmware puede presentar algún bloqueo debido a una tarea demasiado larga, lo que peude ocasionar que el watchdog timer del ESP se dispare y genere un reboot del núcleo.

3. La falla de la conexión WiFi y las lecturas incorrectas pueden deberse a que no se hizo un buen blindaje contra EMI o interferencias de radiofrecuencias del dispositivo y existe ruido eléctrico que está afectando el comportamiento.

## 2. Pruebas que realizaría

Haría pruebas en un ambiente controlado aislando el ruido electromagnético en los equipos que están fallando, revisaría el GPIO o los GPIO que están leyendo el sensor para ver si puede deberse a un daño en algún módulo del ESP, por ejemplo que esté fallando uno de los ADC o el SPI, luego de descartar ambas cosas revisaría el código para ver como se gestionan los eventos de desconexión WiFi por si la conexión es deficiente y finalmente revisaría de nuevo la PCB y el mecanismo de reboot y entrada a modo bootloader de los equipos dañados a ver si algún mal contacto o un componente defectuoso está ocasionando el reboot.

## 3. Mejoras en hardware

Ya que son evebtos aleatorios y me mencionan que el equipo está en un tablero industrial, haría más robusto el diseño de protección contra EMI en la tarjeta, agregaría jaulas de Faraday en lso componentes críticos, despejaría la antena de pistas o planos de tierra cercanos ya que esto genera reflexiones de la señal y afectan la impedancia del diseño RF de las antenas, además implementaría circuitos más robustos para boot del sistema, o para entrada en modo bootloader.

## 4. Mejoras en firmware

Implementaría un sistema de eventos (colas y buffers circulares) que gestionen los eventos de desconexión de la comunicaicón WiFi ya que es indispensable no perder esos datos si ocurre una desconexión a WiFi, un sistema de eventos que procese los eventos desde una cola FIFO permite mejorar esto. Además haría un sistema de chequeo continuo de la salud del sistema, e implementaría un watchdog timer de verdad para saber si en algún momento hay algún bloqueo de CPU por alguna tarea que pueda bloquear el sistema y con ello forzar reset. Tambien implementaría un sistema de logs, que informen de los eventos, de las causas de reset y de errores que se produzcan, esos logs si es posible enviarlos de forma remota con WiFi los pondría a enviarse continuamente, si no los guardaría en memoria no volátil del ESP (al menos los logs críticos) para luego hacer seguimiento de qué pasó.

## 5. Controles

En producción haría obligatorio pruebas de Jitter en las antenas para evaluar primero que nada el sistema de transmisión y ver que en efecto la impedancia de 50 ohms de la antena se cumple y el jitter es mínimo para que el WiFi opere de forma eficiente, además haría una reviisón fuerte de los componentes críticos del sistema de booteo e implementaría una serie de pruebas en ambiente controlado ante situaciones críticas para ver que en efecto los logs funcionen bien y registren loe eventos de forma correcta antes de salir a producción.

