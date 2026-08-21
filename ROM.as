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

    ; Load superblock in page 0
    MOVI R1 1      ; Control: start
    MOVI R2 0x0001 ; Src: Volume ID 1
    MOVI R3 0x0000 ; Dest: RAM
    MOVI R4 0x0000 ; Page 0 (Superblcok)
    MOVI R5 0x0000 ; Load address: 0x0000
    MOVI R6 0x0080 ; length: 128

    STRI R2 R0 0x0601 ; Store the commands into DMA
    STRI R3 R0 0x0602
    STRI R4 R0 0x0603
    STRI R5 R0 0x0604
    STRI R6 R0 0x0605
    STRI R1 R0 0x0600 ; Fire DMA

wait_superblock:
    ; LDRI R7 R0 0x0600
    ; BNE R7 R0 wait_superblock ; Poll until DMA transfer finishes
    ; Unsure to poll considering my DMA takes control of the ram entirely
    ; Actually could be useful when I make my I-cache in the future

    ; Load Kernel from file system
    ; Verify Superblock Magic Number ("FSSB" -> 0x42535346)
    MOVI R8 0x5346
    MOVI R9 0x4253
    MOVI R10 0
find_magic_numer_SB:
    LDR R11 R0 R10
    LDRI R12 R10 1
    MOVI R4 128
    BGT R10 R4 error
    ADDI R10 R10 14
    BNE R11 R8 find_magic_numer_SB
    BNE R12 R9 find_magic_numer_SB
    SUBI R10 R10 14

    ; Load File Table (Page 1) into Scratch Buffer
    LDRI R4 R10 10     ; Dynamically take the pointer to file table start
    MOVI R5 0x0080     ; Load Address: 0x0080 (RAM Offset to not overwrite Superblock)
    STRI R4 R0 0x0603
    STRI R5 R0 0x0604
    STRI R1 R0 0x0600    ; Fire DMA

wait_filetable:
    ; LDRI R7 R0 0x0600
    ; BNE R7 R0 wait_filetable

    ; Verify FileTable Magic Number ("FSFT" -> 0x46545346)
    MOVI R13 0x4654
    MOV R14 R5
find_magic_numer_FT:
    LDR R11 R0 R14
    LDRI R12 R14 1
    MOVI R4 128
    ADD R4 R4 R5
    BGT R14 R4 wait_superblock
    ADDI R14 R14 14
    BNE R11 R8 find_magic_numer_FT
    BNE R12 R13 find_magic_numer_FT
    SUBI R14 R14 14

    ; Find file type Kernel
    ADDI R11 R14 2
    LDRI R11 R11 0
    MOVI R12 2
    ADDI R14 R14 14
    BNE R11 R12 find_magic_numer_FT

    ; Load Kernel (Page 2) into Scratch Buffer
    LDRI R4, R14, 8     ; Page 2 (File Table)
    MOVI R5, 0x00C0     ; Load Address: 0x00C0 (RAM Offset to not overwrite Superblock)
    STRI R4 R0 0x0603
    STRI R5 R0 0x0604
    STRI R1 R0 0x0600    ; Fire DMA

    ; Jump to Kernel
    JMPR R5         ; PC jumps to Kernel 0x00C0

error:
    HLT