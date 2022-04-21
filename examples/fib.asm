start:
    mov dx 10 ; amount of rounds
    call fib
    end

proc fib
    push ax
    push bx
    push cx
    push dx

    mov ax 0
    mov bx 1
fib_loop:
    cmp dx 0
    je end_fib

    mov cx ax
    add cx bx
    mov ax bx
    mov bx cx

    sub dx 1
    jmp fib_loop
end_fib:
    debug

    pop dx
    pop cx
    pop bx
    pop ax
    ret
endp fib
