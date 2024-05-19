bits 16                   ; Define la arquitectura de 16 bits

inicio:
    cli                    ; Deshabilita las interrupciones
    lgdt [gdtr]            ; Carga la GDT en el registro GDTR

    mov ax, 0x0000        ; Inicializa los segmentos de datos en 0x0000
    mov ds, ax            ; Configura el segmento de datos DS
    mov es, ax            ; Configura el segmento adicional ES
    mov ss, ax            ; Configura el segmento de pila SS
    mov sp, 0x9000        ; Configura el puntero de pila en 0x9000 para mayor seguridad
    
    mov eax, cr0          ; Lee el contenido del registro CR0 en EAX
    or eax, 0x00000001    ; Activa el bit PE en EAX para habilitar el modo protegido
    mov cr0, eax          ; Escribe EAX de vuelta en CR0 para activar el modo protegido

    jmp SEG_CODIGO:modo_protegido_iniciar ; Realiza un salto lejano para actualizar CS y EIP al modo protegido

section .pmode
bits 32                   ; Código en modo protegido
modo_protegido_iniciar:
    mov ax, SEG_DATOS     ; Asigna el selector de segmento de datos
    mov ds, ax            ; Establece DS al nuevo segmento de datos
    mov es, ax            ; Establece ES al nuevo segmento adicional
    mov ss, ax            ; Establece SS al nuevo segmento de pila
    ; Aquí iría más código para el funcionamiento en modo protegido

section .gdt_data
gdt_start:
    dq 0x0000000000000000   ; Descriptor nulo

; Descriptor de código
gdt_code:
    dw 0xFFFF               ; Límite bajo
    dw 0x0000               ; Base baja
    db 0x00                 ; Base media
    db 10011010b            ; Acceso (código, ejecutable, de lectura)
    db 11001111b            ; Granularidad (4K páginas, tamaño de 32-bit)
    db 0x00                 ; Base alta

; Descriptor de datos
gdt_data:
    dw 0xFFFF               ; Límite bajo
    dw 0x0000               ; Base baja
    db 0x00                 ; Base media
    db 10010010b            ; Acceso (datos, de escritura)
    db 11001111b            ; Granularidad (4K páginas, tamaño de 32-bit)
    db 0x00                 ; Base alta

gdt_end:

gdtr:
    dw gdt_end - gdt_start - 1 ; Límite de la GDT
    dd gdt_start               ; Dirección base de la GDT

section .bss
resb 8192                   ; Reserva espacio para la pila

SEG_CODIGO equ gdt_code - gdt_start + 0x08  ; Selector para el segmento de código
SEG_DATOS equ gdt_data - gdt_start + 0x08   ; Selector para el segmento de datos
