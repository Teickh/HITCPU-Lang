# ==============================================================================
# HITCPU Program: Basic Fibonacci Sequence to Memory & Frame Buffer
# ==============================================================================

# 1. Initialization
MOVI    R1, 0           # R1 = Current Fib (Fib N-2)
MOVI    R2, 1           # R2 = Next Fib    (Fib N-1)
MOVI    R3, 10          # R3 = Loop Counter (10 iterations)
MOVI    R4, LOOP_MEMORY # R4 = Data Memory Pointer
MOVI    R5, 2048        # R5 = Frame Buffer Address (2048)
MOVI    R6, FONT_TABLE  # R6 = Base Address of FONT_TABLE

# 2. Main Loop
FIB_LOOP:
    # --- Store Current Fib Value to Memory ---
    STRI    R1, R4, 0   # Memory[R4] = R1
    ADDI    R4, R4, 2   # Advance memory pointer (+1 word)

    # --- Font Lookup (Using ADD + LDRI) ---
    LDR     R8, R1, R6   # R8 = Load glyph bitmap = FONT_TABLE + Current Fib Digit (R1)
    NOP
    NOP

    # --- Frame Buffer Write ---
    # STRI    R9, R5, 0   # Write to screen location 2048
    STRI    R8, R5, 0   # Write to screen location 2048

    # --- Compute Next Fibonacci Step ---
    ADD     R7, R1, R2  # R7 = Next number (R1 + R2)
    MOV     R1, R2      # Shift window: R1 = old R2
    MOV     R2, R7      # Shift window: R2 = new R7

    # --- Loop Control ---
    DEC     R3          # Decrement loop counter
    NOP
    NOP
    BGTU    R3, R0, FIB_LOOP
    NOP

# 3. Shutdown
HALT_LBL:
    HLT                 # Stop Execution

# ==============================================================================
# Data Tables
# ==============================================================================
FONT_TABLE:
    .word 0x7B6F  # Digit 0
    .word 0x2C97  # Digit 1
    .word 0x73E7  # Digit 2
    .word 0x73CF  # Digit 3
    .word 0x5BC9  # Digit 4
    .word 0x79CF  # Digit 5
    .word 0x79EF  # Digit 6
    .word 0x7249  # Digit 7
    .word 0x7BEF  # Digit 8
    .word 0x7BC9  # Digit 9

LOOP_MEMORY: