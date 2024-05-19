section .data
    readonly_data dd 0x12345678  ; Datos de solo lectura

section .text
    global _start

_start:
    mov eax, 0xDEADBEEF
    mov [readonly_data], eax  ; Esto debería causar una excepción de protección general (GPF)
