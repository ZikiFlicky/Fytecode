start: ; This is currently not needed
    ; push 0xF3
    ; push 3
    ; call func
    ; debugstack
    ; pop ax
    ; debug ; expects AX: F3 00
; do_end:

    mov ax [0x4]
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
