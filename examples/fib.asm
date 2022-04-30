CODE
start:
    push 11
    push 0
    push 1
    call fib
    debug
    end

; returns into ax
proc fib
    push bp
    mov bp sp
    push bx
    push cx
    push dx

    mov ax [bp + 8]
    cmp ax 0
    jg calculate
    mov ax [bp + 6]
    jmp return
calculate:
    mov ax [bp + 6] ; load a
    mov bx [bp + 4] ; load b
    ; create next
    mov cx ax
    add cx bx

    mov dx [bp + 8]
    sub dx 1
    push dx
    push bx
    push cx
    call fib
return:
    pop dx
    pop cx
    pop bx
    pop bp
    ret 6
endp fib
