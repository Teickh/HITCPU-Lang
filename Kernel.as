; -----------------------------
; TWIST Kernel (Byte-Addressable)
; -----------------------------

kernel_start:
    ; Initialize ISR interrupts

    ; Initialize screen MMIO registers (Byte-mapped addresses)
    ; 0x2000 : SCREEN_CTRL      ; Control flags (Enable, Mode select)
    ; 0x2001 : SCREEN_WIDTH     ; Screen width (Columns or Pixels)
    ; 0x2002 : SCREEN_HEIGHT    ; Screen height (Rows or Pixels)
    ; 0x2003 : SCREEN_DATA_CMD  ; Character byte or GPU Command ID
    ; 0x2004 : SCREEN_PARAM_X   ; GPU Drawing Coordinate X
    ; 0x2005 : SCREEN_PARAM_Y   ; GPU Drawing Coordinate Y
    ; 0x2006 : SCREEN_PARAM_COL ; GPU Active Draw Color
    ; 0x2007 : SCREEN_STATUS    ; Status flags (Ready, VSYNC, Errors)
    
    MOVI R1 80          ; Screen width (e.g., 80 columns)
    MOVI R2 25          ; Screen height (e.g., 25 rows)
    MOVI R3 0x0011      ; Screen start in console mode

    STRI R1 R0 0x2001
    STRI R2 R0 0x2002
    STRI R3 R0 0x2000

    ; --------------------------------------------------
    ; Print TWIST OS to screen
    ; Font is in storage, so locate and load the header
    ; --------------------------------------------------
    
    ; Setup Common DMA Transfer Parameters
    MOVI R1 1           ; Command: Start DMA
    MOVI R2 0x0001      ; Src: Volume ID 1
    MOVI R3 0x0000      ; Dest base: RAM (Byte address 0x0000)
    MOVI R6 128         ; Transfer Length: 128 bytes (64 words)

    ; --------------------------------------------------
    ; Phase 1: Read Superblock (Page 0 -> RAM 0x0000)
    ; --------------------------------------------------
    ; Setup Magic Search Registers
    MOVI R8  0x5346      ; Low 16 bits: "FS" (shared for FSSB & FSFT)
    MOVI R9  0x4253      ; High 16 bits: "SB"
    MOVI R10 0           ; Search byte offset starting at 0x0000
    MOVI R4  256         ; Search byte boundary limit (128 words = 256 bytes)

find_magic_number_SB:
    BGT  R10 R4  error   ; If byte offset > 256, throw error
    LDR  R11 R0  R10     ; Read low 16 bits of header (offset +0 bytes)
    LDRI R12 R10 2       ; Read high 16 bits of header (offset +2 bytes)
    
    ; Test Magic ("FSSB")
    BNE  R11 R8  next_SB_entry
    BEQ  R12 R9  found_SB

next_SB_entry:
    ADDI R10 R10 28      ; Advance to next entry struct (14-word stride = 28 bytes)
    JMP  find_magic_number_SB

found_SB:
    ; --------------------------------------------------
    ; Phase 2: Load File Table (Page -> RAM 0x0100)
    ; --------------------------------------------------
    LDRI R4 R10 20       ; Read FT Page pointer from Superblock (word 10 = byte 20)
    MOVI R5 0x0100       ; Scratch Buffer target byte address: 0x0100 (byte 256)
    
    STRI R4 R0 0x2003    ; Set Page
    STRI R5 R0 0x2004    ; Set Load Address
    STRI R1 R0 0x2000    ; Fire DMA

    ; Search for File Table Magic ("FSFT")
    MOVI R13 0x4654      ; High 16 bits: "FT"
    MOV  R14 R5          ; Search pointer starts at byte 0x0100
    MOVI R4  416         ; Search boundary limit (256 + 160 bytes = 416)

find_magic_number_FT:
    BGT  R14 R4  error   ; If corrupted, handle error
    LDR  R11 R0  R14
    LDRI R12 R14 2       ; Read high 16 bits (offset +2 bytes)

    ; Test Magic ("FSFT")
    BNE  R11 R8  next_FT_entry
    BNE  R12 R13 next_FT_entry

    ; Verify File Type == Font File (e.g., Type ID: 3 or 2)
    LDRI R11 R14 4       ; Read File Type field (word 2 = byte 4)
    MOVI R12 3           ; Target Font File Type ID
    BEQ  R11 R12 found_font

next_FT_entry:
    ADDI R14 R14 28      ; Advance to next entry struct (14-word stride = 28 bytes)
    JMP  find_magic_number_FT

found_font:
    ; --------------------------------------------------
    ; Phase 3: Load Font Header into RAM & Enter Kernel Loop
    ; --------------------------------------------------
    LDRI R4 R14 16       ; Read Font Page Start offset (word 8 = byte 16)
    MOVI R5 Font_header  ; Target RAM address for Font_header
    
    STRI R4 R0 0x0A03    ; Set Page
    STRI R5 R0 0x0A04    ; Set Load Address
    STRI R1 R0 0x0A00    ; Fire DMA

    JMP kernel_loop

error:
    HLT
    JMP error            ; Fallback lock in case HLT is bypassed

kernel_loop:
    ; Program loader, scheduler, future OS systems.
    JMP kernel_loop

Font_header:
    ; Reserved space for Font Header / Asset Data loaded from storage