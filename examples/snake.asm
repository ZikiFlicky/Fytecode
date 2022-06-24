DATA
    CUBE_SIZE = 10
    CUBES_WIDTH = 20
    CUBES_HEIGHT = 20

    MAX_SNAKE_LENGTH = 24
    amount_snake_entries ew 1
    snake_entries eb MAX_SNAKE_LENGTH dup(0, 0)
    dir_x eb 1
    dir_y eb 0

    ; Initialized later
    new_dir_x eb 0
    new_dir_y eb 0

    last_draw_time_sec ew 0
    last_draw_time_ms ew 0

    ; Apple position
    apple_x eb 0
    apple_y eb 0

    ; String to be printed when snake big enough
    game_win_string eb "Game finished! You won", 10, 0
    ; String to be printed when snake hits itself
    game_lose_string eb "Game finished! You lost", 10, 0
    ; Suffix string for number of points
    string_points eb " points", 10, 0
CODE
start: ; This is currently not needed
    ; Initialize screen
    mov ah CUBE_SIZE * CUBES_WIDTH
    mov al CUBE_SIZE * CUBES_HEIGHT
    int 3

    call generate_apple
    call move_snake
    call store_draw_time
    ; Store a copy of the direction
    mov al [dir_x]
    mov [new_dir_x] al
    mov al [dir_y]
    mov [new_dir_y] al

mainloop:
    int 7 ; Get USB scancode
    cmp ax 0
    je end_check_press

    cmp bx 0x50 ; Left arrow
    je pressed_left_arrow
    cmp bx 0x4F ; Right arrow
    je pressed_right_arrow
    cmp bx 0x52 ; Up arrow
    je pressed_up_arrow
    cmp bx 0x51 ; Down arrow
    je pressed_down_arrow

    jmp end_check_press

pressed_left_arrow:
    cmp [byte dir_x] 1 ; Opposite direction
    je end_check_press
    mov [byte new_dir_x] -1
    mov [byte new_dir_y] 0
    jmp end_check_press
pressed_right_arrow:
    cmp [byte dir_x] -1 ; Opposite direction
    je end_check_press
    mov [byte new_dir_x] 1
    mov [byte new_dir_y] 0
    jmp end_check_press
pressed_up_arrow:
    cmp [byte dir_y] 1 ; Opposite direction
    je end_check_press
    mov [byte new_dir_x] 0
    mov [byte new_dir_y] -1
    jmp end_check_press
pressed_down_arrow:
    cmp [byte dir_y] -1 ; Opposite direction
    je end_check_press
    mov [byte new_dir_x] 0
    mov [byte new_dir_y] 1
    jmp end_check_press
end_check_press:

    call has_enough_time_elapsed
    cmp ax 0
    je no_redraw

    ; Set new x direction
    mov al [new_dir_x]
    mov [dir_x] al
    ; Set new y direction
    mov al [new_dir_y]
    mov [dir_y] al

    call move_snake
    call store_draw_time

    ; Store a new copy of the direction
    mov al [dir_x]
    mov [new_dir_x] al
    mov al [dir_y]
    mov [new_dir_y] al

no_redraw:

    ; Play game while snake length is smaller than MAX_SNAKE_LENGTH
    cmp [word amount_snake_entries] MAX_SNAKE_LENGTH
    jb mainloop

    ; Print a win message
    lea ax [game_win_string]
    int 2
    end

; Store the current draw time so that we know when we need to draw again
proc store_draw_time
    push ax
    push bx

    int 6
    mov [last_draw_time_sec] ax
    mov [last_draw_time_ms] bx

    pop bx
    pop ax
    ret
endp store_draw_time

; Returns boolean into ax saying whether enough time has elapsed since the last draw, so that we know to draw again
current_sec = bp - 2
current_ms = bp - 4
diff_sec = bp - 6
diff_ms = bp - 8
proc has_enough_time_elapsed
    push bp
    mov bp sp
    sub sp 8
    push bx

    int 6
    mov [current_sec] ax
    mov [current_ms] bx

    mov ax [current_sec]
    sub ax [last_draw_time_sec]
    mov [diff_sec] ax

    mov ax [current_ms]
    cmp [last_draw_time_ms] ax
    ja ms_bigger

    sub ax [last_draw_time_ms]
    jmp end_ms_check

ms_bigger:
    ; The last ms is bigger than the current one so we need to convert one of the diff seconds into milliseconds
    ; So we don't get a negative result
    dec [word diff_sec]
    add ax 1000
    sub ax [last_draw_time_ms]

end_ms_check:

    mov [diff_ms] ax

    ; Until proven else, enough time has elapsed
    mov ax 1

    cmp [word diff_sec] 0
    ja enough_time_elapsed

    cmp [word diff_ms] 200
    ja enough_time_elapsed

    ; Actually not enough time has elapsed so return 0
    mov ax 0

enough_time_elapsed:

    pop bx
    add sp 8
    pop bp
    ret
endp has_enough_time_elapsed

; Draw a rectangle of a constant size on the screen
x = bp + 4
y = bp + 5
color = bp + 6
proc draw_cube
    push bp
    mov bp sp
    push ax
    push bx
    push cx

    ; Calculate actual x
    mov al [x]
    mov bl CUBE_SIZE
    mul bl
    mov [x] al

    ; Calculate actual y
    mov al [y]
    mov bl CUBE_SIZE
    mul bl
    mov [y] al

    mov bx [color]

    mov ch 0
draw_y_times:

    mov cl 0
draw_width_line:
    ; X for call
    mov ah [x]
    add ah cl
    ; Y for call
    mov al [y]
    add al ch
    ; Color for call
    int 4 ; Draw pixel

    inc cl
    cmp cl CUBE_SIZE
    jb draw_width_line

    inc ch
    cmp ch CUBE_SIZE
    jb draw_y_times

    pop cx
    pop bx
    pop ax
    pop bp
    ret 4
endp draw_cube

; Add a tail entry to the snake
tail_x = bp + 4
tail_y = bp + 5
proc snake_add_entry
    push bp
    mov bp sp
    push ax
    push bx

    mov bx [amount_snake_entries]
    shl bx 1
    mov al [tail_x]
    mov [snake_entries + bx + 0] al
    mov al [tail_y]
    mov [snake_entries + bx + 1] al

    inc [word amount_snake_entries]

    pop bx
    pop ax
    pop bp
    ret 2
endp snake_add_entry

pos_x = bp + 4
pos_y = bp + 5
proc is_position_inside_snake
    push bp
    mov bp sp
    push bx
    push cx

    mov cx 0
    lea bx [snake_entries]
loop_snake:
    mov al [bx + 0] ; X
    cmp al [pos_x]
    jne position_not_equal
    mov al [bx + 1] ; Y
    cmp al [pos_y]
    jne position_not_equal

    ; If we got here the positions are equal so return 1
    mov ax 1
    jmp end_search_position

position_not_equal:
    inc cx
    add bx 2
    cmp cx [amount_snake_entries]
    jl loop_snake

    ; Return value
    mov ax 0

end_search_position:

    pop cx
    pop bx
    pop bp
    ret 2
endp is_position_inside_snake

; Move the snake both graphically and in data
tail_x = bp - 1
tail_y = bp - 2
new_x = bp - 3
new_y = bp - 4
proc move_snake
    push bp
    mov bp sp
    sub sp 4
    push ax
    push bx
    push cx

    ; Load tail x and y
    mov bx [amount_snake_entries]
    shl bx 1
    sub bx 2
    mov al [snake_entries + bx]
    mov ah [snake_entries + bx + 1]
    mov [tail_x] al ; Store x
    mov [tail_y] ah ; Store y

    ; Shift all snake entries
    mov cx [amount_snake_entries]
shift_snake_body:
    dec cx
    cmp cx 0
    je end_shift_snake_body

    ; Store snake entry to copy
    mov bx cx
    shl bx 1
    sub bx 2
    mov al [snake_entries + bx] ; X
    mov ah [snake_entries + bx + 1] ; Y

    ; Current entry
    add bx 2
    mov [snake_entries + bx + 0] al ; X
    mov [snake_entries + bx + 1] ah ; Y

    jmp shift_snake_body

end_shift_snake_body:

    ; Store new x and y
    mov al [dir_x]
    add al [snake_entries + 0]
    mov [new_x] al
    mov al [dir_y]
    add al [snake_entries + 1]
    mov [new_y] al

    cmp [byte new_x] 0
    jl x_too_small
    cmp [byte new_x] CUBES_WIDTH
    jge x_too_big

    jmp x_fine

x_too_small:
    add [byte new_x] CUBES_WIDTH
    jmp x_fine

x_too_big:
    sub [byte new_x] CUBES_WIDTH

x_fine:

    cmp [byte new_y] 0
    jl y_too_small
    cmp [byte new_y] CUBES_HEIGHT
    jge y_too_big

    jmp y_fine

y_too_small:
    add [byte new_y] CUBES_HEIGHT
    jmp y_fine

y_too_big:
    sub [byte new_y] CUBES_HEIGHT

y_fine:

    ; Load head and check if it doesn't touch the body of the snake
    mov al [new_x]
    mov ah [new_y]
    push ax
    call is_position_inside_snake
    cmp ax 0
    je new_position_fine

    ; If we got here the new position was problematic
    lea ax [game_lose_string]
    int 2 ; Print cstr
    end

new_position_fine:
    ; Actually load into head
    mov al [new_x]
    mov ah [new_y]
    mov [snake_entries + 0] al
    mov [snake_entries + 1] ah
    ; Add new head to screen
    push 3 ; Color green
    push ax ; X and Y
    call draw_cube

    ; Remove tail from screen
    mov al [tail_x]
    mov ah [tail_y]
    push 1 ; Color black
    push ax ; X and y
    call draw_cube

    ; Check if reached apple
    mov al [snake_entries + 0]
    cmp al [apple_x]
    jne not_reached_apple
    mov al [snake_entries + 1]
    cmp al [apple_y]
    jne not_reached_apple

    ; If we got here we reached apple
    mov al [tail_x]
    mov ah [tail_y]
    push ax
    call snake_add_entry
    call generate_apple

    ; Say how many snake cubes we have
    mov ah 0
    mov al [amount_snake_entries]
    int 0
    lea ax [string_points] ; Print suffix
    int 2

not_reached_apple:
    ; Update screen
    int 5

    pop cx
    pop bx
    pop ax
    add sp 4
    pop bp
    ret
endp move_snake

; Generate an apple in a valid place on the screen
proc generate_apple
    push ax
    push bx
    push dx

keep_generating:
    ; Generate new apple coords
    int 8 ; Get random
    mov dx 0
    mov bx CUBES_HEIGHT
    div bx
    mov [apple_y] dl
    int 8 ; Get random
    mov dx 0
    mov bx CUBES_WIDTH
    div bx
    mov [apple_x] dl

    ; Generate again if we found the position inside the snake
    mov al [apple_x]
    mov ah [apple_y]
    push ax
    call is_position_inside_snake
    cmp ax 1
    je keep_generating

    mov al [apple_x]
    mov ah [apple_y]
    push 4
    push ax
    call draw_cube

    pop dx
    pop bx
    pop ax
    ret
endp generate_apple
