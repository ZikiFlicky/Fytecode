DATA
    bytes eb 2 + 1 - 1, 3, 4, 4, 5
    words ew 0x33
CODE
start: ; This is currently not needed
    mov al 0xf3
    mov bx 1 + 2 - 22
    debug
    mov [byte 4 - (((bx + 1)))] al
    mov bx [0]
    debug
    end
