#ifndef FY_DEFINES_H
#define FY_DEFINES_H

typedef enum Fy_Reg16 Fy_Reg16;
typedef enum Fy_Reg8 Fy_Reg8;

enum Fy_Reg16 {
    Fy_Reg16_Ax,
    Fy_Reg16_Bx,
    Fy_Reg16_Cx,
    Fy_Reg16_Dx,
    Fy_Reg16_Sp,
    Fy_Reg16_Bp
};

enum Fy_Reg8 {
    Fy_Reg8_Ah,
    Fy_Reg8_Al,
    Fy_Reg8_Bh,
    Fy_Reg8_Bl,
    Fy_Reg8_Ch,
    Fy_Reg8_Cl,
    Fy_Reg8_Dh,
    Fy_Reg8_Dl
};

#endif /* FY_DEFINES_H */
