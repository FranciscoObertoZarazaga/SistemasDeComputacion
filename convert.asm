global convertirFloatAInt
section .text
convertirFloatAInt:
    cvttss2si rax, xmm0  ; Convierte el float en xmm0 a int en rax, truncando
    add rax, 1           ; Suma 1 al valor entero
    ret

section .note.GNU-stack noalloc noexec nowrite progbits
