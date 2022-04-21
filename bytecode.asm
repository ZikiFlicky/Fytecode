start: ; This is currently not needed
;     mov ax -2
;     mov bx -3
;     cmp ax bx
;     jg do_debug
;     jmp dont_do
; do_debug:
;     debug
; dont_do:
    ; push 3
    ; pop ax
    ; mov ax 3
    ; debug
    ; mov bx ax
    ; add bx ax
    ; debug
    ; sub ax bx
    ; debug
    mov al 3
    add ax 1
    mov bh 22
    mov ah bh
    mov ax 0xFf
    debug
;    asdasd
;    asda jmp do_debug2
; do_debug:
;     debug
; do_debug2:
;     debug

    jmp func

do_end:
    ; End program
    end

proc func
    proc func2
    endp f
    debug
    jmp do_end
endp func
