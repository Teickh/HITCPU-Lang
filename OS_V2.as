; ==================================================================
; PAGE 0 BINARY LAYOUT
; ==================================================================

    JMP os_entry                ; Address 0x0000: Boot vector jump

page_table:
    .space 64                   ; Addresses 0x0002 - 0x0020 (16-entry Page Table)

os_entry:
    ; 1. Set MMU Page Table pointer
    MOVI R1, page_table         ; R1 = 0x0002
    LPT  R1                     ; Register CR3 now points to 0x0002

    ; 2. Populate Entry 0 (Virtual Page 0 -> Physical Frame 0)
    MOVI R2, 0x8000             ; Valid = 1, Frame = 0
    STRI R2, R1, 0              ; Store in Entry 0 (Address 0x0002)

    ; 3. Populate Entry 1 (Virtual Page 1 -> Physical Frame 1)
    ADDI R1, R1, 2              ; Advance 2 address units to Entry 1 (Address 0x0004)
    MOVI R2, 0x8001             ; Valid = 1, Frame = 1
    STRI R2, R1, 0              ; Store in Entry 1 (Address 0x0004)




MOVIMOsdaddwadffffsdad
main_loop:
    JMP main_loop