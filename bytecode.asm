DATA
    bytes eb 3 dup(0xFF, 0xEE, 0xDD, 0xBC)
    words ew 2 dup(0xFFEE, 0xDD33, 0x3)
CODE
start: ; This is currently not needed
    mov al 0xf3
    mov bx 3 - -3
    debug
    mov [byte 4 - (((bx + 1)))] al
    mov bx [0]
    debug
    end
