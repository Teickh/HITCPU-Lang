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