start: ; This is currently not needed
;     mov ax -2
;     mov bx -3
;     cmp ax bx
;     jg do_debug
;     jmp dont_do
; do_debug:
;     debug
; dont_do:
    debug
    push 3
    debug

;     jmp do_debug2
; do_debug:
;     debug
; do_debug2:
;     debug

    ; End program
    end
