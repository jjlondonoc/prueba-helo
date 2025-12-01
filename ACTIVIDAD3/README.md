# Diseño de PCB

Diseñé la PCB usando el software **kicad**, implementé constraints y configuré todo el entorno de desarrollo para fabricar la PCB según las especificaciones de **JLCPCB** que es el fabricante que usa Helo. La configuraciones de vías, tracks, clearance, holes, etc. las escogí para que la PCB pueda ser fabricada en el estándar normal, es decir, sin sobrecostos.

También completé el BOM (Bill of Materials) de la tarjeta con los componentes reales a usar incluyendo su link y part number de LCSC que es lo que exigen JLCPCB por si se quisiera hacer el ensamblaje con ellos en china directamente.

Apliqué aislamiento galvánico en la tarjeta para que la parte de potencia AC estuviera aislada de la parte de control  y también creé agujeros de montaje por si son necesarios para la integración de la tarjeta en algún dispositivo.

Usé componentes con encapsulado SMD 0603 en su mayoría pensando en que sean fáciles de soldar a mano por parte de los técnicos y que sean sencillas las posibles reparqciones o reemplazo de componentes.

La serigrafía también la implementé siguiendo patrones de diseño profesionales de PCB.