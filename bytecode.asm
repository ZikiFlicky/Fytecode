DATA
    bytes eb "Hello world!", 10, 0

    x ew 3
CODE
start: ; This is currently not needed
    lea ax [bytes]
    int 2

    mov ah 100
    mov al 100
    int 3

    mov al 0
fill_y:
    cmp al 100
    je end_fill_y
    mov ah 0
fill_x:
    cmp ah 100
    je end_fill_x
    mov bl 3
    int 4
    int 5
    add ah 1
    jmp fill_x
end_fill_x:
    add al 1
    jmp fill_y
end_fill_y:

    ; mov ax 3
    mov ax 22
    int 0

    sub ax [x]
    mov [x] ax
    sub [x] 4
    mov ax [x]
    int 0

    mov [word x] 1
    cmp [word x] 0
    je no_do_something

    debug
no_do_something:

    ; mov al '*' + 3
    ; int 1
    end
