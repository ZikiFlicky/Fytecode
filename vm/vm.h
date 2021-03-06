#ifndef FY_VM_H
#define FY_VM_H

#include <inttypes.h>
#include <stdbool.h>

#include <SDL2/SDL.h>

#include "timecontrol.h"

#define FY_FLAGS_ZERO (1 << 0)
#define FY_FLAGS_SIGN (1 << 1)
#define FY_FLAGS_OVERFLOW (1 << 2)
#define FY_FLAGS_CARRY (1 << 3)

typedef struct Fy_VM Fy_VM;
typedef enum Fy_RuntimeError Fy_RuntimeError;
typedef struct Fy_BytecodeFileStream Fy_BytecodeFileStream;

struct Fy_BytecodeFileStream {
    uint8_t *code;
    uint16_t length, idx;
};

enum Fy_RuntimeError {
    Fy_RuntimeError_InvalidOpcode = 1,
    Fy_RuntimeError_ReadableReg16NotFound,
    Fy_RuntimeError_WritableReg16NotFound,
    Fy_RuntimeError_ReadableReg8NotFound,
    Fy_RuntimeError_WritableReg8NotFound,
    Fy_RuntimeError_InterruptNotFound,
    Fy_RuntimeError_InterruptError,
    Fy_RuntimeError_PixelNotInScreen,
    Fy_RuntimeError_DivisionByZero,
    Fy_RuntimeError_DivisionResultTooBig
};

struct Fy_VM {
    /* Pointer to bottom of allocated memory space */
    uint8_t *mem_space_bottom;
    /* Address in which data starts */
    uint16_t data_offset;
    /* Address in which code starts */
    uint16_t code_offset;
    /* Address in which stack starts */
    uint16_t stack_offset;
    /* Size of stack (in bytes) */
    uint16_t stack_size;
    /* Registers */
    uint8_t reg_ax[2];
    uint8_t reg_bx[2];
    uint8_t reg_cx[2];
    uint8_t reg_dx[2];
    uint16_t reg_ip;
    uint16_t reg_sp;
    uint16_t reg_bp;
    /* Is running? */
    bool running;
    /* Is there an error? combined with `running` */
    bool error;
    uint8_t flags;
    /* Start time */
    Fy_Time start_time;
    /* Keyboard related stuff */
    struct {
        bool has_key;
        SDL_Scancode key_scancode;
    } keyboard;
    /* Random related stuff */
    uint16_t random_seed;

    /* Graphics-related */
    SDL_Window *window;
    SDL_Surface *surface;
};

bool Fy_OpenBytecodeFile(char *filename, Fy_BytecodeFileStream *out);
void Fy_BytecodeFileStream_Destruct(Fy_BytecodeFileStream *bc);

void Fy_VM_Init(Fy_BytecodeFileStream *bc, Fy_VM *out);
void Fy_VM_Destruct(Fy_VM *vm);
uint16_t Fy_VM_generateRandom(Fy_VM *vm);
uint8_t Fy_VM_getMem8(Fy_VM *vm, uint16_t address);
uint16_t Fy_VM_getMem16(Fy_VM *vm, uint16_t address);
void Fy_VM_setMem8(Fy_VM *vm, uint16_t address, uint8_t value);
void Fy_VM_setMem16(Fy_VM *vm, uint16_t address, uint16_t value);
bool Fy_VM_getReg16(Fy_VM *vm, uint8_t reg, uint16_t *out);
bool Fy_VM_setReg16(Fy_VM *vm, uint8_t reg, uint16_t value);
bool Fy_VM_getReg8(Fy_VM *vm, uint8_t reg, uint8_t *out);
bool Fy_VM_setReg8(Fy_VM *vm, uint8_t reg, uint8_t value);
bool Fy_VM_isWritableReg16(Fy_VM *vm, uint8_t reg);
bool Fy_VM_isWritableReg8(Fy_VM *vm, uint8_t reg);
bool Fy_VM_runBinaryOperatorOnReg16(Fy_VM *vm, Fy_BinaryOperator operator, uint8_t reg_id, uint16_t value);
bool Fy_VM_runBinaryOperatorOnReg8(Fy_VM *vm, Fy_BinaryOperator operator, uint8_t reg_id, uint8_t value);
bool Fy_VM_runBinaryOperatorOnMem16(Fy_VM *vm, Fy_BinaryOperator operator, uint16_t address, uint16_t value);
bool Fy_VM_runBinaryOperatorOnMem8(Fy_VM *vm, Fy_BinaryOperator operator, uint16_t address, uint8_t value);
bool Fy_VM_runUnaryOperatorOnReg16(Fy_VM *vm, Fy_UnaryOperator operator, uint8_t reg_id);
bool Fy_VM_runUnaryOperatorOnMem16(Fy_VM *vm, Fy_UnaryOperator operator, uint16_t address);
bool Fy_VM_runUnaryOperatorOnReg8(Fy_VM *vm, Fy_UnaryOperator operator, uint8_t reg_id);
bool Fy_VM_runUnaryOperatorOnMem8(Fy_VM *vm, Fy_UnaryOperator operator, uint16_t address);
void Fy_VM_runtimeError(Fy_VM *vm, Fy_RuntimeError err, char *additional, ...);
int Fy_VM_runAll(Fy_VM *vm);
void Fy_VM_setIpToRelAddress(Fy_VM *vm, uint16_t address);
void Fy_VM_pushToStack(Fy_VM *vm, uint16_t value);
uint16_t Fy_VM_popFromStack(Fy_VM *vm);
uint16_t Fy_VM_calculateAddress(Fy_VM *vm, uint16_t *variable_off_ptr, uint16_t amount_bp, uint16_t amount_bx, uint16_t additional);
uint16_t Fy_VM_readMemoryParam(Fy_VM *vm, uint16_t address, uint16_t *out);

#endif /* FY_VM_H */
