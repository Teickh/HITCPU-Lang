; -----------------------------
; TWIST Kernel
; -----------------------------

kernel_start:
    ; Initialize ISR interrutps

    ; Initialize screen
    ; install the font

    ; Setup Common DMA Transfer Parameters (only set once)
    MOVI R1 1          ; Command: Start DMA
    MOVI R2 0x0001     ; Src: Volume ID 1
    MOVI R3 0x0001     ; Dest base: FONT_ROM (I'm unsure where it is. Placeholder 1)
    MOVI R4            ; Page: I got no idea
    MOVI R5 0x0000     ; Address: No need since the destination isn't towrds an address.
    MOVI R6 0x0071     ; Transfer Length: 95 characters

    STRI R2 R0 0x0A01
    STRI R3 R0 0x0A02
    STRI R4 R0 0x0A03
    STRI R5 R0 0x0A04
    STRI R6 R0 0x0A05
    STRI R1 R0 0x0A00 ; Start DMA

    ; 0x0800 : SCREEN_CTRL      ; Control flags (Enable, Mode select)
    ; 0x0801 : SCREEN_WIDTH     ; Screen width (Columns or Pixels)
    ; 0x0802 : SCREEN_HEIGHT    ; Screen height (Rows or Pixels)
    ; 0x0803 : SCREEN_DATA_CMD  ; Character byte or GPU Command ID
    ; 0x0804 : SCREEN_PARAM_X   ; GPU Drawing Coordinate X
    ; 0x0805 : SCREEN_PARAM_Y   ; GPU Drawing Coordinate Y
    ; 0x0806 : SCREEN_PARAM_COL ; GPU Active Draw Color
    ; 0x0807 : SCREEN_STATUS    ; Status flags (Ready, VSYNC, Errors)
    MOVI R1 ; screen width
    MOVI R2 ; screen height
    MOVI R3 0x0011 ; Screen start in mode console

kernel_loop:
    JMP kernel_loop