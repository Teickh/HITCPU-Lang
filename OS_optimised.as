; ==================================================================
; ADDRESS 0x0000 - THIS IS WHERE YOUR ROM JUMPS TO
; ==================================================================

os_entry:
    ; 1. SETUP PHASE
    ; (R15 is already 0x03FF as set by your hardware ROM)
    CALL init_peripherals       ; Configure your MMIO registers

main_loop:
    ; 2. THE OS SUPER LOOP
    CALL check_mmio_status      ; Poll your MMIO registers or DMA flags
    NOP

    BEQ  R4, R0, idle           ; If R4 == 0 (no work), jump to idle
    NOP

    CALL process_work           ; Do something with the hardware data
    NOP

idle:
    JMP  main_loop              ; Repeat the loop forever
    NOP

; ==================================================================
; SUBROUTINES
; ==================================================================

init_peripherals:
    ; --- 1. Setup MMIO configurations ---
    ; (Your existing peripheral setup code here)

    ; --- 2. Initialize the Flat Page Table ---
    ; Place our 16-entry page table at RAM address 0x0020 (4 bytes per entry)
    MOVI R1, page_table               ; R1 = Pointer to Page Table base in RAM
    NOP

    ; Example: Map Virtual Page 0 to Physical Frame 0 (Valid = 1)
    MOVI R2, 0x8000               ; Bit 15 set (Valid = 1), Frame 0
    NOP
    NOP
    STRI R2, R1, 0                ; Store into Page Table Entry 0

    ; Example: Map Virtual Page 1 to Physical Frame 1 (Valid = 1)
    ADDI R1, R1, 4                ; Move to Entry 1 address
    NOP
    MOVI  R2, 0x8001              ; Valid = 1, Frame 1
    NOP
    NOP
    STRI R2, R1, 0                ; Store into Page Table Entry 1

    ; --- 3. Clear the remaining pages (Entries 2 to 15 = 14 entries) ---
    MOVI R3, 14                   ; R3 = Counter for looping 16 times
    NOP

clear_page_loop:
    ADDI R1, R1, 4
    NOP
    DEC R3
    NOP
    STRI R0, R1, 0                ; Store into Page Table Entry 0
    BNE  R3, R0, clear_page_loop

    ; --- 3. Set the "CR3" Page Table Pointer Register ---
    MOVI R2, 0x0020
    NOP
    NOP
    LPT R2                        ; Send base address to your MMU hardware/mod

    RET

check_mmio_status:
    ; Read status from your hardware MMIO address into register R0
    RET

process_work:
    ; Handle the task (e.g. process data buffer, clear MMIO flag)
    RET

page_table: