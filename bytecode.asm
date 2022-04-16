start: ; This is currently not needed
    mov ax 123
    mov bx ax
    jmp dodebug2

dodebug:
    debug
dodebug2:
    debug

    ; End program
    end
