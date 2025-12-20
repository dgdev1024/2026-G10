# Specification: G10 Linker Object File Format

Below is a formal specification for the G10 Linker Object File Format (`.g10obj`), 
which is output by the G10 CPU Assembler (`g10-asm`), then processed by the G10 Linker
(`g10-link`) in order to produce the final executable binary (`.g10`).

## Overview

The G10 Object File Format is a relocatable binary format designed to represent
assembled G10 machine code along with symbol information and relocation data.
This format enables:

- **Separate Compilation**: Source files can be assembled independently
- **Symbol Resolution**: Labels can reference symbols defined in other object files
- **Relocatable Code**: Code sections can be placed at different addresses by the linker
- **Multiple Sections**: Support for code at different origin addresses

### Design Principles

1. **Simplicity**: Easy to parse and generate
2. **Little-Endian**: Consistent with G10 CPU architecture
3. **Self-Describing**: All sizes are explicitly stored
4. **Extensible**: Version field allows for future enhancements

## File Structure

A G10 object file consists of the following sections in order:

```
┌─────────────────────────────────────────┐
│           File Header (32 bytes)        │
├─────────────────────────────────────────┤
│           Section Table                 │
│     (variable, based on section_count)  │
├─────────────────────────────────────────┤
│           Symbol Table                  │
│     (variable, based on symbol_count)   │
├─────────────────────────────────────────┤
│           Relocation Table              │
│   (variable, based on relocation_count) │
├─────────────────────────────────────────┤
│           String Table                  │
│     (variable, based on string_size)    │
├─────────────────────────────────────────┤
│           Code Data                     │
│       (variable, per section sizes)     │
└─────────────────────────────────────────┘
```

## File Header

The file header is exactly 32 bytes and contains metadata about the object file:

| Offset | Size | Field | Description |
|--------|------|-------|-------------|
| 0x00 | 4 | `magic` | Magic number: `0x47313041` ("G10A" in ASCII, little-endian) |
| 0x04 | 2 | `version` | Object file format version (currently `0x0001`) |
| 0x06 | 2 | `flags` | File flags (see below) |
| 0x08 | 2 | `section_count` | Number of code sections |
| 0x0A | 2 | `symbol_count` | Number of symbols in symbol table |
| 0x0C | 4 | `relocation_count` | Number of relocation entries |
| 0x10 | 4 | `string_table_size` | Size of string table in bytes |
| 0x14 | 4 | `code_size` | Total size of all code sections combined |
| 0x18 | 4 | `source_name_offset` | Offset into string table for source filename |
| 0x1C | 4 | `reserved` | Reserved for future use (must be 0) |

### File Flags

| Bit | Name | Description |
|-----|------|-------------|
| 0 | `FLAG_HAS_ENTRY` | File defines an entry point symbol |
| 1 | `FLAG_DEBUG` | Debug information is included |
| 2-15 | Reserved | Reserved for future use (must be 0) |

### Magic Number Verification

The magic number `0x47313041` represents the ASCII string "G10A" (G10 Assembler)
stored in little-endian format:

- Byte 0: `0x41` = 'A'
- Byte 1: `0x30` = '0'
- Byte 2: `0x31` = '1'
- Byte 3: `0x47` = 'G'

## Section Table

The section table immediately follows the file header. Each section entry is 
16 bytes:

| Offset | Size | Field | Description |
|--------|------|-------|-------------|
| 0x00 | 4 | `base_address` | Base address specified by `.ORG` directive |
| 0x04 | 4 | `size` | Size of this section's code in bytes |
| 0x08 | 4 | `offset` | Offset into code data where this section's code begins |
| 0x0C | 2 | `flags` | Section flags (see below) |
| 0x0E | 2 | `alignment` | Required alignment (power of 2, minimum 1) |

### Section Flags

| Bit | Name | Description |
|-----|------|-------------|
| 0 | `SECT_EXECUTABLE` | Section contains executable code |
| 1 | `SECT_WRITABLE` | Section is writable (RAM region) |
| 2 | `SECT_INITIALIZED` | Section contains initialized data |
| 3 | `SECT_ZERO` | Section is zero-initialized (BSS) |
| 4-15 | Reserved | Reserved for future use |

### Section Ordering

Sections should appear in the section table in order of their base addresses.
If multiple sections have the same base address (which would be an error in
most cases), the assembler should report an error.

## Symbol Table

The symbol table follows the section table. Each symbol entry is 16 bytes:

| Offset | Size | Field | Description |
|--------|------|-------|-------------|
| 0x00 | 4 | `name_offset` | Offset into string table for symbol name |
| 0x04 | 4 | `value` | Symbol value (address for labels, 0 for extern) |
| 0x08 | 2 | `section_index` | Index of section containing symbol (0xFFFF for extern) |
| 0x0A | 1 | `type` | Symbol type (see below) |
| 0x0B | 1 | `binding` | Symbol binding (local, global, extern) |
| 0x0C | 4 | `size` | Size of symbol (0 for labels, size for data) |

### Symbol Types

| Value | Name | Description |
|-------|------|-------------|
| 0x00 | `SYM_UNDEFINED` | Undefined symbol (forward reference) |
| 0x01 | `SYM_LABEL` | Code label (instruction address) |
| 0x02 | `SYM_DATA` | Data label (data address) |
| 0x03 | `SYM_CONSTANT` | Constant value (not an address) |
| 0x04 | `SYM_SECTION` | Section name |

### Symbol Binding

| Value | Name | Description |
|-------|------|-------------|
| 0x00 | `BIND_LOCAL` | Local symbol (not visible outside this file) |
| 0x01 | `BIND_GLOBAL` | Global symbol (exported, defined here) |
| 0x02 | `BIND_EXTERN` | External symbol (imported, defined elsewhere) |
| 0x03 | `BIND_WEAK` | Weak symbol (can be overridden by strong symbol) |

### Symbol Table Ordering

Symbols should appear in the following order for efficient processing:

1. Local symbols (binding = `BIND_LOCAL`)
2. Global symbols (binding = `BIND_GLOBAL`)
3. External symbols (binding = `BIND_EXTERN`)

## Relocation Table

The relocation table follows the symbol table. Each relocation entry is 16 bytes:

| Offset | Size | Field | Description |
|--------|------|-------|-------------|
| 0x00 | 4 | `offset` | Offset within the section where relocation is needed |
| 0x04 | 2 | `section_index` | Index of section containing the relocation site |
| 0x06 | 2 | `symbol_index` | Index of symbol to resolve |
| 0x08 | 4 | `addend` | Signed value to add after symbol resolution |
| 0x0C | 1 | `type` | Relocation type (see below) |
| 0x0D | 3 | `reserved` | Reserved (must be 0) |

### Relocation Types

| Value | Name | Size | Description |
|-------|------|------|-------------|
| 0x00 | `REL_NONE` | 0 | No relocation (placeholder) |
| 0x01 | `REL_ABS32` | 4 | 32-bit absolute address |
| 0x02 | `REL_ABS16` | 2 | 16-bit absolute address (truncated) |
| 0x03 | `REL_ABS8` | 1 | 8-bit absolute address (truncated) |
| 0x04 | `REL_REL32` | 4 | 32-bit PC-relative offset |
| 0x05 | `REL_REL16` | 2 | 16-bit PC-relative offset |
| 0x06 | `REL_REL8` | 1 | 8-bit PC-relative offset (for JPB/JR) |
| 0x07 | `REL_HI16` | 2 | High 16 bits of 32-bit address |
| 0x08 | `REL_LO16` | 2 | Low 16 bits of 32-bit address |

### Relocation Calculation

When the linker processes a relocation entry, it calculates the final value
as follows:

- **Absolute (`REL_ABS*`)**: `final_value = symbol_value + addend`
- **Relative (`REL_REL*`)**: `final_value = symbol_value + addend - relocation_address`
- **High/Low**: Extract the appropriate 16 bits from the 32-bit result

The calculated value is then written to the code at the specified offset,
using little-endian byte order.

## String Table

The string table is a contiguous block of null-terminated UTF-8 strings.
All string references in the file (symbol names, source filename) are 
stored as 32-bit offsets into this table.

```
Offset 0:    "main\0"           (5 bytes)
Offset 5:    "helper\0"         (7 bytes)
Offset 12:   "external_func\0"  (14 bytes)
Offset 26:   "myfile.asm\0"     (11 bytes)
...
```

### String Table Rules

1. The first byte (offset 0) should be a null byte (`\0`) representing an empty string
2. All strings must be null-terminated
3. String offsets must be valid (within `string_table_size`)
4. Duplicate strings may share the same offset (string pooling is allowed)

## Code Data

The code data section contains the raw machine code bytes for all sections,
concatenated in section order. Each section's code starts at its specified
`offset` within this block and extends for `size` bytes.

```
┌────────────────────────────────────────────────────┐
│  Section 0 Code (section[0].size bytes)           │
├────────────────────────────────────────────────────┤
│  Section 1 Code (section[1].size bytes)           │
├────────────────────────────────────────────────────┤
│  ...                                               │
├────────────────────────────────────────────────────┤
│  Section N Code (section[N].size bytes)           │
└────────────────────────────────────────────────────┘
```

### Code Data Integrity

- The sum of all section sizes must equal `header.code_size`
- Each section's `offset + size` must not exceed `header.code_size`
- Sections must not overlap in the code data block

## Complete File Layout Example

For a simple assembly file with two sections and three symbols:

```
File Header (32 bytes)
├── magic: 0x47313041
├── version: 0x0001
├── section_count: 2
├── symbol_count: 3
├── relocation_count: 1
├── string_table_size: 32
└── code_size: 64

Section Table (2 × 16 = 32 bytes)
├── Section 0: base=0x2000, size=48, offset=0
└── Section 1: base=0x8000, size=16, offset=48

Symbol Table (3 × 16 = 48 bytes)
├── Symbol 0: "main", addr=0x2000, global
├── Symbol 1: "data", addr=0x8000, global
└── Symbol 2: "external_func", extern

Relocation Table (1 × 16 = 16 bytes)
└── Reloc 0: offset=10, symbol=2, type=REL_ABS32

String Table (32 bytes)
├── Offset 0: ""
├── Offset 1: "main"
├── Offset 6: "data"
├── Offset 11: "external_func"
└── Offset 25: "test.asm"

Code Data (64 bytes)
├── Section 0 code (48 bytes at offset 0)
└── Section 1 code (16 bytes at offset 48)
```

## C/C++ Structure Definitions

The following C++ structures can be used to read/write G10 object files:

```cpp
#include <cstdint>

namespace g10obj
{
    // File magic number
    constexpr std::uint32_t MAGIC = 0x47313041;  // "G10A"
    
    // Current format version
    constexpr std::uint16_t VERSION = 0x0001;
    
    // File flags
    enum file_flags : std::uint16_t
    {
        FLAG_HAS_ENTRY  = 0x0001,
        FLAG_DEBUG      = 0x0002,
    };
    
    // Section flags
    enum section_flags : std::uint16_t
    {
        SECT_EXECUTABLE  = 0x0001,
        SECT_WRITABLE    = 0x0002,
        SECT_INITIALIZED = 0x0004,
        SECT_ZERO        = 0x0008,
    };
    
    // Symbol types
    enum symbol_type : std::uint8_t
    {
        SYM_UNDEFINED = 0x00,
        SYM_LABEL     = 0x01,
        SYM_DATA      = 0x02,
        SYM_CONSTANT  = 0x03,
        SYM_SECTION   = 0x04,
    };
    
    // Symbol binding
    enum symbol_binding : std::uint8_t
    {
        BIND_LOCAL  = 0x00,
        BIND_GLOBAL = 0x01,
        BIND_EXTERN = 0x02,
        BIND_WEAK   = 0x03,
    };
    
    // Relocation types
    enum relocation_type : std::uint8_t
    {
        REL_NONE  = 0x00,
        REL_ABS32 = 0x01,
        REL_ABS16 = 0x02,
        REL_ABS8  = 0x03,
        REL_REL32 = 0x04,
        REL_REL16 = 0x05,
        REL_REL8  = 0x06,
        REL_HI16  = 0x07,
        REL_LO16  = 0x08,
    };
    
    // File header (32 bytes)
    struct file_header
    {
        std::uint32_t magic;              // 0x00: Magic number
        std::uint16_t version;            // 0x04: Format version
        std::uint16_t flags;              // 0x06: File flags
        std::uint16_t section_count;      // 0x08: Number of sections
        std::uint16_t symbol_count;       // 0x0A: Number of symbols
        std::uint32_t relocation_count;   // 0x0C: Number of relocations
        std::uint32_t string_table_size;  // 0x10: String table size
        std::uint32_t code_size;          // 0x14: Total code size
        std::uint32_t source_name_offset; // 0x18: Source filename offset
        std::uint32_t reserved;           // 0x1C: Reserved
    };
    static_assert(sizeof(file_header) == 32);
    
    // Section entry (16 bytes)
    struct section_entry
    {
        std::uint32_t base_address;       // 0x00: Base address
        std::uint32_t size;               // 0x04: Section size
        std::uint32_t offset;             // 0x08: Offset in code data
        std::uint16_t flags;              // 0x0C: Section flags
        std::uint16_t alignment;          // 0x0E: Alignment
    };
    static_assert(sizeof(section_entry) == 16);
    
    // Symbol entry (16 bytes)
    struct symbol_entry
    {
        std::uint32_t name_offset;        // 0x00: Name in string table
        std::uint32_t value;              // 0x04: Symbol value
        std::uint16_t section_index;      // 0x08: Section index
        std::uint8_t  type;               // 0x0A: Symbol type
        std::uint8_t  binding;            // 0x0B: Symbol binding
        std::uint32_t size;               // 0x0C: Symbol size
    };
    static_assert(sizeof(symbol_entry) == 16);
    
    // Relocation entry (16 bytes)
    struct relocation_entry
    {
        std::uint32_t offset;             // 0x00: Offset in section
        std::uint16_t section_index;      // 0x04: Section index
        std::uint16_t symbol_index;       // 0x06: Symbol index
        std::int32_t  addend;             // 0x08: Addend value
        std::uint8_t  type;               // 0x0C: Relocation type
        std::uint8_t  reserved[3];        // 0x0D: Reserved
    };
    static_assert(sizeof(relocation_entry) == 16);
}
```

## Validation Rules

When reading a G10 object file, the following validations should be performed:

### Header Validation

1. Magic number must be `0x47313041`
2. Version must be supported (currently only `0x0001`)
3. Reserved field must be `0`
4. `section_count`, `symbol_count`, `relocation_count` must be reasonable
5. `code_size` must equal sum of all section sizes

### Section Validation

1. Section offsets must not overlap
2. Section `offset + size` must not exceed `code_size`
3. Base addresses should be within valid memory ranges
4. Writable sections should have base address ≥ `0x80000000`

### Symbol Validation

1. Name offsets must be within string table bounds
2. Section indices must be valid or `0xFFFF` for extern
3. Global/extern symbol names must not be empty
4. Duplicate global symbol names within a file are an error

### Relocation Validation

1. Section indices must be valid
2. Symbol indices must be valid
3. Relocation offsets must be within section bounds
4. Relocation type must be valid

## Error Codes

The following error conditions may occur when processing object files:

| Code | Name | Description |
|------|------|-------------|
| 0x01 | `ERR_INVALID_MAGIC` | Magic number mismatch |
| 0x02 | `ERR_UNSUPPORTED_VERSION` | Unsupported format version |
| 0x03 | `ERR_TRUNCATED_FILE` | File is shorter than expected |
| 0x04 | `ERR_INVALID_SECTION` | Invalid section entry |
| 0x05 | `ERR_INVALID_SYMBOL` | Invalid symbol entry |
| 0x06 | `ERR_INVALID_RELOCATION` | Invalid relocation entry |
| 0x07 | `ERR_STRING_OUT_OF_BOUNDS` | String offset exceeds table |
| 0x08 | `ERR_DUPLICATE_SYMBOL` | Duplicate global symbol |
| 0x09 | `ERR_UNDEFINED_SYMBOL` | Referenced symbol not found |
| 0x0A | `ERR_SECTION_OVERLAP` | Sections overlap in code data |

## Version History

| Version | Description |
|---------|-------------|
| 0x0001 | Initial version - basic sections, symbols, relocations |

## Future Considerations

The following features may be added in future versions:

1. **Debug Information**: Line number mappings, variable locations
2. **Compression**: Optional compression for code data
3. **Checksums**: CRC or hash for integrity verification
4. **Archives**: Library format containing multiple object files
5. **Weak Symbols**: Symbols that can be overridden
6. **Thread-Local Storage**: Support for TLS sections
7. **Exception Handling**: Unwind information for exceptions

