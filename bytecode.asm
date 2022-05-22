DATA
    bytes eb "Hello world!", 10, 0

    x ew 3
CODE
start: ; This is currently not needed
    mov al 255
    int 0

    call newline

no_do_something:

    ; mov al '*' + 3
    ; int 1
    end

proc newline
    mov al 10
    int 1
    ret
endp newline
