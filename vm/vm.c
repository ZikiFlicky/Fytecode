#include "fy.h"

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
}

void Fy_VM_Destruct(Fy_VM *vm) {
    free(vm->mem_space_bottom);
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
    return vm->mem_space_bottom[address];
}

uint16_t Fy_VM_getMem16(Fy_VM *vm, uint16_t address) {
    // assert(address <= vm->mem_size - 2);
    // Little endian
    return vm->mem_space_bottom[address] + (vm->mem_space_bottom[address + 1] << 8);
}

void Fy_VM_setMem16(Fy_VM *vm, uint16_t address, uint16_t value) {
    *(uint16_t*)&vm->mem_space_bottom[address] = value;
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
        div_reg_ptr[0] = value & 0xFF;
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

static void Fy_VM_runInstruction(Fy_VM *vm) {
    uint8_t opcode = Fy_VM_getMem8(vm, vm->reg_ip);
    for (size_t i = 0; i < sizeof(Fy_instructionTypes) / sizeof(Fy_InstructionType*); ++i) {
        Fy_InstructionType *type = Fy_instructionTypes[i];
        if (opcode == type->opcode) {
            uint16_t address = vm->reg_ip + 1;
            if (!type->variable_size)
                vm->reg_ip += 1 + type->additional_size;
            if (type->run_func)
                type->run_func(vm, address);
            return;
        }
    }
    // If we got here, we didn't match any opcode and this is an invalid instruction
    Fy_VM_runtimeError(vm, Fy_RuntimeError_InvalidOpcode, "'%.2x'", opcode);
}

/* Returns exit code */
int Fy_VM_runAll(Fy_VM *vm) {
    while (vm->running) {
        Fy_VM_runInstruction(vm);
    }
    if (vm->error)
        return 1;
    else
        return 0;
}

void Fy_VM_setResult16InFlags(Fy_VM *vm, int16_t res) {
    if (res == 0)
        vm->flags |= FY_FLAGS_ZERO; // Enable
    else
        vm->flags &= ~FY_FLAGS_ZERO; // Disable

    if (res < 0)
        vm->flags |= FY_FLAGS_SIGN; // Enable
    else
        vm->flags &= ~FY_FLAGS_SIGN; // Disable
}

void Fy_VM_setResult8InFlags(Fy_VM *vm, int8_t res) {
    if (res == 0)
        vm->flags |= FY_FLAGS_ZERO; // Enable
    else
        vm->flags &= ~FY_FLAGS_ZERO; // Disable

    if (res < 0)
        vm->flags |= FY_FLAGS_SIGN; // Enable
    else
        vm->flags &= ~FY_FLAGS_SIGN; // Disable
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
