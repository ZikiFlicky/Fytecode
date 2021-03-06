#include "fy.h"

/* Declare functions */
static void Fy_VM_setResult16InFlags(Fy_VM *vm, int16_t res);
static void Fy_VM_setResult8InFlags(Fy_VM *vm, int8_t res);
static uint16_t Fy_VM_add16(Fy_VM *vm, uint16_t lhs, uint16_t rhs);
static uint16_t Fy_VM_sub16(Fy_VM *vm, uint16_t lhs, uint16_t rhs);
static uint8_t Fy_VM_add8(Fy_VM *vm, uint8_t lhs, uint8_t rhs);
static uint8_t Fy_VM_sub8(Fy_VM *vm, uint8_t lhs, uint8_t rhs);

/*
 * Reads all of the binary file into `out`.
 * Returns false on failure.
 */
bool Fy_OpenBytecodeFile(char *filename, Fy_BytecodeFileStream *out) {
    FILE *file;
    uint16_t length;
    uint8_t *stream;

    file = fopen(filename, "r");
    if (!file)
        return false;

    fseek(file, 0, SEEK_END);
    length = ftell(file);
    fseek(file, 0, SEEK_SET);

    stream = malloc(length * sizeof(uint8_t));
    fread(stream, sizeof(uint8_t), length, file);
    fclose(file);

    out->code = stream;
    out->length = length;
    out->idx = 0;

    return true;
}

void Fy_BytecodeFileStream_Destruct(Fy_BytecodeFileStream *bc) {
    if (bc->length > 0)
        free(bc->code);
}

static uint16_t Fy_BytecodeFileStream_readByte(Fy_BytecodeFileStream *bc) {
    uint8_t b;
    if (bc->idx + 1 > bc->length)
        FY_UNREACHABLE(); // FIXME: Error
    b = bc->code[bc->idx];
    ++bc->idx;
    return b;
}

static uint16_t Fy_BytecodeFileStream_readWord(Fy_BytecodeFileStream *bc) {
    uint16_t w;
    if (bc->idx + 2 > bc->length)
        FY_UNREACHABLE(); // FIXME: Error
    w = bc->code[bc->idx] + (bc->code[bc->idx + 1] << 8);
    bc->idx += 2;
    return w;
}

static void Fy_BytecodeFileStream_readBytesInto(Fy_BytecodeFileStream *bc, uint16_t amount, uint8_t *out, bool advance) {
    if (bc->idx + amount > bc->length)
        FY_UNREACHABLE(); // FIXME: Error
    for (uint16_t i = 0; i < amount; ++i)
        out[i] = bc->code[bc->idx + i];
    if (advance)
        bc->idx += amount;
}

static void Fy_BytecodeFileStream_readShebang(Fy_BytecodeFileStream *bc) {
    uint8_t shebang_start[2];
    Fy_BytecodeFileStream_readBytesInto(bc, 2, shebang_start, false);
    if (shebang_start[0] == '#' && shebang_start[1] == '!') {
        uint8_t c;
        bc->idx += 2;
        do {
            c = Fy_BytecodeFileStream_readByte(bc);
        } while (c != '\n');
    }
}

static void Fy_BytecodeFileStream_readNameHeader(Fy_BytecodeFileStream *bc) {
    uint8_t name[2];
    Fy_BytecodeFileStream_readBytesInto(bc, 2, name, false);
    if (name[0] == 'F' && name[1] == 'Y') {
        bc->idx += 2;
    } else {
        FY_UNREACHABLE();
    }
}

static void Fy_BytecodeFileStream_writeBytesInto(Fy_BytecodeFileStream *bc, uint16_t amount, uint8_t *out) {
    if (bc->idx + amount > bc->length) {
        FY_UNREACHABLE();
    }
    memcpy(out, &bc->code[bc->idx], amount);
    bc->idx += amount;
}

static char *Fy_RuntimeError_toString(Fy_RuntimeError error) {
    switch (error) {
    case Fy_RuntimeError_InvalidOpcode:
        return "Invalid opcode";
    case Fy_RuntimeError_ReadableReg16NotFound:
        return "Could not find readable 16-bit register from opcode";
    case Fy_RuntimeError_WritableReg16NotFound:
        return "Could not find writable 16-bit register from opcode";
    case Fy_RuntimeError_ReadableReg8NotFound:
        return "Could not find readable 8-bit register from opcode";
    case Fy_RuntimeError_WritableReg8NotFound:
        return "Could not find writable 8-bit register from opcode";
    case Fy_RuntimeError_InterruptNotFound:
        return "Interrupt not found";
    case Fy_RuntimeError_InterruptError:
        return "Interrupt error";
    case Fy_RuntimeError_DivisionByZero:
        return "Division by zero";
    case Fy_RuntimeError_DivisionResultTooBig:
        return "Division result too big";
    default:
        FY_UNREACHABLE();
    }
}

/* Align word to be bigger by a or more and dividable by a */
static uint16_t Fy_alignWord(uint16_t w, uint16_t a) {
    if (w % a == 0)
        return w + a;
    else
        return w + (a - w % a) + a;
}

void Fy_VM_Init(Fy_BytecodeFileStream *bc, Fy_VM *out) {
    uint16_t data_size, code_size, stack_size;
    uint16_t data_offset, code_offset, stack_offset;

    out->mem_space_bottom = malloc((1 << 16) * sizeof(uint8_t));
    Fy_BytecodeFileStream_readShebang(bc);
    Fy_BytecodeFileStream_readNameHeader(bc);
    // Parse header
    data_size = Fy_BytecodeFileStream_readWord(bc);
    code_size = Fy_BytecodeFileStream_readWord(bc);
    stack_size = Fy_BytecodeFileStream_readWord(bc);
    data_offset = 0;
    code_offset = Fy_alignWord(data_offset + data_size, 0x100);
    stack_offset = Fy_alignWord(code_offset + code_size + stack_size, 0x100);
    Fy_BytecodeFileStream_writeBytesInto(bc, data_size, &out->mem_space_bottom[data_offset]);
    Fy_BytecodeFileStream_writeBytesInto(bc, code_size, &out->mem_space_bottom[code_offset]);

    out->data_offset = data_offset;
    out->code_offset = code_offset;
    out->stack_offset = stack_offset;
    out->stack_size = stack_size; // In bytes
    out->reg_ax[0] = out->reg_ax[1] = 0;
    out->reg_bx[0] = out->reg_bx[1] = 0;
    out->reg_cx[0] = out->reg_cx[1] = 0;
    out->reg_dx[0] = out->reg_dx[1] = 0;
    out->reg_ip = code_offset;
    out->reg_sp = stack_offset;
    out->reg_bp = 0;
    out->running = true;
    out->error = false;
    out->flags = 0;
    out->window = NULL;
    out->surface = NULL;

    Fy_Time_Init(&out->start_time);
}

void Fy_VM_Destruct(Fy_VM *vm) {
    if (vm->window) {
        SDL_FreeSurface(vm->surface);
        SDL_DestroyWindow(vm->window);
        SDL_Quit();
    }
    free(vm->mem_space_bottom);
}

uint16_t Fy_VM_generateRandom(Fy_VM *vm) {
    uint16_t n = vm->random_seed;
    srand(n);
    vm->random_seed = rand();
    return n;
}

void Fy_VM_runtimeError(Fy_VM *vm, Fy_RuntimeError err, char *additional, ...) {
    (void)vm;
    printf("RuntimeError: %s", Fy_RuntimeError_toString(err));
    if (additional) {
        va_list va;
        printf(": ");
        va_start(va, additional);
        vprintf(additional, va);
        va_end(va);
    }
    printf("\n");
    vm->running = false;
    vm->error = true;
}

uint8_t Fy_VM_getMem8(Fy_VM *vm, uint16_t address) {
    // assert(address <= vm->mem_size - 1);
    return ((uint8_t*)vm->mem_space_bottom)[address];
}

uint16_t Fy_VM_getMem16(Fy_VM *vm, uint16_t address) {
    // assert(address <= vm->mem_size - 2);
    // Little endian
    return ((uint16_t)vm->mem_space_bottom[address]) + ((uint16_t)vm->mem_space_bottom[address + 1] << 8);
}

void Fy_VM_setMem16(Fy_VM *vm, uint16_t address, uint16_t value) {
    vm->mem_space_bottom[address] = (uint8_t)(value & 0xff);
    vm->mem_space_bottom[address + 1] = (uint8_t)(value >> 8);
}

void Fy_VM_setMem8(Fy_VM *vm, uint16_t address, uint8_t value) {
    vm->mem_space_bottom[address] = value;
}

static uint8_t *Fy_VM_getDividedReg16Ptr(Fy_VM *vm, uint8_t reg) {
    uint8_t *reg_ptr;

    switch (reg) {
    case Fy_Reg16_Ax:
        reg_ptr = vm->reg_ax;
        break;
    case Fy_Reg16_Bx:
        reg_ptr = vm->reg_bx;
        break;
    case Fy_Reg16_Cx:
        reg_ptr = vm->reg_cx;
        break;
    case Fy_Reg16_Dx:
        reg_ptr = vm->reg_dx;
        break;
    default:
        reg_ptr = NULL;
    }

    return reg_ptr;
}

static uint16_t *Fy_VM_getReg16Ptr(Fy_VM *vm, uint8_t reg) {
    uint16_t *reg_ptr;
    switch (reg) {
    case Fy_Reg16_Sp:
        reg_ptr = &vm->reg_sp;
        break;
    case Fy_Reg16_Bp:
        reg_ptr = &vm->reg_bp;
        break;
    default:
        reg_ptr = NULL;
    }
    return reg_ptr;
}

bool Fy_VM_getReg16(Fy_VM *vm, uint8_t reg, uint16_t *out) {
    uint8_t *div_reg_ptr;
    uint16_t *reg_ptr;
    uint16_t value;

    if ((div_reg_ptr = Fy_VM_getDividedReg16Ptr(vm, reg))) {
        value = div_reg_ptr[0] + (div_reg_ptr[1] << 8); // Little-endian
    } else if ((reg_ptr = Fy_VM_getReg16Ptr(vm, reg))) {
        value = *reg_ptr;
    } else {
        Fy_VM_runtimeError(vm, Fy_RuntimeError_ReadableReg16NotFound, "'%X'", reg);
        return false;
    }

    *out = value;
    return true;
}

bool Fy_VM_setReg16(Fy_VM *vm, uint8_t reg, uint16_t value) {
    uint8_t *div_reg_ptr;
    uint16_t *reg_ptr;

    if ((div_reg_ptr = Fy_VM_getDividedReg16Ptr(vm, reg))) {
        div_reg_ptr[0] = value & 0xff;
        div_reg_ptr[1] = value >> 8;
    } else if ((reg_ptr = Fy_VM_getReg16Ptr(vm, reg))) {
        *reg_ptr = value;
    } else {
        Fy_VM_runtimeError(vm, Fy_RuntimeError_WritableReg16NotFound, "'%X'", reg);
        return false;
    }

    // Set flags matching the operation
    Fy_VM_setResult16InFlags(vm, value);
    return true;
}

static uint8_t *Fy_VM_getReg8Ptr(Fy_VM *vm, uint8_t reg) {
    switch (reg) {
    case Fy_Reg8_Ah:
        return &vm->reg_ax[1];
    case Fy_Reg8_Al:
        return &vm->reg_ax[0];
    case Fy_Reg8_Bh:
        return &vm->reg_bx[1];
    case Fy_Reg8_Bl:
        return &vm->reg_bx[0];
    case Fy_Reg8_Ch:
        return &vm->reg_cx[1];
    case Fy_Reg8_Cl:
        return &vm->reg_cx[0];
    case Fy_Reg8_Dh:
        return &vm->reg_dx[1];
    case Fy_Reg8_Dl:
        return &vm->reg_dx[0];
    default:
        Fy_VM_runtimeError(vm, Fy_RuntimeError_ReadableReg8NotFound, "%d", reg);
        return NULL;
    }
}

bool Fy_VM_getReg8(Fy_VM *vm, uint8_t reg, uint8_t *out) {
    uint8_t *reg_ptr = Fy_VM_getReg8Ptr(vm, reg);
    if (!reg_ptr)
        return false;

    *out = *reg_ptr;
    return true;
}

bool Fy_VM_setReg8(Fy_VM *vm, uint8_t reg, uint8_t value) {
    uint8_t *reg_ptr = Fy_VM_getReg8Ptr(vm, reg);
    if (!reg_ptr)
        return false;

    *reg_ptr = value;
    // Set flags matching the operation
    Fy_VM_setResult8InFlags(vm, value);
    return true;
}

bool Fy_VM_isWritableReg16(Fy_VM *vm, uint8_t reg) {
    if (Fy_VM_getReg16Ptr(vm, reg))
        return true;
    if (Fy_VM_getDividedReg16Ptr(vm, reg))
        return true;
    return false;
}

bool Fy_VM_isWritableReg8(Fy_VM *vm, uint8_t reg) {
    if (Fy_VM_getReg8Ptr(vm, reg))
        return true;
    return false;
}

bool Fy_VM_runBinaryOperatorOnReg16(Fy_VM *vm, Fy_BinaryOperator operator, uint8_t reg_id, uint16_t value) {
    uint16_t reg_value;
    bool set_in_flags = true;
    bool set_in_reg = true;

    if (!Fy_VM_getReg16(vm, reg_id, &reg_value))
        return false;

    switch (operator) {
    case Fy_BinaryOperator_Mov:
        reg_value = value;
        set_in_flags = false;
        break;
    case Fy_BinaryOperator_Add:
        reg_value = Fy_VM_add16(vm, reg_value, value);
        break;
    case Fy_BinaryOperator_Sub:
        reg_value = Fy_VM_sub16(vm, reg_value, value);
        break;
    case Fy_BinaryOperator_And:
        reg_value &= value;
        break;
    case Fy_BinaryOperator_Or:
        reg_value |= value;
        break;
    case Fy_BinaryOperator_Xor:
        reg_value ^= value;
        break;
    case Fy_BinaryOperator_Shl:
        reg_value <<= value;
        break;
    case Fy_BinaryOperator_Shr:
        reg_value >>= value;
        break;
    case Fy_BinaryOperator_Cmp:
        reg_value = Fy_VM_sub16(vm, reg_value, value);
        set_in_reg = false;
        break;
    default:
        Fy_VM_runtimeError(vm, Fy_RuntimeError_InvalidOpcode, "Invalid operator opcode '%d'", operator);
        return false;
    }

    // Set result in flags unless explicitly not allowed
    if (set_in_flags)
        Fy_VM_setResult16InFlags(vm, reg_value);
    // Set result in the register unless explicitly not allowed
    if (set_in_reg) {
        if (!Fy_VM_setReg16(vm, reg_id, reg_value))
            return false;
    }
    return true;
}

bool Fy_VM_runBinaryOperatorOnReg8(Fy_VM *vm, Fy_BinaryOperator operator, uint8_t reg_id, uint8_t value) {
    uint8_t reg_value;
    bool set_in_flags = true;
    bool set_in_reg = true;

    if (!Fy_VM_getReg8(vm, reg_id, &reg_value))
        return false;

    switch (operator) {
    case Fy_BinaryOperator_Mov:
        reg_value = value;
        set_in_flags = false;
        break;
    case Fy_BinaryOperator_Add:
        reg_value = Fy_VM_add8(vm, reg_value, value);
        break;
    case Fy_BinaryOperator_Sub:
        reg_value = Fy_VM_sub8(vm, reg_value, value);
        break;
    case Fy_BinaryOperator_And:
        reg_value &= value;
        break;
    case Fy_BinaryOperator_Or:
        reg_value |= value;
        break;
    case Fy_BinaryOperator_Xor:
        reg_value ^= value;
        break;
    case Fy_BinaryOperator_Shl:
        reg_value <<= value;
        break;
    case Fy_BinaryOperator_Shr:
        reg_value >>= value;
        break;
    case Fy_BinaryOperator_Cmp:
        // Just for the flags
        reg_value = Fy_VM_sub8(vm, reg_value, value);
        set_in_reg = false;
        break;
    default:
        Fy_VM_runtimeError(vm, Fy_RuntimeError_InvalidOpcode, "Invalid operator opcode '%d'", operator);
        return false;
    }

    // Set result in flags unless explicitly not allowed
    if (set_in_flags)
        Fy_VM_setResult8InFlags(vm, reg_value);
    // Set result in the register unless explicitly not allowed
    if (set_in_reg) {
        if (!Fy_VM_setReg8(vm, reg_id, reg_value))
            return false;
    }
    return true;
}

bool Fy_VM_runBinaryOperatorOnMem16(Fy_VM *vm, Fy_BinaryOperator operator, uint16_t address, uint16_t value) {
    uint16_t mem_value;
    bool set_in_flags = true;
    bool set_in_mem = true;

    mem_value = Fy_VM_getMem16(vm, address);

    switch (operator) {
    case Fy_BinaryOperator_Mov:
        mem_value = value;
        set_in_flags = false;
        break;
    case Fy_BinaryOperator_Add:
        mem_value = Fy_VM_add16(vm, mem_value, value);
        break;
    case Fy_BinaryOperator_Sub:
        mem_value = Fy_VM_sub16(vm, mem_value, value);
        break;
    case Fy_BinaryOperator_And:
        mem_value &= value;
        break;
    case Fy_BinaryOperator_Or:
        mem_value |= value;
        break;
    case Fy_BinaryOperator_Xor:
        mem_value ^= value;
        break;
    case Fy_BinaryOperator_Shl:
        mem_value <<= value;
        break;
    case Fy_BinaryOperator_Shr:
        mem_value >>= value;
        break;
    case Fy_BinaryOperator_Cmp:
        // Just for the flags
        mem_value = Fy_VM_sub16(vm, mem_value, value);
        set_in_mem = false;
        break;
    default:
        Fy_VM_runtimeError(vm, Fy_RuntimeError_InvalidOpcode, "Invalid operator opcode '%d'", operator);
        return false;
    }

    // Set result in flags unless explicitly not allowed
    if (set_in_flags)
        Fy_VM_setResult16InFlags(vm, mem_value);
    // Set result in memory unless explicitly not allowed
    if (set_in_mem)
        Fy_VM_setMem16(vm, address, mem_value);
    return true;
}

bool Fy_VM_runBinaryOperatorOnMem8(Fy_VM *vm, Fy_BinaryOperator operator, uint16_t address, uint8_t value) {
    uint8_t mem_value;
    bool set_in_flags = true;
    bool set_in_mem = true;

    mem_value = Fy_VM_getMem8(vm, address);

    switch (operator) {
    case Fy_BinaryOperator_Mov:
        mem_value = value;
        set_in_flags = false;
        break;
    case Fy_BinaryOperator_Add:
        mem_value = Fy_VM_add8(vm, mem_value, value);
        break;
    case Fy_BinaryOperator_Sub:
        mem_value = Fy_VM_sub8(vm, mem_value, value);
        break;
    case Fy_BinaryOperator_And:
        mem_value &= value;
        break;
    case Fy_BinaryOperator_Or:
        mem_value |= value;
        break;
    case Fy_BinaryOperator_Xor:
        mem_value ^= value;
        break;
    case Fy_BinaryOperator_Shl:
        mem_value <<= value;
        break;
    case Fy_BinaryOperator_Shr:
        mem_value >>= value;
        break;
    case Fy_BinaryOperator_Cmp:
        mem_value = Fy_VM_sub8(vm, mem_value, value);
        set_in_mem = false;
        break;
    default:
        Fy_VM_runtimeError(vm, Fy_RuntimeError_InvalidOpcode, "Invalid operator opcode '%d'", operator);
        return false;
    }

    // Set result in flags unless explicitly not allowed
    if (set_in_flags)
        Fy_VM_setResult8InFlags(vm, mem_value);
    // Set result in memory unless explicitly not allowed
    if (set_in_mem)
        Fy_VM_setMem8(vm, address, mem_value);
    return true;
}

static bool Fy_VM_runUnaryOperator16(Fy_VM *vm, Fy_UnaryOperator operator, uint16_t *value) {
    switch (operator) {
    case Fy_UnaryOperator_Neg:
        *value = ~(*value) + 1; // Do neg manually instead of casting
        break;
    case Fy_UnaryOperator_Inc:
        *value = Fy_VM_add16(vm, *value, 1);
        break;
    case Fy_UnaryOperator_Dec:
        *value = Fy_VM_sub16(vm, *value, 1);
        break;
    case Fy_UnaryOperator_Not:
        *value = ~(*value);
        break;
    default:
        Fy_VM_runtimeError(vm, Fy_RuntimeError_InvalidOpcode, "Invalid operator opcode '%d'", operator);
        return false;
    }

    Fy_VM_setResult16InFlags(vm, *value);
    return true;
}

static bool Fy_VM_runUnaryOperator8(Fy_VM *vm, Fy_UnaryOperator operator, uint8_t *value) {
    switch (operator) {
    case Fy_UnaryOperator_Neg:
        *value = ~(*value) + 1; // Do neg manually instead of casting
        break;
    case Fy_UnaryOperator_Inc:
        *value = Fy_VM_add8(vm, *value, 1);
        break;
    case Fy_UnaryOperator_Dec:
        *value = Fy_VM_sub8(vm, *value, 1);
        break;
    case Fy_UnaryOperator_Not:
        *value = ~(*value);
        break;
    default:
        Fy_VM_runtimeError(vm, Fy_RuntimeError_InvalidOpcode, "Invalid operator opcode '%d'", operator);
        return false;
    }

    Fy_VM_setResult8InFlags(vm, *value);
    return true;
}

bool Fy_VM_runUnaryOperatorOnReg16(Fy_VM *vm, Fy_UnaryOperator operator, uint8_t reg_id) {
    uint16_t reg_value;

    if (!Fy_VM_getReg16(vm, reg_id, &reg_value))
        return false;

    if (!Fy_VM_runUnaryOperator16(vm, operator, &reg_value))
        return false;

    if (!Fy_VM_setReg16(vm, reg_id, reg_value))
        return false;

    return true;
}

bool Fy_VM_runUnaryOperatorOnMem16(Fy_VM *vm, Fy_UnaryOperator operator, uint16_t address) {
    uint16_t mem_value;

    mem_value = Fy_VM_getMem16(vm, address);

    if (!Fy_VM_runUnaryOperator16(vm, operator, &mem_value))
        return false;

    Fy_VM_setMem16(vm, address, mem_value);
    return true;
}


bool Fy_VM_runUnaryOperatorOnReg8(Fy_VM *vm, Fy_UnaryOperator operator, uint8_t reg_id) {
    uint8_t reg_value;

    if (!Fy_VM_getReg8(vm, reg_id, &reg_value))
        return false;

    if (!Fy_VM_runUnaryOperator8(vm, operator, &reg_value))
        return false;

    if (!Fy_VM_setReg8(vm, reg_id, reg_value))
        return false;

    return true;
}

bool Fy_VM_runUnaryOperatorOnMem8(Fy_VM *vm, Fy_UnaryOperator operator, uint16_t address) {
    uint8_t mem_value;

    mem_value = Fy_VM_getMem8(vm, address);

    if (!Fy_VM_runUnaryOperator8(vm, operator, &mem_value))
        return false;

    Fy_VM_setMem8(vm, address, mem_value);
    return true;
}

static void Fy_VM_runInstruction(Fy_VM *vm) {
    uint8_t opcode = Fy_VM_getMem8(vm, vm->reg_ip);
    // If the opcode exists
    if (opcode < sizeof(Fy_instructionTypes) / sizeof(Fy_InstructionType*)) {
        const Fy_InstructionType *type = Fy_instructionTypes[opcode];
        uint16_t address = vm->reg_ip + 1;
        if (!type->variable_size)
            vm->reg_ip += 1 + type->additional_size;
        if (type->run_func)
            type->run_func(vm, address);
    } else {
        // Out of range
        Fy_VM_runtimeError(vm, Fy_RuntimeError_InvalidOpcode, "'%.2x'", opcode);
    }
}

static void Fy_VM_handleEvents(Fy_VM *vm) {
    // If we have a window check window events
    if (vm->window) {
        SDL_Event event;
        // Loop all SDL events
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
            case SDL_KEYDOWN:
                vm->keyboard.has_key = true;
                vm->keyboard.key_scancode = event.key.keysym.scancode;
                break;
            case SDL_WINDOWEVENT:
                switch (event.window.event) {
                case SDL_WINDOWEVENT_CLOSE:
                    // If we close the window we should exit the program
                    vm->running = false;
                    vm->error = true;
                    break;
                }
                break;
            }
        }
    }
    if (Fy_hadExitSignal) {
        vm->running = false;
        vm->error = true;
    }
}

/* Returns exit code */
int Fy_VM_runAll(Fy_VM *vm) {
    while (vm->running) {
        // Handle other events
        Fy_VM_handleEvents(vm);
        // Run the awaiting instructions
        Fy_VM_runInstruction(vm);
    }
    if (vm->error)
        return 1;
    else
        return 0;
}

static void Fy_VM_setResult16InFlags(Fy_VM *vm, int16_t res) {
    if (res == 0)
        vm->flags |= FY_FLAGS_ZERO; // Enable
    else
        vm->flags &= ~FY_FLAGS_ZERO; // Disable

    if (res < 0)
        vm->flags |= FY_FLAGS_SIGN; // Enable
    else
        vm->flags &= ~FY_FLAGS_SIGN; // Disable
}

static void Fy_VM_setResult8InFlags(Fy_VM *vm, int8_t res) {
    if (res == 0)
        vm->flags |= FY_FLAGS_ZERO; // Enable
    else
        vm->flags &= ~FY_FLAGS_ZERO; // Disable

    if (res < 0)
        vm->flags |= FY_FLAGS_SIGN; // Enable
    else
        vm->flags &= ~FY_FLAGS_SIGN; // Disable
}

static void Fy_VM_setOverflowFlag(Fy_VM *vm, int32_t no_error, int32_t maybe_error) {
    if (no_error != maybe_error)
        vm->flags |= FY_FLAGS_OVERFLOW;
    else
        vm->flags &= ~FY_FLAGS_OVERFLOW;
}

static void Fy_VM_setCarryFlag(Fy_VM *vm, uint32_t no_error, uint32_t maybe_error) {
    if (no_error != maybe_error)
        vm->flags |= FY_FLAGS_CARRY;
    else
        vm->flags &= ~FY_FLAGS_CARRY;
}

static uint16_t Fy_VM_add16(Fy_VM *vm, uint16_t lhs, uint16_t rhs) {
    uint16_t res = lhs + rhs;
    Fy_VM_setCarryFlag(vm, (uint32_t)lhs + (uint32_t)rhs, (uint32_t)res);
    Fy_VM_setOverflowFlag(vm, (int32_t)(*(int16_t*)&lhs + *(int16_t*)&rhs), (int32_t)(*(int16_t*)&lhs + *(int16_t*)&rhs));
    return res;
}

static uint16_t Fy_VM_sub16(Fy_VM *vm, uint16_t lhs, uint16_t rhs) {
    uint16_t res = lhs - rhs;
    Fy_VM_setCarryFlag(vm, (uint32_t)lhs - (uint32_t)rhs, (uint32_t)res);
    Fy_VM_setOverflowFlag(vm, (int32_t)(*(int16_t*)&lhs - *(int16_t*)&rhs), (int32_t)(*(int16_t*)&lhs - *(int16_t*)&rhs));
    return res;
}

static uint8_t Fy_VM_add8(Fy_VM *vm, uint8_t lhs, uint8_t rhs) {
    uint8_t res = lhs + rhs;
    Fy_VM_setCarryFlag(vm, (uint32_t)lhs + (uint32_t)rhs, (uint32_t)res);
    Fy_VM_setOverflowFlag(vm, (int32_t)(*(int8_t*)&lhs + *(int8_t*)&rhs), (int32_t)(*(int8_t*)&lhs + *(int8_t*)&rhs));
    return res;
}

static uint8_t Fy_VM_sub8(Fy_VM *vm, uint8_t lhs, uint8_t rhs) {
    uint8_t res = lhs - rhs;
    Fy_VM_setCarryFlag(vm, (uint32_t)lhs - (uint32_t)rhs, (uint32_t)res);
    Fy_VM_setOverflowFlag(vm, (int32_t)(*(int8_t*)&lhs - *(int8_t*)&rhs), (int32_t)(*(int8_t*)&lhs - *(int8_t*)&rhs));
    return res;
}

/* Set the ip register to the given address relative to the code's start point in memory */
void Fy_VM_setIpToRelAddress(Fy_VM *vm, uint16_t address) {
    vm->reg_ip = vm->code_offset + address;
}

void Fy_VM_pushToStack(Fy_VM *vm, uint16_t value) {
    // FIXME: Do some stack overflow error
    if (vm->stack_offset - vm->reg_sp >= vm->stack_size)
        FY_UNREACHABLE();

    vm->reg_sp -= 2;

    Fy_VM_setMem16(vm, vm->reg_sp, value);
}

uint16_t Fy_VM_popFromStack(Fy_VM *vm) {
    uint16_t value;

    // FIXME: Do some stack underflow error
    if (vm->reg_sp >= vm->stack_offset)
        FY_UNREACHABLE();

    value = Fy_VM_getMem16(vm, vm->reg_sp);

    vm->reg_sp += 2;

    return value;
}

uint16_t Fy_VM_calculateAddress(Fy_VM *vm, uint16_t *variable_off_ptr, uint16_t amount_bp, uint16_t amount_bx, uint16_t additional) {
    uint16_t address = 0;
    uint16_t bx_value;

    if (!Fy_VM_getReg16(vm, Fy_Reg16_Bx, &bx_value))
        FY_UNREACHABLE();
    address += variable_off_ptr ? *(int16_t*)variable_off_ptr + vm->data_offset : 0;
    address += *(int16_t*)&amount_bp * vm->reg_bp;
    address += *(int16_t*)&amount_bx * bx_value;
    address += *(int16_t*)&additional;
    return address;
}

/* Returns size of param and puts memory address into `out` */
uint16_t Fy_VM_readMemoryParam(Fy_VM *vm, uint16_t address, uint16_t *out) {
    uint8_t mapping = Fy_VM_getMem8(vm, address + 0);
    uint16_t variable = 0;
    uint16_t amount_bp = 0;
    uint16_t amount_bx = 0;
    uint16_t mem_addr = 0;
    uint16_t full_addr;
    uint16_t size = 1;

    if (mapping & FY_INLINEVAL_MAPPING_HASVAR) {
        variable = Fy_VM_getMem16(vm, address + size);
        size += 2;
    }
    if (mapping & FY_INLINEVAL_MAPPING_HASBP) {
        amount_bp = Fy_VM_getMem16(vm, address + size);
        size += 2;
    }
    if (mapping & FY_INLINEVAL_MAPPING_HASBX) {
        amount_bx = Fy_VM_getMem16(vm, address + size);
        size += 2;
    }
    if (mapping & FY_INLINEVAL_MAPPING_HASNUM) {
        mem_addr = Fy_VM_getMem16(vm, address + size);
        size += 2;
    }

    full_addr = Fy_VM_calculateAddress(vm, mapping & FY_INLINEVAL_MAPPING_HASVAR ? &variable : NULL, amount_bp, amount_bx, mem_addr);
    *out = full_addr;
    return size;
}
