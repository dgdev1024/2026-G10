# Specification: G10 Executable Program File Format

Below is a formal specification for the G10 Executable Program File Format (`.g10`),
which is the final output of the G10 toolchain, produced by the G10 Linker Tool
(`g10-link`). This file format is intended to be executed by a virtual machine
powered by the G10 CPU architecture.

## Overview

The G10 Executable Program File Format is a fully-linked, ready-to-execute binary
format that contains all the machine code and data needed to run a program on a
G10-powered virtual machine. Unlike the relocatable object file format (`.g10obj`),
executable files have all symbols resolved and all relocations applied.

### Purpose

The executable format serves as the bridge between the toolchain and the virtual
machine:

- **Final Output**: The result of linking one or more object files
- **Self-Contained**: All code and initialized data included
- **VM-Ready**: Can be loaded directly into memory and executed
- **Memory-Mapped**: Defines how to populate the G10 memory space

### Design Principles

1. **Simplicity**: Easy to parse and load into memory
2. **Little-Endian**: Consistent with G10 CPU architecture
3. **Self-Describing**: All sizes and addresses are explicitly stored
4. **Memory-Mapped**: Segments specify exact load addresses
5. **Minimal Overhead**: No unnecessary metadata for execution

## File Structure

A G10 executable file consists of the following sections in order:

```
┌─────────────────────────────────────────┐
│         File Header (64 bytes)          │
├─────────────────────────────────────────┤
│         Program Segment Table           │
│     (variable, based on segment_count)  │
├─────────────────────────────────────────┤
│         Segment Data                    │
│     (variable, per segment file_sizes)  │
└─────────────────────────────────────────┘
```

### Key Differences from Object Files

| Aspect | Object File (`.g10obj`) | Executable (`.g10`) |
|--------|-------------------------|---------------------|
| Purpose | Intermediate (linkable) | Final (executable) |
| Symbols | Included (for linking) | Optional (for debugging) |
| Relocations | Present (unresolved) | Applied (resolved) |
| Addresses | Base addresses per section | Final load addresses |
| Magic | `G10A` (0x47313041) | `G10X` (0x47313058) |

## File Header

The file header is exactly 64 bytes and contains metadata about the executable:

| Offset | Size | Field | Description |
|--------|------|-------|-------------|
| 0x00 | 4 | `magic` | Magic number: `0x47313058` ("G10X" in ASCII, little-endian) |
| 0x04 | 2 | `version` | Executable format version (currently `0x0001`) |
| 0x06 | 2 | `flags` | File flags (see below) |
| 0x08 | 4 | `entry_point` | Address where execution begins |
| 0x0C | 4 | `stack_pointer` | Initial stack pointer value |
| 0x10 | 2 | `segment_count` | Number of program segments |
| 0x12 | 2 | `reserved1` | Reserved (must be 0) |
| 0x14 | 4 | `total_file_size` | Total size of this file in bytes |
| 0x18 | 4 | `total_memory_size` | Total memory required (including BSS) |
| 0x1C | 4 | `checksum` | CRC-32 checksum of file contents (0 = not computed) |
| 0x20 | 32 | `program_name` | Null-terminated program name (max 31 chars + null) |

### Magic Number

The magic number `0x47313058` represents the ASCII string "G10X" (G10 eXecutable)
stored in little-endian format:

- Byte 0: `0x58` = 'X'
- Byte 1: `0x30` = '0'
- Byte 2: `0x31` = '1'
- Byte 3: `0x47` = 'G'

### File Flags

| Bit | Name | Description |
|-----|------|-------------|
| 0 | `FLAG_HAS_SYMBOLS` | Optional symbol table appended for debugging |
| 1 | `FLAG_COMPRESSED` | Segment data is compressed (future use) |
| 2 | `FLAG_SIGNED` | File includes a cryptographic signature (future use) |
| 3-15 | Reserved | Reserved for future use (must be 0) |

### Entry Point

The `entry_point` field specifies the memory address where the virtual machine
should begin execution. This is typically the address of the `main` label or
a designated entry point symbol from the source files.

The entry point must be within a valid ROM region (`$00002000` - `$7FFFFFFF`).

### Stack Pointer Initialization

The `stack_pointer` field specifies the initial value for the SP register when
the program starts. This should point to a valid location in the RAM region
(`$80000000` - `$FFFFFFFF`).

A typical value might be `$FFFFFFFC` (top of the 32-bit address space, aligned
to 4 bytes), or a more conservative value if the VM has limited RAM.

If this field is 0, the VM should use a default stack pointer value.

### Program Name

The `program_name` field is a null-terminated string of up to 31 characters
that identifies the program. This is useful for VM status displays and debugging.
If not specified, this field should contain all zeros.

## Program Segment Table

The program segment table immediately follows the file header. Each segment entry
is 24 bytes:

| Offset | Size | Field | Description |
|--------|------|-------|-------------|
| 0x00 | 4 | `load_address` | Memory address where segment should be loaded |
| 0x04 | 4 | `memory_size` | Size in memory (may be larger than file_size for BSS) |
| 0x08 | 4 | `file_size` | Size of segment data in the file (0 for BSS) |
| 0x0C | 4 | `file_offset` | Offset in file where segment data begins |
| 0x10 | 2 | `flags` | Segment flags (see below) |
| 0x12 | 2 | `alignment` | Required alignment (power of 2, minimum 1) |
| 0x14 | 4 | `reserved` | Reserved (must be 0) |

### Segment Flags

| Bit | Name | Description |
|-----|------|-------------|
| 0 | `SEG_READ` | Segment is readable |
| 1 | `SEG_WRITE` | Segment is writable |
| 2 | `SEG_EXEC` | Segment is executable |
| 3 | `SEG_ZERO` | Segment should be zero-initialized (BSS) |
| 4-15 | Reserved | Reserved for future use |

### Segment Types

Based on the flags and load address, segments fall into several categories:

| Type | Flags | Address Range | Description |
|------|-------|---------------|-------------|
| Code | READ \| EXEC | `$00002000` - `$7FFFFFFF` | Executable code |
| ROData | READ | `$00002000` - `$7FFFFFFF` | Read-only data |
| Data | READ \| WRITE | `$80000000` - `$FFFFFFFF` | Initialized data |
| BSS | READ \| WRITE \| ZERO | `$80000000` - `$FFFFFFFF` | Uninitialized data |

### Memory Layout Alignment

Segments should align with the G10 memory layout:

| Memory Region | Address Range | Typical Use |
|---------------|---------------|-------------|
| Metadata | `$00000000` - `$00000FFF` | Program info (populated by VM) |
| Interrupt Table | `$00001000` - `$00001FFF` | Interrupt handlers |
| ROM | `$00002000` - `$7FFFFFFF` | Code and constants |
| RAM | `$80000000` - `$FFFFFFFF` | Variables and stack |

### BSS Segments

BSS (Block Started by Symbol) segments represent uninitialized data that should
be zero-filled at load time. These segments have:

- `file_size = 0` (no data in the file)
- `memory_size > 0` (space to allocate)
- `SEG_ZERO` flag set

This saves file space for large uninitialized buffers.

## Segment Data

The segment data section contains the raw bytes for each loadable segment,
concatenated in the order they appear in the segment table. Only segments with
`file_size > 0` contribute data to this section.

```
┌────────────────────────────────────────────────────┐
│  Segment 0 Data (segment[0].file_size bytes)       │
├────────────────────────────────────────────────────┤
│  Segment 1 Data (segment[1].file_size bytes)       │
├────────────────────────────────────────────────────┤
│  ...                                               │
├────────────────────────────────────────────────────┤
│  Segment N Data (segment[N].file_size bytes)       │
└────────────────────────────────────────────────────┘
```

### Loading Segments

When the VM loads an executable:

1. **Validate Header**: Check magic number, version, and sizes
2. **Allocate Memory**: Reserve memory for all segments
3. **Load Segments**: For each segment:
   - If `file_size > 0`: Copy data from file to `load_address`
   - If `SEG_ZERO` set: Zero-fill from `file_size` to `memory_size`
4. **Initialize CPU**: Set PC to `entry_point`, SP to `stack_pointer`
5. **Begin Execution**: Start the CPU fetch-decode-execute cycle

## Complete File Layout Example

For a program with two segments (code at $2000, BSS at $80000000):

```
File Header (64 bytes)
├── magic: 0x47313058 ("G10X")
├── version: 0x0001
├── flags: 0x0000
├── entry_point: 0x00002000
├── stack_pointer: 0xFFFFFFFC
├── segment_count: 2
├── total_file_size: 1136
├── total_memory_size: 2048
├── checksum: 0x00000000
└── program_name: "hello_world\0"

Segment Table (2 × 24 = 48 bytes)
├── Segment 0 (Code):
│   ├── load_address: 0x00002000
│   ├── memory_size: 1024
│   ├── file_size: 1024
│   ├── file_offset: 112
│   ├── flags: SEG_READ | SEG_EXEC (0x0005)
│   └── alignment: 4
└── Segment 1 (BSS):
    ├── load_address: 0x80000000
    ├── memory_size: 1024
    ├── file_size: 0
    ├── file_offset: 0
    ├── flags: SEG_READ | SEG_WRITE | SEG_ZERO (0x000B)
    └── alignment: 4

Segment Data (1024 bytes)
└── Segment 0 code data (1024 bytes at file offset 112)
```

## C/C++ Structure Definitions

The following C++ structures can be used to read/write G10 executable files:

```cpp
#include <cstdint>

namespace g10exe
{
    // Magic number for executable files
    constexpr std::uint32_t MAGIC = 0x47313058;  // "G10X" little-endian
    
    // Current format version
    constexpr std::uint16_t VERSION = 0x0001;
    
    // File flags
    enum file_flags : std::uint16_t
    {
        FLAG_HAS_SYMBOLS = 0x0001,  // Optional symbol table appended
        FLAG_COMPRESSED  = 0x0002,  // Segment data is compressed
        FLAG_SIGNED      = 0x0004,  // File includes signature
    };
    
    // Segment flags
    enum segment_flags : std::uint16_t
    {
        SEG_READ  = 0x0001,  // Segment is readable
        SEG_WRITE = 0x0002,  // Segment is writable
        SEG_EXEC  = 0x0004,  // Segment is executable
        SEG_ZERO  = 0x0008,  // Zero-initialize (BSS)
    };
    
    // File header (64 bytes)
    struct file_header
    {
        std::uint32_t magic;              // 0x00: Magic number (0x47313058)
        std::uint16_t version;            // 0x04: Format version
        std::uint16_t flags;              // 0x06: File flags
        std::uint32_t entry_point;        // 0x08: Entry point address
        std::uint32_t stack_pointer;      // 0x0C: Initial stack pointer
        std::uint16_t segment_count;      // 0x10: Number of segments
        std::uint16_t reserved1;          // 0x12: Reserved
        std::uint32_t total_file_size;    // 0x14: Total file size
        std::uint32_t total_memory_size;  // 0x18: Total memory required
        std::uint32_t checksum;           // 0x1C: CRC-32 checksum
        char          program_name[32];   // 0x20: Program name
    };
    static_assert(sizeof(file_header) == 64);
    
    // Segment entry (24 bytes)
    struct segment_entry
    {
        std::uint32_t load_address;  // 0x00: Memory load address
        std::uint32_t memory_size;   // 0x04: Size in memory
        std::uint32_t file_size;     // 0x08: Size in file
        std::uint32_t file_offset;   // 0x0C: Offset in file
        std::uint16_t flags;         // 0x10: Segment flags
        std::uint16_t alignment;     // 0x12: Alignment requirement
        std::uint32_t reserved;      // 0x14: Reserved
    };
    static_assert(sizeof(segment_entry) == 24);
}
```

## Validation Rules

When reading a G10 executable file, the following validations should be performed:

### Header Validation

1. Magic number must be `0x47313058`
2. Version must be supported (currently only `0x0001`)
3. `entry_point` must be within ROM range (`$00002000` - `$7FFFFFFF`)
4. `stack_pointer` must be 0 or within RAM range (`$80000000` - `$FFFFFFFF`)
5. `segment_count` must be at least 1
6. `total_file_size` must match actual file size
7. If `checksum != 0`, verify CRC-32 matches

### Segment Validation

1. `load_address` must be within valid memory ranges
2. `file_offset + file_size` must not exceed `total_file_size`
3. `memory_size >= file_size`
4. Segments must not overlap in memory
5. If `SEG_WRITE` is set, `load_address` should be in RAM (`>= $80000000`)
6. If `SEG_EXEC` is set, `load_address` should be in ROM (`< $80000000`)
7. Alignment must be a power of 2

### Address Range Validation

| Segment Type | Valid Address Range | Flags Required |
|--------------|---------------------|----------------|
| Code | `$00002000` - `$7FFFFFFF` | SEG_READ, SEG_EXEC |
| Data/BSS | `$80000000` - `$FFFFFFFF` | SEG_READ, SEG_WRITE |
| Interrupt | `$00001000` - `$00001FFF` | SEG_READ, SEG_EXEC |

## Error Codes

The following error conditions may occur when loading executable files:

| Code | Name | Description |
|------|------|-------------|
| 0x01 | `ERR_INVALID_MAGIC` | Magic number mismatch |
| 0x02 | `ERR_UNSUPPORTED_VERSION` | Unsupported format version |
| 0x03 | `ERR_TRUNCATED_FILE` | File is shorter than expected |
| 0x04 | `ERR_INVALID_ENTRY` | Entry point not in valid range |
| 0x05 | `ERR_INVALID_SEGMENT` | Invalid segment entry |
| 0x06 | `ERR_SEGMENT_OVERLAP` | Segments overlap in memory |
| 0x07 | `ERR_CHECKSUM_MISMATCH` | CRC-32 verification failed |
| 0x08 | `ERR_OUT_OF_MEMORY` | Insufficient memory for segments |

## Linker Output Process

The linker produces an executable by:

1. **Reading Object Files**: Parse all input `.g10obj` files
2. **Symbol Resolution**: Match external references to definitions
3. **Section Merging**: Combine sections with matching addresses
4. **Relocation**: Apply all relocation fixups
5. **Segment Creation**: Create program segments from sections
6. **Entry Point Selection**: Determine the program entry point
7. **Header Generation**: Write the executable header
8. **File Output**: Write the complete `.g10` file

### Section to Segment Mapping

| Object Section Flags | Segment Type | Segment Flags |
|----------------------|--------------|---------------|
| SECT_EXECUTABLE \| SECT_INITIALIZED | Code | SEG_READ \| SEG_EXEC |
| SECT_INITIALIZED (ROM) | ROData | SEG_READ |
| SECT_WRITABLE \| SECT_INITIALIZED | Data | SEG_READ \| SEG_WRITE |
| SECT_WRITABLE \| SECT_ZERO | BSS | SEG_READ \| SEG_WRITE \| SEG_ZERO |

## Version History

| Version | Description |
|---------|-------------|
| 0x0001 | Initial version - basic segments, entry point, stack pointer |

## Future Considerations

The following features may be added in future versions:

1. **Symbol Table**: Optional debug symbols for debuggers
2. **Compression**: LZ4 or similar compression for segment data
3. **Signatures**: Cryptographic signatures for authenticity
4. **Resources**: Embedded data resources (images, sounds, etc.)
5. **Shared Libraries**: Dynamic linking support
6. **Thread Support**: Multiple entry points for threads
7. **Memory Protection**: Fine-grained segment permissions