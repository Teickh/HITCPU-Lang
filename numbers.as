# ==============================================================================
# HITCPU Program: Print Digits 1 to 9 via Font Lookup Table
# ==============================================================================

# 1. Initialization
MOVI    R1, 0          # R1 = Current Digit Counter (Start at 1, going up to 9)
MOVI    R2, 10         # R2 = Upper Bound (Stop when counter reaches 10)
MOVI    R3, 2048       # R3 = MMIO Frame Buffer Pointer (Bit 12 is 1)
MOVI    R4, FONT_TABLE # R4 = Base Address of our Font Lookup Table

# 2. Main Print Loop
PRINT_LOOP:
    # --- Calculate the memory address of the font data ---
    # Each digit layout is 2 bytes wide (1 word). 
    # Address = Base (R4) + (Current Digit (R1) * 2)
    # If your architecture doesn't have a multiply instruction, 
    # we can just add the index to itself or add 2 to the pointer manually!
    
    # ADD     R5, R1, R1     # R5 = R1 * 2 (Byte offset for 2-byte words)
    ADD     R5, R1, R4     # R5 = FONT_TABLE + Digit Index
    # ADD     R5, R5, R4     # R5 = Font Base Address + Byte Offset

    # --- Load the Glyph Data from RAM ---
    LDRI    R6, R5, 0      # R6 = Load the raw pixel bitmap for this digit

    # --- Write it to the Screen (MMIO) ---
    STRI    R6, R3, 0      # Dump the bitmap into the Frame Buffer

    # --- Advance Screen Pointer & Increment Counter ---
    INC     R3             # Move screen pointer to the next slot so they don't overwrite
    INC     R1             # Increment digit counter (e.g., from 1 to 2)

    # --- Loop Condition ---
    # If R2 (10) > R1 (Current Digit), keep looping
    BGTU    R2, R1, PRINT_LOOP

# 3. Shutdown
HALT_LBL:
    HLT                    # Stop Execution

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