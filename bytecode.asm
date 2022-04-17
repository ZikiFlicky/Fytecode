start: ; This is currently not needed
    mov ax 5
    ; debug
    mov bx ax
    debug
    add bx ax
    sub ax 3
    debug
    sub bx ax
    debug
;     jmp do_debug2
; do_debug:
;     debug
; do_debug2:
;     debug

    ; End program
    end
