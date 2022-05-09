DATA
    bytes eb "Hello world!", 10, 0
CODE
start:
    lea ax [bytes]
    int 2
    end
