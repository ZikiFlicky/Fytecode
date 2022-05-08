DATA
    bytes eb "Some string here!", 13, 10, 0
    words ew 2 dup(0xFFEE, 0xDD33, 0x3)
CODE
start: ; This is currently not needed
    ; lea ax [bytes]
    ; int 2
    mov al 'c' + 3
    int 1
    end
