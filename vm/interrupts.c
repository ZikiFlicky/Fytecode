#include "fy.h"

static void Fy_interruptPutNumber_run(Fy_VM *vm);
static void Fy_interruptPutChar_run(Fy_VM *vm);
static void Fy_interruptPutString_run(Fy_VM *vm);
static void Fy_interruptOpenWindow_run(Fy_VM *vm);
static void Fy_interruptSetPixel_run(Fy_VM *vm);
static void Fy_interruptUpdate_run(Fy_VM *vm);
static void Fy_interruptGetTime_run(Fy_VM *vm);
static void Fy_interruptGetKeyboardInput_run(Fy_VM *vm);
static void Fy_interruptGetRandom_run(Fy_VM *vm);

Fy_InterruptRunFunc Fy_interruptFuncs[] = {
    Fy_interruptPutNumber_run,
    Fy_interruptPutChar_run,
    Fy_interruptPutString_run,
    Fy_interruptOpenWindow_run,
    Fy_interruptSetPixel_run,
    Fy_interruptUpdate_run,
    Fy_interruptGetTime_run,
    Fy_interruptGetKeyboardInput_run,
    Fy_interruptGetRandom_run
};


SDL_Color Fy_screenPalette[] = {
    { 0xff, 0xff, 0xff, 0xff },
    { 0, 0, 0, 0xff },
    { 0xff, 0, 0, 0xff },
    { 0, 0xff, 0, 0xff },
    { 0, 0, 0xff, 0xff }
};

static void Fy_interruptPutNumber_run(Fy_VM *vm) {
    uint16_t number;
    Fy_VM_getReg16(vm, Fy_Reg16_Ax, &number);
    printf("%d", number);
}

static void Fy_interruptPutChar_run(Fy_VM *vm) {
    uint8_t c;
    Fy_VM_getReg8(vm, Fy_Reg8_Al, &c);
    printf("%c", c);
}

static void Fy_interruptPutString_run(Fy_VM *vm) {
    uint16_t addr;
    uint16_t i = 0;
    uint8_t c;
    Fy_VM_getReg16(vm, Fy_Reg16_Ax, &addr);
    do {
        c = Fy_VM_getMem8(vm, addr + i);
        putchar(c);
        ++i;
    } while (c != 0); // Until reached NUL
}

static void Fy_interruptOpenWindow_run(Fy_VM *vm) {
    uint8_t width;
    uint8_t height;
    SDL_Window *window;
    SDL_Surface *surface;

    if (vm->window) {
        Fy_VM_runtimeError(vm, Fy_RuntimeError_InterruptError, "Window reopened");
        return;
    }

    Fy_VM_getReg8(vm, Fy_Reg8_Ah, &width);
    Fy_VM_getReg8(vm, Fy_Reg8_Al, &height);

    SDL_Init(SDL_INIT_VIDEO);
    window = SDL_CreateWindow("Fytecode window", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, (int)width, (int)height, SDL_WINDOW_SHOWN);
    if (!window) {
        Fy_VM_runtimeError(vm, Fy_RuntimeError_InterruptError, "Window could not be opened");
        return;
    }

    surface = SDL_GetWindowSurface(window);
    vm->window = window;
    vm->surface = surface;
}

static void Fy_interruptSetPixel_run(Fy_VM *vm) {
    uint8_t x;
    uint8_t y;
    uint8_t color_id;
    SDL_Color color;
    uint8_t *base;

    Fy_VM_getReg8(vm, Fy_Reg8_Ah, &x);
    Fy_VM_getReg8(vm, Fy_Reg8_Al, &y);
    Fy_VM_getReg8(vm, Fy_Reg8_Bl, &color_id);

    if (color_id > sizeof(Fy_screenPalette) / sizeof(SDL_Color)) {
        Fy_VM_runtimeError(vm, Fy_RuntimeError_InterruptError, "color '%d' invalid", color_id);
        return;
    }

    color = Fy_screenPalette[color_id];
    base = &((uint8_t*)vm->surface->pixels)[vm->surface->pitch * y + x * vm->surface->format->BytesPerPixel];
    base[0] = color.r;
    base[1] = color.g;
    base[2] = color.b;
    base[3] = color.a;
}

static void Fy_interruptGetTime_run(Fy_VM *vm) {
    Fy_Time diff;
    Fy_Time_getTimeSince(&vm->start_time, &diff);

    Fy_VM_setReg16(vm, Fy_Reg16_Ax, diff.seconds);
    Fy_VM_setReg16(vm, Fy_Reg16_Bx, diff.milliseconds);
}

static void Fy_interruptGetKeyboardInput_run(Fy_VM *vm) {
    Fy_VM_setReg16(vm, Fy_Reg16_Ax, vm->keyboard.has_key ? 1 : 0);
    if (vm->keyboard.has_key) {
        Fy_VM_setReg16(vm, Fy_Reg16_Bx, vm->keyboard.key_scancode);
        vm->keyboard.has_key = false;
    }
}

static void Fy_interruptGetRandom_run(Fy_VM *vm) {
    Fy_VM_setReg16(vm, Fy_Reg16_Ax, Fy_VM_generateRandom(vm));
}

static void Fy_interruptUpdate_run(Fy_VM *vm) {
    SDL_UpdateWindowSurface(vm->window);
}

Fy_InterruptRunFunc Fy_findInterruptFuncByOpcode(uint8_t opcode) {
    if (opcode < sizeof(Fy_interruptFuncs) / sizeof(Fy_InterruptRunFunc))
        return Fy_interruptFuncs[opcode];
    else
        return NULL;
}
