; =========================================
; MINIMAL ROM 
; The reset vector (0x8000)
; =========================================
_start:
    ; Clear general-purpose registers (R1 - R14)
    ; R0 is a zero reg
    MOVI R1,  0x0000
    MOVI R2,  0x0000
    MOVI R3,  0x0000
    MOVI R4,  0x0000
    MOVI R5,  0x0000
    MOVI R6,  0x0000
    MOVI R7,  0x0000
    MOVI R8,  0x0000
    MOVI R9,  0x0000
    MOVI R10, 0x0000
    MOVI R11, 0x0000
    MOVI R12, 0x0000
    MOVI R13, 0x0000
    MOVI R14, 0x0000

    ; Initialize Stack Pointer (SP) to top word of RAM
    MOVI R15, 0x03FF    ; Point SP to word 1023 (last 4-byte word in 4 KiB RAM)

    ; Reset cache
    INVALL

    ; Reset DMA
    STRI R0 R0 0x0A01
    STRI R0 R0 0x0A02
    STRI R0 R0 0x0A03
    STRI R0 R0 0x0A04
    STRI R0 R0 0x0A05
    STRI R0 R0 0x0A00

    ; Load superblock in page 0
    MOVI R1 1      ; Control: start
    MOVI R2 0x0001 ; Src: Volume ID 1
    MOVI R3 0x0000 ; Dest: RAM
    MOVI R4 0x0000 ; Page 0 (Superblcok)
    MOVI R5 0x0000 ; Load address: 0x0000
    MOVI R6 0x0040 ; length: 64

    STRI R2 R0 0x0A01 ; Store the commands into DMA
    STRI R3 R0 0x0A02
    STRI R4 R0 0x0A03
    STRI R5 R0 0x0A04
    STRI R6 R0 0x0A05
    STRI R1 R0 0x0A00 ; Fire DMA

wait_superblock:
    ; LDRI R7 R0 0x0A00
    ; BNE R7 R0 wait_superblock ; Poll until DMA transfer finishes
    ; Unsure to poll considering my DMA takes control of the ram entirely
    ; Actually could be useful when I make my I-cache in the future

    ; Load Kernel from file system
    ; Verify Superblock Magic Number ("FSSB" -> 0x42535346)
    MOVI R8  0x5346
    MOVI R9  0x4253
    MOVI R10 0
find_magic_numer_SB:
    LDR  R11 R0  R10
    LDRI R12 R10 1
    MOVI R4  128
    BGT  R10 R4  error
    ADDI R10 R10 14
    BNE  R11 R8  find_magic_numer_SB
    BNE  R12 R9  find_magic_numer_SB
    SUBI R10 R10 14

    ; Load File Table (Page 1) into Scratch Buffer
    LDRI R4 R10 10     ; Dynamically take the pointer to file table start
    MOVI R5 0x0080     ; Load Address: 0x0080 (RAM Offset to not overwrite Superblock)
    STRI R4 R0 0x0A03
    STRI R5 R0 0x0A04
    STRI R1 R0 0x0A00    ; Fire DMA

wait_filetable:
    ; LDRI R7 R0 0x0A00
    ; BNE  R7 R0 wait_filetable

    ; Verify FileTable Magic Number ("FSFT" -> 0x46545346)
    MOVI R13 0x4654
    MOV  R14 R5
find_magic_numer_FT:
    LDR  R11 R0  R14
    LDRI R12 R14 1
    MOVI R4  128
    ADD  R4  R4  R5
    BGT  R14 R4  wait_superblock
    ADDI R14 R14 14
    BNE  R11 R8  find_magic_numer_FT
    BNE  R12 R13 find_magic_numer_FT
    SUBI R14 R14 14

    ; Find file type Kernel
    ADDI R11 R14 2
    LDRI R11 R11 0
    MOVI R12 2
    ADDI R14 R14 14
    BNE  R11 R12 find_magic_numer_FT

    ; Load Kernel (Page 2) into Scratch Buffer
    LDRI R4 R14 8     ; Page 2 (File Table)
    MOVI R5 0x00C0     ; Load Address: 0x00C0 (RAM Offset to not overwrite Superblock)
    STRI R4 R0 0x0A03
    STRI R5 R0 0x0A04
    STRI R1 R0 0x0A00    ; Fire DMA

    ; Jump to Kernel
    JMPR R5         ; PC jumps to Kernel 0x00C0

error:
    HLT

; Helper method for context storing
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

IREQ

; DMA vector
DMA_vector:
    SUBI R15 R15 2
    STRI R14 R15 0

    CALL Store_CPU_context

    ; --- Continue ISR Logic Here ---

    JMP Load_CPU_context