[ Words 0 - 3   ] : Interrupt & Reset Jump Vectors (4 words)
[ Words 4 - 19  ] : The 16-Entry Page Table (16 words)
[ Words 20 - 63 ] : OS Entry Code (os_entry, init_peripherals) (44 words)

; ==================================================================
; ADDRESS 0x0000 - THIS IS WHERE YOUR ROM JUMPS TO
; ==================================================================

os_entry:
    ; 1. SETUP PHASE
    CALL init_peripherals       ; Configure MMIO registers & Page Table

main_loop:
    ; 2. THE OS SUPER LOOP
    CALL check_mmio_status      ; Returns status code into R4 (Not R0!)

    BEQ  R4, R0, idle           ; If R4 == 0 (no work), jump to idle
    CALL process_work           ; Do something with the hardware data

idle:
    JMP  main_loop              ; Repeat the loop forever

; ==================================================================
; SUBROUTINES
; ==================================================================

init_peripherals:
    ; --- 1. Setup MMIO configurations ---
    ; (Your peripheral setup code here)

    ; --- 2. Initialize the Flat Page Table ---
    ; Page Table Base Address = 0x0020
    MOVI R1, 0x0020             ; R1 = Pointer to Page Table base in RAM

    ; Map Virtual Page 0 -> Physical Frame 0 (Valid = 1 -> 0x8000)
    MOVI R2, 0x8000             ; Bit 15 set (Valid = 1), Frame 0
    STRI R2, R1, 0              ; Store into Page Table Entry 0 (RAM 0x0020)

    ; Map Virtual Page 1 -> Physical Frame 1 (Valid = 1 -> 0x8001)
    ADDI R1, R1, 4              ; Move 4 bytes forward to RAM 0x0024
    MOVI R2, 0x8001             ; Valid = 1, Frame 1
    STRI R2, R1, 0              ; Store into Page Table Entry 1 (RAM 0x0024)

    ; --- 3. Clear the remaining pages (Entries 2 to 15 = 14 entries) ---
    MOVI R3, 14                 ; Counter for remaining 14 entries

clear_page_loop:
    ADDI R1, R1, 4              ; Move to next entry (0x0028, 0x002C, etc.)
    DEC  R3
    STRI R0, R1, 0              ; Store hardwired zero (R0) into Page Table
    BNE  R3, R0, clear_page_loop

    ; --- 4. Set the Page Table Pointer Register (CR3) ---
    MOVI R2, 0x0020             ; Must match the 0x0020 base address above!
    LPT  R2                     ; Send 0x0020 to MMU hardware

    RET

check_mmio_status:
    ; Read status from MMIO into R4 (R0 discards writes since it's hardwired 0)
    RET

process_work:
    ; Handle the task
    RET