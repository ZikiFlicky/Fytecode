#include "fy.h"

static void Fy_interruptPutNumber_run(Fy_VM *vm);
static void Fy_interruptPutChar_run(Fy_VM *vm);
static void Fy_interruptPutString_run(Fy_VM *vm);
static void Fy_interruptOpenWindow_run(Fy_VM *vm);
static void Fy_interruptSetPixel_run(Fy_VM *vm);
static void Fy_interruptUpdate_run(Fy_VM *vm);

Fy_InterruptDef Fy_interruptPutNumber = {
    .opcode = 0,
    .func = Fy_interruptPutNumber_run
};
Fy_InterruptDef Fy_interruptPutChar = {
    .opcode = 1,
    .func = Fy_interruptPutChar_run
};
Fy_InterruptDef Fy_interruptPutString = {
    .opcode = 2,
    .func = Fy_interruptPutString_run
};
Fy_InterruptDef Fy_interruptOpenWindow = {
    .opcode = 3,
    .func = Fy_interruptOpenWindow_run
};
Fy_InterruptDef Fy_interruptSetPixel = {
    .opcode = 4,
    .func = Fy_interruptSetPixel_run
};
Fy_InterruptDef Fy_interruptUpdate = {
    .opcode = 5,
    .func = Fy_interruptUpdate_run
};

Fy_InterruptDef *Fy_interruptDefs[] = {
    &Fy_interruptPutNumber,
    &Fy_interruptPutChar,
    &Fy_interruptPutString,
    &Fy_interruptOpenWindow,
    &Fy_interruptSetPixel,
    &Fy_interruptUpdate
};


SDL_Color Fy_screenPalette[] = {
    { 0xff, 0xff, 0xff, 0xff },
    { 0, 0, 0, 0xff },
    { 0xff, 0, 0, 0xff },
    { 0, 0xff, 0, 0xff },
    { 0, 0, 0xff, 0xff },
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

static void Fy_interruptUpdate_run(Fy_VM *vm) {
    SDL_UpdateWindowSurface(vm->window);
}

void Fy_InterruptDef_run(Fy_InterruptDef *def, Fy_VM *vm) {
    assert(def->func);
    def->func(vm);
}

Fy_InterruptDef *Fy_findInterruptDefByOpcode(uint8_t opcode) {
    for (size_t i = 0; i < sizeof(Fy_interruptDefs) / sizeof(Fy_InterruptDef*); ++i) {
        Fy_InterruptDef *possible_interrupt = Fy_interruptDefs[i];
        if (opcode == possible_interrupt->opcode)
            return possible_interrupt;
    }
    return NULL;
}
