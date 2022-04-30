DATA
    bytes eb 2, 3, 4, 4, 5
    words ew 0x33
CODE
start: ; This is currently not needed
    ; mov bx 2
    ; mov bp 0x10C
    ; mov bx 1
    ; mov bp 2
    ; mov ax [bytes + bx + bp - 2]
    ; debug
    mov ax 0xFF88
    mov [bytes + 1] ax
    debug
    mov ax [bytes + 2]
    debug
; some:
;     jmp some
    ; mov ax [bp + bx + bx + 1]
    ; debugstack
    ; mov ax bytes
    ; mov bx words

    ; End program
    end
