DATA
    bytes eb "ABCDEFG", 2, 0
    words ew "ABCDEFG", 2
CODE
start: ; This is currently not needed
    mov al 0xf3
    mov bx 3 - -3
    debug
    mov [byte 4 - (((bx + 1)))] al
    mov bx [0]
    debug
    end
