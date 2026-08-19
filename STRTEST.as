MOVI    R1, FONT_TABLE
MOVI    R2, MEMORY
MOVI    R4, 10
MOVI    R3, 2048

LOOP:
    STRI    R1, R2, 0
    DEC     R4
    STRI    R1, R3, 0
    ADDI    R3, R3, 2
    BGTU    R4, R0, LOOP

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

MEMORY: