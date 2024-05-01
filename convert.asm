global convertirFloatAInt   ; Hace la función 'convertirFloatAInt' accesible desde otros archivos.

section .text               ; Comienza la sección de texto, donde reside el código de la función.

convertirFloatAInt:         ; Etiqueta que define el inicio de la función 'convertirFloatAInt'.
    cvttss2si rax, xmm0     ; Instrucción que convierte el valor de punto flotante contenido en el registro 'xmm0' 
                            ; a un valor entero utilizando la conversión escalar de punto flotante a entero (con truncamiento) 
                            ; y lo almacena en el registro 'rax'. El truncamiento descarta cualquier parte fraccionaria del número.
    
    add rax, 1              ; Suma 1 al valor entero que ahora está en 'rax'. Esto es útil para ajustar el valor 
                            ; convertido en alguna lógica específica del programa (por ejemplo, redondear hacia arriba).

    ret                     ; Retorna de la función y pasa el control de vuelta a la función llamadora.
                            ; El valor de retorno se encuentra en 'rax', conforme a la convención de llamadas.

section .note.GNU-stack noalloc noexec nowrite progbits
                            ; Esta sección le indica al enlazador que la pila (stack) no debe ser 
                            ; ejecutable, lo que ayuda a prevenir ciertos tipos de ataques de seguridad.
                            ; 'noalloc' indica que no hay necesidad de asignar espacio para esta sección,
                            ; 'noexec' indica que la sección no debe ser ejecutable,
                            ; y 'nowrite' indica que no debe ser escribible.
