DATA
    SCREEN_WIDTH = 100
    SCREEN_HEIGHT = 100

    bytes eb "Hello world!", 10, 0

    left_y eb SCREEN_HEIGHT * 2
CODE
start: ; This is currently not needed
    mov al [byte 2 * bx]
    debug
    end
;     mov al 255
;     int 1

;     call newline

; no_do_something:

;     end

; proc newline
;     mov al 10
;     int 1
;     ret
; endp newline
