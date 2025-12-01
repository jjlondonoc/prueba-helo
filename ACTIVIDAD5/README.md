# Actividad 5

## Priorización de tareas

La prioridad máxima sería atender las necesidades de firmware que me pide el área comercial, ya que una tasa de fallo del 8% en 300 dispositivos me da margen para delegar la revisión de esas fallas a un segundo plano mientras atiendo las necesidades del nuevo firmware que finalmente compete al 100% del lote, por lo que primero debo garantizar el correcto funcionamiento de ese firmware, luego en el tiempo restante una vez quede listo e implementado el firmware revisaría las fallas en el 8% comenzando desde fallas en hardware hasta el firmware.

## Comunicaicón de decisiones

Mediante un cronograma de trabajo, correos, comunicados, reuniones, etc., creando tareas con prioridad y haciéndoles seguimientos a las tareas y a los asignados a esas tareas, siguiendo una metodología similar a los sprints, donde el equipo de trabajo comunique los avances y reporte los bugs o problemas encontrado.

## Riesgos identificados

El principal riesgo que identifico es el de hardware, ya que las fallas en hardware suelen ser críticas, la manera de mitigarlos sería primero con un fuerte control de calidad sobre todo en elementos críticos, de modo que se identifiquen eventos ireccuperables o "fatales", fuera de esto se considera que el equipo es operable y el firmware puede correr y ya este se encargaría de registrar los eventos o errores que puedan ocurrir, algo muy similar a los proyectos médicos que he desarrollado, en desarrollo de firmware médico que tiene una altísima exigencia en mitigación y manejo de errores la normativa siempre exige que se proteja al paciente, en este caso sería similar pero protegiendo siempre la integridad y correcto funcionamiento del dispositivo, esto mediante detección y manejo de eventos con severidad, y según esa severidad dle evento tomar decisiones respecto del equipo, pero siempre teniendo esa trazabilidad de los eventos por medio de logs.
