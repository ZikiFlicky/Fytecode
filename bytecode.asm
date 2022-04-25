DATA
    bytes eb 1, 2, 3, 4
    words ew 0x33, 0x44
CODE
start: ; This is currently not needed
    mov ax bytes
    mov bx words
    debug

    ; End program
    end
