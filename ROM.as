; =========================================
; MINIMAL ROM (Mapped at ROM address e.g., 0x1000)
; =========================================
_start:
    ; 1. Initialize Stack Pointer (SP / R15) to top word of RAM
    MOVI R15, 0x03FF    ; Point SP to word 1023 (last 4-byte word in 4 KiB RAM)

    ; 2. Clear general-purpose registers (R1 - R14)
    ; (Leaving R0 as constant zero or clear it too if needed)
    MOVI R1,  0x0000
    MOVI R2,  0x0000
    MOVI R3,  0x0000
    MOVI R4,  0x0000
    MOVI R5,  0x0000
    MOVI R6,  0x0000
    MOVI R7,  0x0000
    MOVI R8,  0x0000
    MOVI R9,  0x0000
    MOVI R10, 0x0000
    MOVI R11, 0x0000
    MOVI R12, 0x0000
    MOVI R13, 0x0000
    MOVI R14, 0x0000

    ; --------------------------------------------------
    ; [Future Stage 2: DMA / OS Loader Code Goes Here]
    ; --------------------------------------------------

    ; 3. Jump to start of RAM
    JMPA  0x0000         ; PC jumps to RAM Word 0 / Page 0