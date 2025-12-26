# Specification: G10 Object File Format

Below is a specification for the G10 CPU object file format. This format is used
for G10 object files, output by the G10 assembler tool (`g10asm`) and input and
processed by the G10 linker tool (`g10link`) in order to produce the final
binary file, which can be executed on G10 CPU-powered virtual machines.

## Details

- **File Extensions**: `.g10obj`, `.g10o`, `.obj`, `.o`
    - This is not strictly enforced; files with any (or no) extension can be
        used, just as long as they contain file data conformant to the G10
        object file format specification.
- **Endianness**: Little-endian
- **Magic Number**: `0x4731304F` (`'G10O'` in ASCII)
- **Version**: `0xMMmmPPPP` (4 bytes)
    - `MM` = Major version (1 byte)
    - `mm` = Minor version (1 byte)
    - `PPPP` = Patch version (2 bytes)

## File Structure Overview

The G10 object file is organized into a header followed by multiple sections.
Each section contains specific types of data needed by the linker to produce
the final executable binary. The file structure is designed to support:

- **Modular Assembly**: Code and data can be split across multiple source files.
- **Symbol Resolution**: Labels can be exported (global) and imported (extern).
- **Relocation**: Address references can be patched by the linker.
- **Separate ROM/RAM Handling**: Distinguishes between initialized data (ROM) 
    and uninitialized reservations (BSS in RAM).

### File Layout

| Offset (Bytes)    | Size (Bytes)      | Description                                       |
|-------------------|-------------------|---------------------------------------------------|
| `0x00000000`      | 4                 | Magic Number (`0x4731304F`)                       |
| `0x00000004`      | 4                 | Version Number                                    |
| `0x00000008`      | 4                 | Flags                                             |
| `0x0000000C`      | 4                 | Section Count                                     |
| `0x00000010`      | 4                 | Symbol Table Offset                               |
| `0x00000014`      | 4                 | Symbol Table Size (in entries)                    |
| `0x00000018`      | 4                 | String Table Offset                               |
| `0x0000001C`      | 4                 | String Table Size (in bytes)                      |
| `0x00000020`      | 4                 | Relocation Table Offset                           |
| `0x00000024`      | 4                 | Relocation Table Size (in entries)                |
| `0x00000028`      | 24                | Reserved (must be zero)                           |
| `0x00000040`      | Variable          | Section Headers (16 bytes each)                   |
| Variable          | Variable          | Section Data                                      |
| Variable          | Variable          | Symbol Table                                      |
| Variable          | Variable          | String Table                                      |
| Variable          | Variable          | Relocation Table                                  |

## Header Fields

This section describes each field in the file header in detail.

### Magic Number (Offset `0x00`, 4 bytes)

A 32-bit value used to identify the file as a valid G10 object file. The magic
number is `0x4731304F`, which corresponds to the ASCII string `'G10O'` when
read as little-endian bytes (`0x47` = 'G', `0x31` = '1', `0x30` = '0', 
`0x4F` = 'O').

### Version Number (Offset `0x04`, 4 bytes)

A 32-bit value representing the version of the object file format. The version
number is structured as follows:

| Bits      | Field         | Description                                       |
|-----------|---------------|---------------------------------------------------|
| 31-24     | Major         | Major version number (0-255)                      |
| 23-16     | Minor         | Minor version number (0-255)                      |
| 15-0      | Patch         | Patch version number (0-65535)                    |

Version compatibility rules:
- Files with different major versions are incompatible.
- Files with the same major version but different minor versions are 
    backward-compatible (newer linkers can read older minor versions).
- Patch versions indicate bug fixes and are always compatible.

### Flags (Offset `0x08`, 4 bytes)

A 32-bit bitfield containing file-level flags:

| Bit   | Name              | Description                                       |
|-------|-------------------|---------------------------------------------------|
| 0     | `HAS_ENTRY`       | File contains an entry point symbol               |
| 1     | `DEBUG_INFO`      | File contains debug information                   |
| 2     | `RELOCATABLE`     | File is relocatable (not absolute)                |
| 3-31  | Reserved          | Reserved for future use (must be zero)            |

### Section Count (Offset `0x0C`, 4 bytes)

A 32-bit unsigned integer indicating the number of section headers present in
the file. Section headers begin immediately after the fixed header at offset
`0x40`.

### Symbol Table Offset (Offset `0x10`, 4 bytes)

A 32-bit unsigned integer indicating the byte offset from the start of the file
to the beginning of the symbol table. If no symbols are present, this value
should be zero.

### Symbol Table Size (Offset `0x14`, 4 bytes)

A 32-bit unsigned integer indicating the number of entries in the symbol table.
Each symbol table entry is 16 bytes in size.

### String Table Offset (Offset `0x18`, 4 bytes)

A 32-bit unsigned integer indicating the byte offset from the start of the file
to the beginning of the string table. The string table contains null-terminated
strings referenced by symbol names and section names.

### String Table Size (Offset `0x1C`, 4 bytes)

A 32-bit unsigned integer indicating the size of the string table in bytes,
including all null terminators.

### Relocation Table Offset (Offset `0x20`, 4 bytes)

A 32-bit unsigned integer indicating the byte offset from the start of the file
to the beginning of the relocation table. If no relocations are present, this
value should be zero.

### Relocation Table Size (Offset `0x24`, 4 bytes)

A 32-bit unsigned integer indicating the number of entries in the relocation
table. Each relocation table entry is 16 bytes in size.

### Reserved (Offset `0x28`, 24 bytes)

Reserved space for future header extensions. All bytes in this region must be
set to zero. Linkers should ignore non-zero values in this region for forward
compatibility.

## Section Headers

Section headers describe the individual sections within the object file. Each
section represents a contiguous block of code or data that will be placed at a
specific address in the final binary. Section headers begin at offset `0x40` and
are stored contiguously.

### Section Header Layout

Each section header is 16 bytes in size and structured as follows:

| Offset    | Size  | Field             | Description                               |
|-----------|-------|-------------------|-------------------------------------------|
| `0x00`    | 4     | Name Offset       | Offset into string table for section name |
| `0x04`    | 4     | Virtual Address   | Target address in memory                  |
| `0x08`    | 4     | Size              | Size of section data in bytes             |
| `0x0C`    | 2     | Type              | Section type identifier                   |
| `0x0E`    | 2     | Flags             | Section attribute flags                   |

### Section Types

The section type field indicates the kind of data contained in the section:

| Value     | Name          | Description                                       |
|-----------|---------------|---------------------------------------------------|
| `0x0000`  | `NULL`        | Unused section header entry                       |
| `0x0001`  | `CODE`        | Executable code (ROM region)                      |
| `0x0002`  | `DATA`        | Initialized data (ROM region, read-only)          |
| `0x0003`  | `BSS`         | Uninitialized data reservation (RAM region)       |

The distinction between `CODE` and `DATA` allows linkers and debuggers to
apply appropriate handling. All initialized data in the ROM region is
inherently read-only at runtime. For the G10 architecture:
- `CODE` and `DATA` sections should have addresses in the ROM region 
    (`0x00000000` to `0x7FFFFFFF`).
- `BSS` sections should have addresses in the RAM region 
    (`0x80000000` to `0xFFFFFFFF`).

#### Aliases

- `RODATA` is an alias for `DATA` (initialized read-only data).
- `RWDATA` and `RADATA` are aliases for `BSS` (read-write data).

### Section Flags

The section flags field is a 16-bit bitfield describing section attributes:

| Bit   | Name          | Description                                       |
|-------|---------------|---------------------------------------------------|
| 0     | `ALLOC`       | Section occupies memory at runtime                |
| 1     | `LOAD`        | Section data should be loaded from the file       |
| 2     | `EXEC`        | Section contains executable code                  |
| 3     | `WRITE`       | Section is writable at runtime                    |
| 4     | `MERGE`       | Section can be merged with similar sections       |
| 5     | `STRINGS`     | Section contains null-terminated strings          |
| 6-15  | Reserved      | Reserved for future use (must be zero)            |

Common flag combinations:
- **Code Section**: `ALLOC | LOAD | EXEC` = `0x0007`
- **Data Section**: `ALLOC | LOAD` = `0x0003`
- **BSS Section**: `ALLOC | WRITE` = `0x0009`

### Section Data

For sections with the `LOAD` flag set (types `CODE` and `DATA`), the actual
section data follows the section headers. The data is stored contiguously in
the order of the section headers. Each section's data begins at the offset
calculated by summing:
1. The header offset (`0x40`)
2. The total size of all section headers (16 bytes × section count)
3. The cumulative size of all preceding section data

For `BSS` sections, no data is stored in the file; only the size field indicates
how many bytes should be reserved in RAM at runtime.

## Symbol Table

The symbol table contains entries for all symbols (labels) defined in or
referenced by the object file. Symbols are used by the linker to resolve
references between object files and to determine final addresses for labels.

### Symbol Table Entry Layout

Each symbol table entry is 16 bytes in size and structured as follows:

| Offset    | Size  | Field             | Description                               |
|-----------|-------|-------------------|-------------------------------------------|
| `0x00`    | 4     | Name Offset       | Offset into string table for symbol name  |
| `0x04`    | 4     | Value             | Symbol value (address or size)            |
| `0x08`    | 4     | Section Index     | Index of associated section (or special)  |
| `0x0C`    | 1     | Type              | Symbol type                               |
| `0x0D`    | 1     | Binding           | Symbol binding (scope)                    |
| `0x0E`    | 2     | Flags             | Additional symbol flags                   |

### Symbol Types

The symbol type field indicates what kind of entity the symbol represents:

| Value     | Name          | Description                                       |
|-----------|---------------|---------------------------------------------------|
| `0x00`    | `NONE`        | Unspecified type                                  |
| `0x01`    | `LABEL`       | Code label (jump/call target)                     |
| `0x02`    | `DATA`        | Data label (memory location)                      |
| `0x03`    | `SECTION`     | Section name symbol                               |
| `0x04`    | `FILE`        | Source file name                                  |

### Symbol Bindings

The symbol binding field indicates the symbol's visibility and scope:

| Value     | Name          | Description                                       |
|-----------|---------------|---------------------------------------------------|
| `0x00`    | `LOCAL`       | Symbol is local to this object file               |
| `0x01`    | `GLOBAL`      | Symbol is visible to other object files           |
| `0x02`    | `EXTERN`      | Symbol is defined in another object file          |
| `0x03`    | `WEAK`        | Symbol can be overridden by a global definition   |

The binding field corresponds to the assembler directives:
- **`.global`**: Marks a locally-defined symbol as `GLOBAL`, exporting it for
    use by other modules. Once defined globally, a symbol cannot be redefined
    within the same object file or in any other object file being linked with it.
- **`.extern`**: Declares a symbol as `EXTERN`, indicating it is defined in
    another module and must be resolved at link time. A symbol marked `.extern`
    cannot be defined (neither as `.global` nor as a local definition) within
    the same object file.

### Symbol Scoping Rules

The following rules govern how symbols can be declared and defined:

1. **Global Symbols**: A symbol marked with `.global` must be defined within the
    same object file. Once globally defined, it cannot be redefined (neither as
    a local redefinition within the same file nor as another global definition
    in a linked file).

2. **External Symbols**: A symbol marked with `.extern` must NOT be defined
    within the same object file. It is expected to be resolved by the linker
    from a global symbol defined in another object file.

3. **Mutually Exclusive Declarations**: A symbol cannot be marked with both
    `.global` and `.extern` directives within the same object file. These
    directives are mutually exclusive.

4. **Local Symbols**: Symbols not marked with `.global` or `.extern` are
    considered local symbols. Local symbols are visible only within their
    defining object file and can be redefined multiple times within the same
    object file (useful for labels in different code sections).
    - Although not required, it is a good practice to prefix local symbols
        with a dot (`.`) to indicate their local scope - just make sure the
        identifier being used is not also defined as a language keyword 
        (directive) or another global/extern symbol.

### Symbol Flags

The symbol flags field is a 16-bit bitfield for additional attributes:

| Bit   | Name          | Description                                       |
|-------|---------------|---------------------------------------------------|
| 0     | `ENTRY`       | Symbol is the program entry point                 |
| 1     | `ABSOLUTE`    | Symbol value is an absolute address (no reloc.)   |
| 2     | `COMMON`      | Symbol is a common block (tentative definition)   |
| 3-15  | Reserved      | Reserved for future use (must be zero)            |

### Special Section Indices

The section index field usually refers to a section header index (0-based).
However, certain special values have predefined meanings:

| Value         | Name          | Description                                   |
|---------------|---------------|-----------------------------------------------|
| `0xFFFFFFFF`  | `UNDEF`       | Symbol is undefined (external reference)      |
| `0xFFFFFFFE`  | `ABS`         | Symbol has an absolute value                  |
| `0xFFFFFFFD`  | `COMMON`      | Symbol is in a common block                   |

## String Table

The string table is a contiguous block of null-terminated strings used to store
symbol names, section names, and other textual data. Strings are referenced by
their byte offset from the start of the string table.

### String Table Format

The string table begins with a null byte (`0x00`) at offset 0, representing the
empty string. All other strings follow, each terminated by a null byte:

```
Offset 0x00:  0x00                          (empty string)
Offset 0x01:  'm' 'a' 'i' 'n' 0x00          ("main")
Offset 0x06:  'h' 'e' 'l' 'p' 'e' 'r' 0x00  ("helper")
Offset 0x0D:  ...
```

### String Table Rules

1. The first byte of the string table must be a null byte (`0x00`).
2. All strings are encoded as UTF-8 (ASCII-compatible for typical identifiers).
3. String references use the byte offset from the start of the string table.
4. Duplicate strings may share storage (string interning is permitted but not
    required).
5. The string table size in the header includes all bytes, including null
    terminators.

## Relocation Table

The relocation table contains entries that describe locations in the section
data that need to be modified by the linker. Relocations are necessary when
code or data references symbols whose final addresses are not known until
link time.

### When Relocations Are Needed

Relocations are generated by the assembler in the following situations:

1. **Label References in Instructions**: When an instruction uses a label as an
    operand (e.g., `jmp main`, `jpb nc, loop`, `call helper`), the assembler
    cannot know the final address if:
    - The label is defined in another module (`.extern`).
    - The final layout depends on other sections being linked together.

2. **Direct Memory Operands with Labels**: When a label is used in a direct
    memory operand (e.g., `ld d0, [data_start]`, `st [counter], w0`), the
    address must be patched by the linker.

3. **Data Directives with Label References**: When `.byte`, `.word`, or `.dword`
    directives contain label references or expressions involving labels.

### Relocation Entry Layout

Each relocation table entry is 16 bytes in size and structured as follows:

| Offset    | Size  | Field             | Description                               |
|-----------|-------|-------------------|-------------------------------------------|
| `0x00`    | 4     | Offset            | Byte offset within the section            |
| `0x04`    | 4     | Symbol Index      | Index into the symbol table               |
| `0x08`    | 4     | Section Index     | Index of the section containing the reloc |
| `0x0C`    | 2     | Type              | Relocation type                           |
| `0x0E`    | 2     | Addend Low        | Low 16 bits of addend (or zero)           |

Note: For relocations requiring a larger addend, the addend is stored in the
section data itself (at the relocation offset), and the linker adds the symbol
value to it.

### Relocation Types

The relocation type field specifies how the linker should calculate and apply
the relocation. Types are designed around the G10 CPU's instruction formats:

| Value     | Name              | Size  | Description                               |
|-----------|-------------------|-------|-------------------------------------------|
| `0x0000`  | `NONE`            | 0     | No relocation (placeholder)               |
| `0x0001`  | `ABS32`           | 4     | 32-bit absolute address                   |
| `0x0002`  | `ABS16`           | 2     | 16-bit absolute address (truncated)       |
| `0x0003`  | `ABS8`            | 1     | 8-bit absolute address (truncated)        |
| `0x0004`  | `REL32`           | 4     | 32-bit PC-relative offset                 |
| `0x0005`  | `REL16`           | 2     | 16-bit PC-relative offset                 |
| `0x0006`  | `REL8`            | 1     | 8-bit PC-relative offset                  |
| `0x0007`  | `QUICK16`         | 2     | 16-bit offset relative to `$FFFF0000`     |
| `0x0008`  | `PORT8`           | 1     | 8-bit offset relative to `$FFFFFF00`      |

### Relocation Calculations

The linker calculates the final value based on the relocation type:

- **`ABS32`, `ABS16`, `ABS8`**: 
    ```
    final_value = symbol_value + addend
    ```
    
- **`REL32`, `REL16`, `REL8`** (for `jpb`, relative jumps):
    ```
    final_value = (symbol_value + addend) - (relocation_address + instruction_size)
    ```
    The offset is from the end of the instruction to the target.

- **`QUICK16`** (for `ldq` ("Load Quick"), `stq` ("Store Quick") instructions):
    ```
    final_value = (symbol_value + addend) - 0xFFFF0000
    ```
    The resulting value must fit in 16 bits.

- **`PORT8`** (for `ldp` ("Load Port"), `stp` ("Store Port") instructions):
    ```
    final_value = (symbol_value + addend) - 0xFFFFFF00
    ```
    The resulting value must fit in 8 bits.

### Relocation Processing

When the linker processes relocations:

1. For each relocation entry, locate the target byte offset within the 
    specified section.
2. Look up the symbol by index to obtain its final resolved address.
3. Apply the relocation calculation based on the type.
4. Write the calculated value back to the section data, respecting the size
    and endianness (little-endian).

If a relocation cannot be satisfied (e.g., an undefined external symbol, or a
value that overflows the target size), the linker should report an error.

## Assembly Directives Mapping

This section describes how G10 assembler directives translate to object file
structures.

### `.org` Directive

The `.org` directive sets the assembler's location counter to a specific
address. This affects the virtual address field of subsequent section headers:

- When `.org` specifies an address in the ROM region (`0x00000000` to 
    `0x7FFFFFFF`), subsequent code and data are placed in a `CODE` or `DATA`
    section with the `LOAD` flag set.
- When `.org` specifies an address in the RAM region (`0x80000000` to 
    `0xFFFFFFFF`), subsequent data reservations create a `BSS` section without
    the `LOAD` flag.

The assembler may create multiple sections when `.org` is used to switch
between non-contiguous address ranges.

### `.byte`, `.word`, `.dword` Directives

These directives emit data or reserve space, depending on the current location:

**In ROM Region (address < `0x80000000`; bit 31 clear):**
- Data is emitted directly into the section data.
- Each value is stored in little-endian byte order.
- String arguments in `.byte` emit one byte per character (not null-terminated).
- If values contain label references, relocations are generated.

**In RAM Region (address >= `0x80000000`; bit 31 set):**
- No data is emitted; only the section size is increased.
- The argument specifies the count of units to reserve, not values.
- Multiple arguments are summed to determine total reservation.

| Directive     | Unit Size     | ROM Behavior          | RAM Behavior          |
|---------------|---------------|-----------------------|-----------------------|
| `.byte`       | 1 byte        | Emit byte values      | Reserve N bytes       |
| `.word`       | 2 bytes       | Emit word values      | Reserve N×2 bytes     |
| `.dword`      | 4 bytes       | Emit dword values     | Reserve N×4 bytes     |

### `.global` Directive

The `.global` directive marks one or more labels as globally visible and
exportable for use by other modules:

```asm
.global main, helper
```

This creates symbol table entries with:
- **Binding**: `GLOBAL` (`0x01`)
- **Type**: `LABEL` (`0x01`) or `DATA` (`0x02`) depending on usage
- **Value**: The address assigned to the label
- **Section Index**: The section where the label is defined

**Scoping Rules**:
- The symbol must be defined (as a label or data location) somewhere within
    the same object file.
- A globally-defined symbol cannot be redefined within the same object file.
- A globally-defined symbol cannot be redefined in any other object file linked
    with this one; attempting to do so is a linker error.
- A symbol cannot be marked both `.global` and `.extern`.

### `.extern` Directive

The `.extern` directive declares symbols that are defined in other modules and
must be resolved by the linker:

```asm
.extern printf, malloc
```

This creates symbol table entries with:
- **Binding**: `EXTERN` (`0x02`)
- **Type**: Typically `NONE` (`0x00`) until resolved by the linker
- **Value**: `0x00000000` (to be filled by the linker)
- **Section Index**: `UNDEF` (`0xFFFFFFFF`)

**Scoping Rules**:
- The symbol must NOT be defined (as a label, data location, or any other
    entity) within the same object file.
- References to external symbols within the object file generate relocation
    entries for the linker to resolve.
- A symbol cannot be marked both `.extern` and `.global`.

## Instruction Encoding and Relocations

This section describes how G10 CPU instructions with address operands interact
with the object file format.

### Instruction Lengths and Immediate Data

As specified in the G10 CPU specification, instructions have the following 
lengths based on their operands:

| Instruction Type                          | Total Length    | Immediate Size  |
|-------------------------------------------|-----------------|-----------------|
| No operands / register-only               | 2 bytes         | None            |
| 8-bit immediate operand                   | 3 bytes         | 1 byte          |
| 16-bit immediate operand                  | 4 bytes         | 2 bytes         |
| 32-bit immediate/address operand          | 6 bytes         | 4 bytes         |

### Branch and Jump Instructions

The G10 CPU supports several branching instructions that may require relocations:

**`jmp` (Jump)**: Unconditional or conditional jump to an absolute address.
- Total length: 6 bytes (2-byte opcode + 4-byte address)
- Relocation type: `ABS32` at offset +2 from instruction start

**`jpb` (Jump By / Jump Relative)**: PC-relative jump.
- Total length: 6 bytes (2-byte opcode + 4-byte signed offset)
- Relocation type: `REL32` at offset +2 from instruction start
- The offset is calculated from the end of the instruction

**`call` (Call Subroutine)**: Call to an absolute address.
- Total length: 6 bytes (2-byte opcode + 4-byte address)
- Relocation type: `ABS32` at offset +2 from instruction start

### Load and Store with Direct Memory

Instructions that access memory at a direct (absolute) address:

**`ld` with direct memory operand**: Load from memory.
```asm
ld d0, [label]      ; 6 bytes: opcode + 32-bit address
```
- Relocation type: `ABS32` at offset +2

**`st` with direct memory operand**: Store to memory.
```asm
st [label], d0      ; 6 bytes: opcode + 32-bit address
```
- Relocation type: `ABS32` at offset +2

### Quick Load/Store Instructions

The `ldq`/`stq` and `ldp`/`stp` instructions use relative addressing:

**`ldq`/`stq` (Quick RAM access)**: Address relative to `$FFFF0000`.
```asm
ldq w0, [0x1234]    ; 4 bytes: opcode + 16-bit offset
```
- Relocation type: `QUICK16` at offset +2
- Final address = `$FFFF0000` + offset
- Example resulting address: `$FFFF1234`

**`ldp`/`stp` (I/O Port access)**: Address relative to `$FFFFFF00`.
```asm
ldp l0, [0x40]      ; 3 bytes: opcode + 8-bit offset
```
- Relocation type: `PORT8` at offset +2
- Final address = `$FFFFFF00` + offset
- Example resulting address: `$FFFFFF40`

### Immediate Values with Label References

When an immediate value references a label:

```asm
ld d0, label        ; Load address of 'label' into d0
```

- The immediate field contains the 32-bit label address.
- Relocation type: `ABS32` at the immediate data offset.

### Relocation Example

Consider the following assembly:

```asm
.extern helper
.org 0x4000

main:
    ld d0, 0x100
    call helper         ; Needs relocation
    jpb nc, main        ; Needs relocation (PC-relative)
    ret nc
```

The assembler generates:

**Section Data** (at virtual address `0x4000`):
```
Offset  Bytes                   Disassembly
0x0000  XX XX 00 01 00 00       ld d0, 0x100
0x0006  XX XX 00 00 00 00       call 0x00000000  (placeholder)
0x000C  XX XX 00 00 00 00       jpb nc, 0x00000000  (placeholder)
0x0012  XX XX                   ret nc
```

**Relocation Entries**:

| Offset    | Symbol Index  | Section   | Type      | Notes                     |
|-----------|---------------|-----------|-----------|---------------------------|
| `0x0008`  | (helper)      | 0         | `ABS32`   | Absolute call target      |
| `0x000E`  | (main)        | 0         | `REL32`   | PC-relative back to main  |

## Example Object File

This section provides a complete example of a G10 object file structure.

### Source Assembly

```asm
; example.asm
.global main
.extern helper

.org 0x4000

main:
    ld d0, 0x42
    call helper
    jpb nc, main

.org 0x80000000

counter:
    .dword 1        ; Reserve 1 dword (4 bytes) in BSS
```

### Resulting Object File Structure

**File Header (64 bytes at offset 0x00):**

| Offset    | Value             | Field                         |
|-----------|-------------------|-------------------------------|
| 0x00      | `0x4731304F`      | Magic Number ("G10O")         |
| 0x04      | `0x01000000`      | Version 1.0.0                 |
| 0x08      | `0x00000000`      | Flags (none set)              |
| 0x0C      | `0x00000002`      | Section Count (2)             |
| 0x10      | `0x00000060`      | Symbol Table Offset           |
| 0x14      | `0x00000003`      | Symbol Table Size (3 entries) |
| 0x18      | `0x00000090`      | String Table Offset           |
| 0x1C      | `0x00000014`      | String Table Size (20 bytes)  |
| 0x20      | `0x000000A4`      | Relocation Table Offset       |
| 0x24      | `0x00000002`      | Relocation Table Size (2)     |
| 0x28      | (24 bytes of 0)   | Reserved                      |

**Section Headers (32 bytes at offset 0x40):**

Section 0 (CODE):

| Offset    | Value             | Field                         |
|-----------|-------------------|-------------------------------|
| 0x40      | `0x00000001`      | Name Offset ("main")          |
| 0x44      | `0x00004000`      | Virtual Address               |
| 0x48      | `0x00000012`      | Size (18 bytes)               |
| 0x4C      | `0x0001`          | Type (CODE)                   |
| 0x4E      | `0x0007`          | Flags (ALLOC|LOAD|EXEC)       |

Section 1 (BSS):

| Offset    | Value             | Field                         |
|-----------|-------------------|-------------------------------|
| 0x50      | `0x00000006`      | Name Offset ("counter")       |
| 0x54      | `0x80000000`      | Virtual Address               |
| 0x58      | `0x00000004`      | Size (4 bytes reserved)       |
| 0x5C      | `0x0003`          | Type (BSS)                    |
| 0x5E      | `0x0009`          | Flags (ALLOC|WRITE)           |

**Section Data (at offset 0x60, but code section first):**

Note: The section data immediately follows the headers. Only the CODE section
has actual data (18 bytes). The BSS section has no file data.

```
Offset 0x50 (relative to section): Section 0 data
  00: XX XX  42 00 00 00     ; ld d0, 0x42
  06: XX XX  00 00 00 00     ; call 0x00000000 (reloc: helper)
  0C: XX XX  00 00 00 00     ; jpb nc, 0x00000000 (reloc: main)
```

(Note: `XX XX` represents the instruction opcode bytes, which depend on the
specific encoding defined in the G10 CPU specification.)

**Symbol Table (48 bytes at offset 0x60):**

Entry 0 (main):

| Offset    | Value             | Field                         |
|-----------|-------------------|-------------------------------|
| 0x60      | `0x00000001`      | Name Offset ("main")          |
| 0x64      | `0x00004000`      | Value (address)               |
| 0x68      | `0x00000000`      | Section Index (0)             |
| 0x6C      | `0x01`            | Type (LABEL)                  |
| 0x6D      | `0x01`            | Binding (GLOBAL)              |
| 0x6E      | `0x0000`          | Flags                         |

Entry 1 (helper):

| Offset    | Value             | Field                         |
|-----------|-------------------|-------------------------------|
| 0x70      | `0x00000006`      | Name Offset ("helper")        |
| 0x74      | `0x00000000`      | Value (undefined)             |
| 0x78      | `0xFFFFFFFF`      | Section Index (UNDEF)         |
| 0x7C      | `0x00`            | Type (NONE)                   |
| 0x7D      | `0x02`            | Binding (EXTERN)              |
| 0x7E      | `0x0000`          | Flags                         |

Entry 2 (counter):

| Offset    | Value             | Field                         |
|-----------|-------------------|-------------------------------|
| 0x80      | `0x0000000D`      | Name Offset ("counter")       |
| 0x84      | `0x80000000`      | Value (address)               |
| 0x88      | `0x00000001`      | Section Index (1)             |
| 0x8C      | `0x02`            | Type (DATA)                   |
| 0x8D      | `0x00`            | Binding (LOCAL)               |
| 0x8E      | `0x0000`          | Flags                         |

**String Table (20 bytes at offset 0x90):**

```
Offset  Content
0x00    0x00                    (empty string)
0x01    "main" 0x00             (5 bytes)
0x06    "helper" 0x00           (7 bytes)
0x0D    "counter" 0x00          (8 bytes)
```

**Relocation Table (32 bytes at offset 0xA4):**

Entry 0 (call helper):

| Offset    | Value             | Field                         |
|-----------|-------------------|-------------------------------|
| 0xA4      | `0x00000008`      | Offset in section             |
| 0xA8      | `0x00000001`      | Symbol Index (helper)         |
| 0xAC      | `0x00000000`      | Section Index (0)             |
| 0xB0      | `0x0001`          | Type (ABS32)                  |
| 0xB2      | `0x0000`          | Addend                        |

Entry 1 (jpb main):

| Offset    | Value             | Field                         |
|-----------|-------------------|-------------------------------|
| 0xB4      | `0x0000000E`      | Offset in section             |
| 0xB8      | `0x00000000`      | Symbol Index (main)           |
| 0xBC      | `0x00000000`      | Section Index (0)             |
| 0xC0      | `0x0004`          | Type (REL32)                  |
| 0xC2      | `0x0000`          | Addend                        |

## Linker Processing

This section describes how the G10 linker (`g10link`) processes object files to
produce a final executable binary.

### Input Processing

1. **Load Object Files**: Read and validate each input object file, verifying
    the magic number and version compatibility.

2. **Collect Sections**: Gather all sections from all input files, grouping them
    by type (CODE, DATA, BSS).

3. **Build Symbol Table**: Merge symbol tables from all input files:
    - Local symbols are prefixed with their source file name to avoid conflicts.
    - Global symbols are collected into a unified table.
    - External symbol references are recorded for later resolution.

4. **Resolve Symbols**: Match external symbol references to global symbol
    definitions:
    - Each `EXTERN` binding must find exactly one `GLOBAL` definition.
    - Multiple definitions of the same global symbol is an error.
    - Unresolved external references are an error.

### Section Layout

1. **Address Assignment**: Sections are laid out in the output binary:
    - Sections with explicit addresses (from `.org`) retain those addresses.
    - Sections without explicit addresses are placed contiguously.
    - BSS sections are typically placed at the end of the RAM region.

2. **Overlap Detection**: The linker verifies that no sections overlap in the
    final memory layout.

3. **Alignment**: Sections may be aligned to specific boundaries as needed by
    the target platform (typically 4-byte alignment for the G10).

### Relocation Application

For each relocation entry in each input object file:

1. Look up the target symbol to obtain its final resolved address.
2. Calculate the relocation value based on the relocation type.
3. Write the value to the appropriate location in the output binary.
4. Verify that the value fits in the target field size.

### Output Generation

The linker produces a flat binary file suitable for execution:

1. **ROM Region**: All CODE and DATA sections are written to their assigned
    addresses, with gaps filled with a specified fill byte (typically
    `0x00` or `0xFF`).

2. **Entry Point**: If the `HAS_ENTRY` flag is set in any input file, the
    corresponding symbol's address is recorded in the program metadata region
    (`$00000000` - `$00000FFF`).

3. **BSS Initialization**: BSS sections are not included in the output binary;
    the runtime is responsible for zeroing this memory.

## File Validation

When processing a G10 object file, the following validations should be
performed:

### Header Validation

| Check                             | Error Condition                       |
|-----------------------------------|---------------------------------------|
| Magic number                      | Not equal to `0x4731304F`             |
| Version major                     | Greater than supported version        |
| Reserved fields                   | Non-zero values (warning only)        |
| Section count                     | Zero sections                         |
| Table offsets                     | Extend beyond file size               |

### Section Validation

| Check                             | Error Condition                       |
|-----------------------------------|---------------------------------------|
| Name offset                       | Beyond string table bounds            |
| Section type                      | Unrecognized type value               |
| Section overlap                   | Two sections occupy same address range|
| ROM/RAM consistency               | CODE/DATA in RAM, BSS in ROM          |

### Symbol Validation

| Check                             | Error Condition                       |
|-----------------------------------|---------------------------------------|
| Name offset                       | Beyond string table bounds            |
| Section index                     | Invalid (not special, not valid index)|
| Global redefinition               | Same global symbol defined twice      |
| Global/extern conflict            | Symbol marked both `.global` and `.extern`|
| Extern definition                 | Symbol marked `.extern` but defined   |
| Circular references               | Symbol depends on itself              |

### Relocation Validation

| Check                             | Error Condition                       |
|-----------------------------------|---------------------------------------|
| Offset                            | Beyond section size                   |
| Symbol index                      | Beyond symbol table size              |
| Section index                     | Invalid section reference             |
| Type                              | Unrecognized relocation type          |
| Overflow                          | Calculated value doesn't fit in field |

## Version History

| Version   | Date          | Changes                                       |
|-----------|---------------|-----------------------------------------------|
| 1.0.0     | 2025-12-26    | Initial specification                         |

## Appendix A: Quick Reference

### File Header (64 bytes)

```
+0x00  Magic (4)       +0x04  Version (4)     +0x08  Flags (4)
+0x0C  SectionCount    +0x10  SymTabOff       +0x14  SymTabSize
+0x18  StrTabOff       +0x1C  StrTabSize      +0x20  RelocOff
+0x24  RelocSize       +0x28  Reserved (24 bytes)
```

### Section Header (16 bytes)

```
+0x00  NameOff (4)     +0x04  VirtAddr (4)    +0x08  Size (4)
+0x0C  Type (2)        +0x0E  Flags (2)
```

### Symbol Entry (16 bytes)

```
+0x00  NameOff (4)     +0x04  Value (4)       +0x08  SectIdx (4)
+0x0C  Type (1)        +0x0D  Binding (1)     +0x0E  Flags (2)
```

### Relocation Entry (16 bytes)

```
+0x00  Offset (4)      +0x04  SymIdx (4)      +0x08  SectIdx (4)
+0x0C  Type (2)        +0x0E  Addend (2)
```
