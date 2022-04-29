DATA
    bytes eb 4, 2, 3, 4
    words ew 0x33, 0x44
CODE
start: ; This is currently not needed
    ; mov bx 2
    ; mov bp 0x10C
    lea ax [words + bx + 2]
    ; mov ax [bp + bx + bx + 1]
    debug
    debugstack
    ; mov ax bytes
    ; mov bx words

    ; End program
    end
