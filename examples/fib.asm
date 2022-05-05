CODE
start:
    push 11
    push 1
    push 0
    call fib
    debug
    end

; returns into ax
a = bp + 4
b = bp + 6
n = bp + 8
proc fib
    push bp
    mov bp sp
    push bx
    push cx
    push dx

    mov ax [n]
    cmp ax 0
    jg calculate
    mov ax [a]
    jmp return
calculate:
    mov ax [a] ; load a
    mov bx [b] ; load b
    ; create next
    mov cx ax
    add cx bx

    mov dx [n]
    sub dx 1
    push dx
    push cx
    push bx
    call fib

return:
    pop dx
    pop cx
    pop bx
    pop bp
    ret 6
endp fib
