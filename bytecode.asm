start: ; This is currently not needed
    push 0xF3
    push 3
    call func
    pop ax
    debug ; expects AX: F3 00
do_end:
    ; End program
    end

proc func
    ; debug
    ret 2
endp func
