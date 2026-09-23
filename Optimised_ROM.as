; =========================================
; OPTIMIZED MINIMAL ROM (Byte-Addressable)
; The reset vector (0x8000)
; =========================================
_start:
    ; Initialize Stack Pointer (SP) to top word of RAM
    MOVI R15, 0x0FFE    ; SP points to byte 4094 (last 16-bit word in 4 KiB RAM)

    ; Reset cache
    INVALL

    ; Reset DMA Channels
    STRI R0 R0 0x0A00
    STRI R0 R0 0x0A01
    STRI R0 R0 0x0A02
    STRI R0 R0 0x0A03
    STRI R0 R0 0x0A04
    STRI R0 R0 0x0A05

    ; Setup Common DMA Transfer Parameters
    MOVI R1 1          ; Command: Start DMA
    MOVI R2 0x0001     ; Src: Volume ID 1
    MOVI R3 0x0000     ; Dest base: RAM (Byte address 0x0000)
    MOVI R6 128        ; Transfer Length: 128 bytes (64 words)

    STRI R2 R0 0x0A01
    STRI R3 R0 0x0A02
    STRI R6 R0 0x0A05

    ; --------------------------------------------------
    ; Phase 1: Load Superblock (Page 0 -> RAM 0x0000)
    ; --------------------------------------------------
    STRI R0 R0 0x0A03   ; Page 0
    STRI R0 R0 0x0A04   ; Load address 0x0000
    STRI R1 R0 0x0A00   ; Fire DMA

    ; Setup Magic Search Registers
    MOVI R8  0x5346     ; Low 16 bits: "FS"
    MOVI R9  0x4253     ; High 16 bits: "SB"
    MOVI R10 0          ; Search byte offset starting at 0x0000
    MOVI R4  256        ; Search byte limit (128 words = 256 bytes)

find_magic_number_SB:
    BGT  R10 R4  error  ; If byte offset > 256, throw error
    LDR  R11 R0  R10    ; Read low 16 bits of header (offset +0 bytes)
    LDRI R12 R10 2      ; Read high 16 bits of header (offset +2 bytes)
    
    ; Test Magic ("FSSB")
    BNE  R11 R8  next_SB_entry
    BEQ  R12 R9  found_SB

next_SB_entry:
    ADDI R10 R10 28     ; Advance to next entry struct (14-word stride = 28 bytes)
    JMP  find_magic_number_SB

found_SB:
    ; --------------------------------------------------
    ; Phase 2: Load File Table (Page -> RAM 0x0100)
    ; --------------------------------------------------
    LDRI R4 R10 20      ; Read FT Page pointer from Superblock (word 10 offset = byte 20)
    MOVI R5 0x0100      ; Scratch Buffer target byte address: 0x0100 (byte 256)
    
    STRI R4 R0 0x0A03   ; Set Page
    STRI R5 R0 0x0A04   ; Set Load Address
    STRI R1 R0 0x0A00   ; Fire DMA

    ; Search for File Table Magic ("FSFT")
    MOVI R13 0x4654     ; High 16 bits: "FT"
    MOV  R14 R5         ; Search pointer starts at byte 0x0100 (256)
    MOVI R4  416        ; Search boundary limit (256 + 160 bytes = 416)

find_magic_number_FT:
    BGT  R14 R4  _start ; If corrupted, restart bootloader process
    LDR  R11 R0  R14
    LDRI R12 R14 2      ; Offset +2 bytes

    ; Test Magic ("FSFT")
    BNE  R11 R8  next_FT_entry
    BNE  R12 R13 next_FT_entry

    ; Verify File Type == Kernel (Type ID: 2)
    LDRI R11 R14 4      ; Read File Type field (word 2 offset = byte 4)
    MOVI R12 2
    BEQ  R11 R12 found_kernel

next_FT_entry:
    ADDI R14 R14 28     ; Advance to next entry struct (14-word stride = 28 bytes)
    JMP  find_magic_number_FT

found_kernel:
    ; --------------------------------------------------
    ; Phase 3: Load Kernel (Kernel Page -> RAM 0x0180) & Jump
    ; --------------------------------------------------
    LDRI R4 R14 16      ; Read Kernel Page Start offset (word 8 offset = byte 16)
    MOVI R5 0x0180      ; Kernel target load byte address: 0x0180 (byte 384)
    
    STRI R4 R0 0x0A03   ; Set Page
    STRI R5 R0 0x0A04   ; Set Load Address
    STRI R1 R0 0x0A00   ; Fire DMA

    JMPR R5             ; Jump execution directly to Kernel byte address 0x0180

error:
    HLT

; Helper method for context storing (Stack offsets in bytes)
Store_CPU_context:
    SUBI R15 R15 26

    STRI R1  R15 0
    STRI R2  R15 2
    STRI R3  R15 4
    STRI R4  R15 6
    STRI R5  R15 8
    STRI R6  R15 10
    STRI R7  R15 12
    STRI R8  R15 14
    STRI R9  R15 16
    STRI R10 R15 18
    STRI R11 R15 20
    STRI R12 R15 22
    STRI R13 R15 24

    RET

Load_CPU_context:
    LDRI R1  R15 0
    LDRI R2  R15 2
    LDRI R3  R15 4
    LDRI R4  R15 6
    LDRI R5  R15 8
    LDRI R6  R15 10
    LDRI R7  R15 12
    LDRI R8  R15 14
    LDRI R9  R15 16
    LDRI R10 R15 18
    LDRI R11 R15 20
    LDRI R12 R15 22
    LDRI R13 R15 24
    LDRI R14 R15 26

    ADDI R15 R15 28
    IREQ

; DMA vector
DMA_vector:
    SUBI R15 R15 2
    STRI R14 R15 0

    CALL Store_CPU_context

    ; --- Continue ISR Logic Here ---
    HLT

    JMP Load_CPU_context