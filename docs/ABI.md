# TWIST ABI

## Overview

The TWIST ABI defines the conventions used by compiled programs,
libraries, and the TWIST operating system when interacting with
the TWIST architecture.

This document currently targets the TWIST V3 architecture.

## Register Convention

| Register | Purpose                   |
|----------|---------------------------|
| R0       | Zero register             |
| R1-R13   | General-purpose registers |
| R14      | Return address            |
| R15      | Stack pointer             |

R0 always reads as zero and ignores writes.

R14 contains the return address during normal function calls.

R15 is used as the stack pointer.

## TWIST V3 Memory Map

| Address | Memory | Size |
| :--- | :--- | :--- |
| 0x0000 - 0x0FFF | RAM | 4 KiB | 
| 0x1000 - 0x1FFF | MMIO | 4 KiB address space | 
| 0x2000 - 0x.... | ROM | Firmware / reset code | 

Layout subject to change.



# DMA Controller Specification

The DMA Controller handles asynchronous block data transfers between Disk storage and system RAM. It occupies a 16-byte block in MMIO space aligned to 2-byte (16-bit) word boundaries using **Big-Endian** encoding.

## Register Map

| Offset  | Bytes  | Register Name | Access  | Description |
|---------|--------|---------------|---------|-------------|
| `+0x00` | `0..1` | `DMA_CTRL`    |   R/W   | **Control Register**<br>• Write `1` (`0x0001`) to initiate DMA transfer.<br>• Hardware resets to `0` when transfer completes/IRQ accepted. |
| `+0x02` | `2..3` | `DMA_SRC` | R/W | **Transfer source**<br>The file source from specified disk |
| `+0x04` | `4..5` | `DMA_DEST` | R/W | **Transfer Destination**<br>Target destination of the transfer |
| `+0x06` | `6..7` | `DMA_PAGE` | R/W | **Disk Page**<br>Initial disk page index to begin reading from. |
| `+0x08` | `8..9` | `DMA_ADDR` | R/W | **Destination Address**<br>Target memory destination address for data transfer. |
| `+0x0A` | `10..11` | `DMA_LEN` | R/W | **Transfer Length**<br>Number of bytes/words to transfer. |
| `+0x0C` | `12..13` | `DMA_STATUS` | Read | **Status Register**<br>Current status of the hardware state machine (see table below). |
| `+0x0E` | `14..15` | `DMA_PAGE_CTR`| Read | **Page Counter**<br>Hardware page tracker for active/multi-page transfers. |

---

## DMA Status Codes (`DMA_STATUS`)

| Code | Value | Name | Description |
| :--- | :--- | :--- | :--- |
| `0` | `0x0000` | `STATUS_IDLE` | DMA engine is idle and ready for requests. |
| `1` | `0x0001` | `STATUS_DONE` | Full transfer complete (triggers IRQ check & control reset). |
| `2` | `0x0002` | `STATUS_BUSY` | Hardware clock active; page transfer in progress. |
| `3` | `0x0003` | `STATUS_PAGE_DONE` | Current disk page finished; triggers page increment pulse. |
| `4` | `0x0004` | `STATUS_LOAD_NEXT_PAGE` | Next page requested from disk buffer. |
| `5` | `0x0005` | `STATUS_PAGE_LOADED` | Next page buffered into hardware; releases clock stall. |



# FS-16/32 Simple Filesystem (FSSB/FSFT) ABI Specification v1.0

## 1. Physical & Architectural Parameters

* **Endianness:** Little-Endian (`0x1234` stored as `0x34`, `0x12`)
* **Page Size:** 256 Bytes (64 x 32-bit words / 128 x 16-bit words)
* **Disk Capacity:** 256 Pages = 65,536 Bytes (64 KiB total)
* **Maximum Files:** 8 files (constrained by Page 1 File Table size)
* **File Name Constraint:** 8 ASCII characters, 8-bit ASCII, right-padded with space (`0x20`), null terminator **not** guaranteed.

---

## 2. Disk Memory Map

| Page Range | Byte Range | 32-Bit Word Range | Usage / Description |
| :--- | :--- | :--- | :--- |
| **Page 0** | `0x0000` – `0x00FF` | `0` – `63` | **Superblock** (Disk metadata & header) |
| **Page 1** | `0x0100` – `0x01FF` | `64` – `127` | **File Table** (Contains up to 8 File Headers) |
| **Pages 2–3** | `0x0200` – `0x03FF` | `128` – `255` | **Reserved System Pages** |
| **Pages 4–255** | `0x0400` – `0xFFFF` | `256` – `16383` | **Contiguous Data Pages** (Executables, Kernels, Assets) |

---

## 3. Data Structures

### 3.1 Superblock Layout (Page 0)
Located at fixed byte offset **`0x0000`**. Occupies 28 bytes (7 x 32-bit words); remaining 228 bytes of Page 0 are reserved/zeroed.

| Byte Offset | 16-Bit Word | 32-Bit Word | Type | Field Name | Constant / Description |
| :--- | :--- | :--- | :--- | :--- | :--- |
| `0x00` | `0` | `0` (low) | `uint16` | `magic_low` | `0x5346` ("FS") |
| `0x02` | `1` | `0` (high)| `uint16` | `magic_high` | `0x4253` ("SB") -> Combined 32-bit Magic: `0x42535346` |
| `0x04` | `2` | `1` | `uint32` | `version` | Filesystem Version (default: `1`) |
| `0x08` | `4` | `2` | `uint32` | `volume_id` | Volume / Medium Identifier |
| `0x0C` | `6` | `3` | `uint32` | `total_pages` | Total disk capacity in pages (`256`) |
| `0x10` | `8` | `4` | `uint32` | `next_free_page`| Page index where the next file allocation begins |
| `0x14` | `10` | `5` | `uint32` | `file_table_page`| Start page index of File Table (`1`) |
| `0x18` | `12` | `6` | `uint32` | `total_files` | Total valid directory entries in File Table (0 <= N <= 8) |

---

### 3.2 File Table Entry Layout (Page 1)
The File Table starts at byte offset **`0x0100`**. Each file entry has a fixed stride of **32 Bytes** (8 x 32-bit words / 16 x 16-bit words).

`Entry Byte Offset = 0x0100 + (FileIndex * 32)`

| Byte Offset | 16-Bit Word | 32-Bit Word | Type | Field Name | Description |
| :--- | :--- | :--- | :--- | :--- | :--- |
| `+0x00` | `+0` | `+0` (low) | `uint16` | `magic_low` | `0x5346` ("FS") |
| `+0x02` | `+1` | `+0` (high)| `uint16` | `magic_high` | `0x4654` ("FT") -> Combined 32-bit Magic: `0x46545346` |
| `+0x04` | `+2` | `+1` | `uint32` | `file_type` | Enumerated File Type ID (See Section 3.3) |
| `+0x08` | `+4` | `+2` | `char[4]` | `name_chunk0` | First 4 characters of file name (ASCII, LE) |
| `+0x0C` | `+6` | `+3` | `char[4]` | `name_chunk1` | Last 4 characters of file name (ASCII, LE) |
| `+0x10` | `+8` | `+4` | `uint32` | `start_page` | Starting page index on disk |
| `+0x14` | `+10` | `+5` | `uint32` | `page_count` | Allocated contiguous page count |
| `+0x18` | `+12` | `+6` | `uint32` | `reserved_0` | Padding / Reserved (`0x00000000`) |
| `+0x1C` | `+14` | `+7` | `uint32` | `reserved_1` | Padding / Reserved (`0x00000000`) |

---

### 3.3 File Type Enumeration (`file_type`)

| Value (`uint32`) | Identifier | Description | Recommended Extension |
| :--- | :--- | :--- | :--- |
| `0` | `FILE_TYPE_UNKNOWN` | Fallback / Raw Unclassified Binary | `.dat` / none |
| `1` | `FILE_TYPE_EXECUTABLE` | User Executable Binary | `.bin` / `.exe` |
| `2` | `FILE_TYPE_KERNEL` | OS Kernel Executable Binary | `.krn` / `kernel.bin` |
| `3` | `FILE_TYPE_IMAGE` | Display Graphic / Font Asset / Texture | `.img` / `.bmp` |
| `4` | `FILE_TYPE_TEXT` | Configuration / Human-readable ASCII | `.txt` / `.cfg` |

---

## 4. Software Access & Traversal Rules

1. **Superblock Parsing:**
   * Read 32-bit value at byte `0x0000`. Assert value equals `0x42535346` ("FSSB").
   * Read `uint32` at byte `0x018` to obtain `total_files`.

2. **File Table Lookup Algorithm:**
   * Base Address = `0x0100`.
   * Loop `i` from `0` to `total_files - 1`:
     * Target Entry Base = `0x0100 + (i * 32)`.
     * Verify entry magic at `Target Entry Base + 0x00` equals `0x46545346` ("FSFT").
     * Compare `file_type` at `Target Entry Base + 0x04` or match string at `Target Entry Base + 0x08`.
     * Read `start_page` at `Target Entry Base + 0x10` and `page_count` at `Target Entry Base + 0x14`.

3. **Data Address Calculation:**
   * `RAM Base Load Address = start_page * 256`
   * `File Size in Bytes = page_count * 256`