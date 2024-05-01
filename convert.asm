global convertirFloatAInt
section .text
convertirFloatAInt:
    push rbp
    mov rbp, rsp
    sub rsp, 16          ; Alineación a 16 bytes y espacio para variables locales

    movss xmm0, dword [rdi]  ; Carga el parámetro 'numero', pasado por RDI, en xmm0
    cvttss2si rax, xmm0      ; Convierte el float de xmm0 a int, truncando
    add rax, 1               ; Suma 1 al valor entero
    mov [rbp-8], rax         ; Guarda el resultado en la variable local (opcional)

    mov rsp, rbp
    pop rbp
    ret

section .note.GNU-stack noalloc noexec nowrite progbits
