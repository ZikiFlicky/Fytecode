start:
    mov ax 0
    mov bx 1
    mov dx 10 ; amount of rounds
fib:
    cmp dx 0
    je end_fib

    mov cx ax
    add cx bx
    mov ax bx
    mov bx cx

    sub dx 1
    jmp fib
end_fib:
    debug
