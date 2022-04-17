start: ; This is currently not needed

    mov ax 1
    mov bx 1
fib:
    cmp ax 13
    je end_fib
    debug

    mov cx ax
    add cx bx
    mov ax bx
    mov bx cx

    jmp fib
end_fib:

;     jmp do_debug2
; do_debug:
;     debug
; do_debug2:
;     debug

    ; End program
    end
