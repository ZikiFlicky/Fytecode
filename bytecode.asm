start: ; This is currently not needed
    mov ax -1
    mov bx 4
    cmp ax bx
    jl do_debug
    jmp dont_do
do_debug:
    debug
dont_do:

;     jmp do_debug2
; do_debug:
;     debug
; do_debug2:
;     debug

    ; End program
    end
