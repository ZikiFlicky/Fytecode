DATA
    bytes eb "Hello world!", 10, 0
CODE
start: ; This is currently not needed
    lea ax [bytes]
    int 2
    ; mov al '*' + 3
    ; int 1
    end
