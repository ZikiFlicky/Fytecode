start: ; This is currently not needed
    ; push 0xF3
    ; push 3
    ; call func
    ; debugstack
    ; pop ax
    ; debug ; expects AX: F3 00
; do_end:

    mov bx -1
    mov ax [0xC - 0x1 + 0x2 - 0x3 + 0x2 + bx + bx]
    debug

    ; End program
    end

; proc func
;     ; debug
;     nop
;     nop
;     nop
;     jmp somehting
;     ret 22
; somehting:
;     ret 2
; endp func
