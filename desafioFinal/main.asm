bits 16                   ; Define la arquitectura de 16 bits

inicio:
    mov ax, 0x0000        ; Inicializa los segmentos de datos en 0x0000
    mov ds, ax            ; Configura el segmento de datos DS
    mov es, ax            ; Configura el segmento adicional ES
    mov ss, ax            ; Configura el segmento de pila SS
    mov sp, 0x9000        ; Configura el puntero de pila en 0x9000 para mayor seguridad
    cli                   ; Deshabilita las interrupciones
    mov eax, cr0          ; Lee el contenido del registro CR0 en EAX
    or eax, 0x00000001    ; Activa el bit PE en EAX para habilitar el modo protegido
    mov cr0, eax          ; Escribe EAX de vuelta en CR0 para activar el modo protegido

    jmp SEG_CODIGO:modo_protegido_iniciar ; Realiza un salto lejano para actualizar CS y EIP al modo protegido

modo_protegido_iniciar:
    ; Código en modo protegido
    ; Configura los selectores de segmento
    mov ax, 0x10          ; Asigna 0x10 a AX, que corresponde al selector de segmento de datos
    mov ds, ax            ; Establece DS al nuevo segmento de datos
    mov es, ax            ; Establece ES al nuevo segmento adicional
    mov ss, ax            ; Establece SS al nuevo segmento de pila
    