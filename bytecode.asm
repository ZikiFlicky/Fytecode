DATA
    bytes eb 2, 3, 4, 4, 5
    words ew 0x33
CODE
start: ; This is currently not needed
    mov al 0xf3
    mov [byte bx + 1] al
    mov bx [0]
    debug
    end
