# ==============================================================================
# HITCPU Program: Print Digits 0 to 9
# ==============================================================================

# 1. Initialization
MOVI    R1, 0          # R1 = Current Digit Counter (0 to 9)
MOVI    R2, 10         # R2 = Upper Bound (Stop at 10)
MOVI    R3, 2048       # R3 = MMIO Frame Buffer Pointer
MOVI    R4, FONT_TABLE # R4 = Base Address of Font Lookup Table
NOP
NOP

# 2. Main Print Loop
PRINT_LOOP:
    ADD     R5, R1, R4     # Calculate memory address for current digit

    # --- Fill 2 slots between ADD and LDR (R5 hazard) ---
    ADDI    R1, R1, 1      # Slot 1: Modifies R1 (independent of R5)
    NOP

    LDR     R6, R5, 0      # EX phase calculates address, MEM reads data
    

    # --- Fill 2 slots between LDR and STR (R6 hazard) ---
    # Need 2 instructions here while LDR passes through MEM and WB!
    # If we have no useful work, we use BGTU pre-calculation or NOPs:
    NOP                    # Delay slot 1 for LDR -> STR
    NOP                    # Delay slot 2 for LDR -> STR
    
    STR     R6, R3, 0     # Reads R6 safely (since INC R3 ran earlier, offset is -1)
    ADDI    R3, R3, 1      # Slot 2: Modifies R3 (will adjust offset in STR)

    # If R2 (10) > R1 (Current Digit), keep looping
    BGTU    R2, R1, PRINT_LOOP
    NOP                    # Branch delay slot (if your CPU requires one)

# 3. Shutdown
HALT_LBL:
    HLT

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