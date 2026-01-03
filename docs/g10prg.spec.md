# Specification: G10 Program File Format

Below is a specification for the G10 CPU program file format. This format is
used for G10 program files, output by the G10 linker tool (`g10link`) and
executable by any virtual machine software powered by the G10 CPU architecture.

## Details

- **File Extension**: `.g10`
    - As with the G10 object file format, the file extension is not enforced;
        files with any name and extension can be used, so as long as the file
        contents conform to the G10 program file format specification.
- **Endianness**: Little-endian
- **Magic Number**: `0x47313050` (`'G10P'` in ASCII)
- **Version**: Same as G10 object file format version; `0xMMmmPPPP`
    - `MM`: Major version (1 byte)
    - `mm`: Minor version (1 byte)
    - `PPPP`: Patch version (2 bytes)

## File Structure Overview

The G10 program file is the final executable output produced by the G10 linker.
Unlike object files, program files contain fully-resolved code and data ready
for direct execution by a G10-powered virtual machine. All symbol references
have been resolved and all relocations have been applied.

The program file format is designed to:

- **Load Directly into Memory**: Program segments map directly to G10 memory
    regions with minimal processing required by the VM loader.
- **Support ROM/RAM Separation**: Distinguish between initialized ROM data
    (loaded from file) and BSS RAM reservations (zero-initialized at runtime).
- **Provide Execution Metadata**: Include entry point, stack initialization,
    and optional program information for the VM.

### File Layout

| Offset (Bytes)    | Size (Bytes)      | Description                                       |
|-------------------|-------------------|---------------------------------------------------|
| `0x00000000`      | 4                 | Magic Number (`0x47313050`)                       |
| `0x00000004`      | 4                 | Version Number                                    |
| `0x00000008`      | 4                 | Flags                                             |
| `0x0000000C`      | 4                 | Entry Point Address                               |
| `0x00000010`      | 4                 | Initial Stack Pointer                             |
| `0x00000014`      | 4                 | Segment Count                                     |
| `0x00000018`      | 4                 | Program Info Offset (optional metadata)           |
| `0x0000001C`      | 4                 | Program Info Size (in bytes)                      |
| `0x00000020`      | 32                | Reserved (must be zero)                           |
| `0x00000040`      | Variable          | Segment Headers (16 bytes each)                   |
| Variable          | Variable          | Segment Data (ROM segments only)                  |
| Variable          | Variable          | Program Info (optional)                           |

## Header Fields

This section describes each field in the program file header in detail.

### Magic Number (Offset `0x00`, 4 bytes)

A 32-bit value used to identify the file as a valid G10 program file. The magic
number is `0x47313050`, which corresponds to the ASCII string `'G10P'` when
read as little-endian bytes (`0x47` = 'G', `0x31` = '1', `0x30` = '0', 
`0x50` = 'P').

### Version Number (Offset `0x04`, 4 bytes)

A 32-bit value representing the version of the program file format. The version
number is structured identically to the object file format:

| Bits      | Field         | Description                                       |
|-----------|---------------|---------------------------------------------------|
| 31-24     | Major         | Major version number (0-255)                      |
| 23-16     | Minor         | Minor version number (0-255)                      |
| 15-0      | Patch         | Patch version number (0-65535)                    |

Version compatibility rules:
- Files with different major versions are incompatible.
- Files with the same major version but different minor versions may be
    forward-compatible (newer VMs can run older program versions).
- Patch versions indicate bug fixes and are always compatible.

### Flags (Offset `0x08`, 4 bytes)

A 32-bit bitfield containing program-level flags:

| Bit   | Name              | Description                                       |
|-------|-------------------|---------------------------------------------------|
| 0     | `HAS_ENTRY`       | Program has a valid entry point                   |
| 1     | `HAS_STACK_INIT`  | Program specifies initial stack pointer           |
| 2     | `HAS_INFO`        | Program contains optional info section            |
| 3     | `DEBUG_BUILD`     | Program was built with debug information          |
| 4     | `DOUBLE_SPEED`    | Program requests double-speed CPU mode            |
| 5-31  | Reserved          | Reserved for future use (must be zero)            |

### Entry Point Address (Offset `0x0C`, 4 bytes)

A 32-bit unsigned integer specifying the memory address where program execution
should begin. This is the address loaded into the CPU's Program Counter (PC)
register upon program start.

The entry point must be within the ROM region (`0x00002000` to `0x7FFFFFFF`).
If the `HAS_ENTRY` flag is not set, this field should contain `0x00002000`
(the default program start address).

### Initial Stack Pointer (Offset `0x10`, 4 bytes)

A 32-bit unsigned integer specifying the initial value for the CPU's Stack
Pointer (SP) register. The stack grows downward in the G10 architecture.

The initial stack pointer must be within the RAM region (`0x80000000` to
`0xFFFFFFFF`). If the `HAS_STACK_INIT` flag is not set, the VM should use its
default stack pointer value (typically `0xFFFFFFFC`, the top of RAM aligned to
4 bytes).

### Segment Count (Offset `0x14`, 4 bytes)

A 32-bit unsigned integer indicating the number of segment headers present in
the file. Segment headers begin immediately after the fixed header at offset
`0x40`.

### Program Info Offset (Offset `0x18`, 4 bytes)

A 32-bit unsigned integer indicating the byte offset from the start of the file
to the optional program information section. If no program info is present (the
`HAS_INFO` flag is not set), this value should be zero.

### Program Info Size (Offset `0x1C`, 4 bytes)

A 32-bit unsigned integer indicating the size of the program information section
in bytes. If no program info is present, this value should be zero.

### Reserved (Offset `0x20`, 32 bytes)

Reserved space for future header extensions. All bytes in this region must be
set to zero. VMs should ignore non-zero values in this region for forward
compatibility.

## Segment Headers

Segment headers describe the memory segments that comprise the program. Each
segment represents a contiguous block of memory that the VM loader must prepare.
Segment headers begin at offset `0x40` and are stored contiguously.

### Segment Header Layout

Each segment header is 16 bytes in size and structured as follows:

| Offset    | Size  | Field             | Description                               |
|-----------|-------|-------------------|-------------------------------------------|
| `0x00`    | 4     | Load Address      | Target address in G10 memory              |
| `0x04`    | 4     | Memory Size       | Size in memory (bytes)                    |
| `0x08`    | 4     | File Size         | Size in file (bytes); 0 for BSS           |
| `0x0C`    | 2     | Type              | Segment type identifier                   |
| `0x0E`    | 2     | Flags             | Segment attribute flags                   |

### Segment Types

The segment type field indicates how the loader should handle the segment:

| Value     | Name          | Description                                       |
|-----------|---------------|---------------------------------------------------|
| `0x0000`  | `NULL`        | Unused segment header entry                       |
| `0x0001`  | `CODE`        | Executable code (ROM region)                      |
| `0x0002`  | `DATA`        | Initialized read-only data (ROM region)           |
| `0x0003`  | `BSS`         | Uninitialized data (RAM region, zero-filled)      |
| `0x0004`  | `METADATA`    | Program metadata region                           |
| `0x0005`  | `INTERRUPT`   | Interrupt vector table                            |

Segment types and their expected memory regions:
- `CODE` and `DATA` segments should target the ROM region 
    (`0x00002000` to `0x7FFFFFFF`).
- `BSS` segments should target the RAM region 
    (`0x80000000` to `0xFFFFFFFF`).
- `METADATA` segments target the program metadata region 
    (`0x00000000` to `0x00000FFF`).
- `INTERRUPT` segments target the interrupt table region 
    (`0x00001000` to `0x00001FFF`).

### Segment Flags

The segment flags field is a 16-bit bitfield describing segment attributes:

| Bit   | Name          | Description                                       |
|-------|---------------|---------------------------------------------------|
| 0     | `LOAD`        | Segment data is loaded from file                  |
| 1     | `ZERO_FILL`   | Segment is zero-initialized (not loaded)          |
| 2     | `EXEC`        | Segment contains executable code                  |
| 3     | `WRITE`       | Segment is writable at runtime                    |
| 4-15  | Reserved      | Reserved for future use (must be zero)            |

Common flag combinations:
- **Code Segment**: `LOAD | EXEC` = `0x0005`
- **Data Segment**: `LOAD` = `0x0001`
- **BSS Segment**: `ZERO_FILL | WRITE` = `0x000A`
- **Interrupt Table**: `LOAD | EXEC` = `0x0005`

### Segment Data

For segments with the `LOAD` flag set, the actual segment data is stored in the
file following all segment headers. Data is stored contiguously in the order of
the segment headers. Each segment's file data begins at the offset calculated
by summing:
1. The header offset (`0x40`)
2. The total size of all segment headers (16 bytes × segment count)
3. The cumulative file size of all preceding segments

For `BSS` segments (with `ZERO_FILL` flag), no data is stored in the file. The
VM loader is responsible for allocating and zero-initializing this memory
region at runtime.

### Memory Size vs. File Size

The distinction between Memory Size and File Size allows for efficient handling
of different segment types:

- **For `CODE` and `DATA` segments**: Memory Size equals File Size. The loader
    copies exactly this many bytes from the file into memory.
    
- **For `BSS` segments**: File Size is zero (no file data). Memory Size
    indicates how many bytes to allocate and zero-fill in RAM.
    
- **For partially initialized segments**: If Memory Size > File Size, the
    loader copies File Size bytes from the file, then zero-fills the remaining
    (Memory Size - File Size) bytes. This is useful for segments with trailing
    uninitialized data.

## Program Info Section

The optional Program Info section contains human-readable metadata about the
program. This information is useful for VM user interfaces, debuggers, and
program management tools, but is not required for execution.

### Program Info Structure

The Program Info section consists of a fixed header followed by variable-length
string data:

| Offset    | Size  | Field             | Description                               |
|-----------|-------|-------------------|-------------------------------------------|
| `0x00`    | 2     | Info Version      | Version of info structure (currently 1)   |
| `0x02`    | 2     | Flags             | Info section flags                        |
| `0x04`    | 4     | Name Offset       | Offset to program name string             |
| `0x08`    | 4     | Name Length       | Length of program name (bytes)            |
| `0x0C`    | 4     | Version Offset    | Offset to version string                  |
| `0x10`    | 4     | Version Length    | Length of version string (bytes)          |
| `0x14`    | 4     | Author Offset     | Offset to author string                   |
| `0x18`    | 4     | Author Length     | Length of author string (bytes)           |
| `0x1C`    | 4     | Desc Offset       | Offset to description string              |
| `0x20`    | 4     | Desc Length       | Length of description string (bytes)      |
| `0x24`    | 4     | Build Date        | Build timestamp (Unix epoch, seconds)     |
| `0x28`    | 4     | Checksum          | CRC-32 of segment data                    |
| `0x2C`    | 4     | Reserved          | Reserved (must be zero)                   |
| `0x30`    | Var   | String Data       | Concatenated string data                  |

### Info Flags

| Bit   | Name              | Description                                       |
|-------|-------------------|---------------------------------------------------|
| 0     | `HAS_NAME`        | Program name string is present                    |
| 1     | `HAS_VERSION`     | Version string is present                         |
| 2     | `HAS_AUTHOR`      | Author string is present                          |
| 3     | `HAS_DESC`        | Description string is present                     |
| 4     | `HAS_CHECKSUM`    | Checksum field is valid                           |
| 5-15  | Reserved          | Reserved for future use (must be zero)            |

### String Data Format

String offsets are relative to the start of the String Data region (offset
`0x30` within the Program Info section). Strings are stored as raw bytes
without null terminators; the length field specifies the exact byte count.

All strings should be encoded as UTF-8 for maximum compatibility.

### Checksum Calculation

If the `HAS_CHECKSUM` flag is set, the Checksum field contains a CRC-32 value
calculated over all segment data in the file. This allows the VM to verify
program integrity before execution. The CRC-32 uses the standard polynomial
`0xEDB88320` (reflected form, as used by zlib and PNG).

## Loading Process

This section describes how a G10-powered virtual machine should load and
prepare a program file for execution.

### Step 1: Header Validation

1. Read the first 64 bytes (file header).
2. Verify the magic number equals `0x47313050`.
3. Check version compatibility with the VM's supported format version.
4. Validate that reserved fields are zero (warn if not, for forward compat.).
5. Record the entry point, initial stack pointer, and segment count.

### Step 2: Segment Loading

For each segment header (starting at offset `0x40`):

1. Read the 16-byte segment header.
2. Validate the load address is within the expected memory region for its type.
3. Check for segment overlap with previously loaded segments.
4. Based on segment flags:
    - If `LOAD` flag is set: Copy File Size bytes from the file into memory
        at the Load Address.
    - If `ZERO_FILL` flag is set: Zero-initialize Memory Size bytes at the
        Load Address.
    - If Memory Size > File Size and `LOAD` is set: Copy File Size bytes,
        then zero-fill the remaining (Memory Size - File Size) bytes.

### Step 3: Memory Initialization

After all segments are loaded:

1. Initialize the Program Metadata region (`0x00000000` - `0x00000FFF`) if no
    `METADATA` segment was provided. The VM may populate default values.
2. Ensure the Interrupt Table region (`0x00001000` - `0x00001FFF`) is properly
    initialized. If no `INTERRUPT` segment was provided, this region should
    contain default interrupt handlers or be zero-filled.
3. Zero-initialize any BSS (RAM) regions not explicitly covered by segments.

### Step 4: CPU Initialization

1. Reset the CPU to its initial state.
2. Set the Program Counter (PC) to the Entry Point Address from the header.
3. Set the Stack Pointer (SP) to the Initial Stack Pointer from the header,
    or the VM's default if `HAS_STACK_INIT` flag is not set.
4. If the `DOUBLE_SPEED` flag is set, configure the CPU for double-speed mode.
5. Clear all general-purpose registers to zero.
6. Clear the flags register.
7. Set the Interrupt Master Enable (IME) flag based on VM policy (typically
    disabled at startup; the program enables interrupts when ready).

### Step 5: Execution

Begin CPU execution. The CPU will fetch its first instruction from the
address in the PC register (the entry point).

## Linker Output Generation

This section describes how the G10 linker (`g10link`) produces a program file
from one or more object files.

### Symbol Resolution

Before generating the program file, the linker must:

1. Collect all symbols from input object files.
2. Resolve external references to global definitions.
3. Verify no duplicate global symbols exist.
4. Identify the entry point symbol (typically `main` or `_start`).

### Relocation Application

For each relocation entry in the input object files:

1. Calculate the final symbol address based on section placement.
2. Apply the relocation formula based on the relocation type.
3. Patch the resolved value into the section data.

All relocations must be fully resolved; unresolved external references are
a linker error.

### Section Merging

The linker combines sections from multiple object files:

1. **CODE sections**: Merged into one or more CODE segments, maintaining
    the address ordering specified by `.org` directives.
2. **DATA sections**: Merged into DATA segments following code.
3. **BSS sections**: Combined into BSS segments in the RAM region.

Adjacent sections of the same type at contiguous addresses may be merged
into a single segment to reduce segment count.

### Address Assignment

Sections are assigned final addresses based on:

1. Explicit addresses from `.org` directives (preserved exactly).
2. Implicit ordering for sections without explicit addresses.
3. Alignment requirements (typically 4-byte alignment for G10).

The linker verifies that:
- No segments overlap in memory.
- CODE and DATA segments are in the ROM region.
- BSS segments are in the RAM region.
- The entry point is within a CODE segment.

### Segment Generation

For each distinct memory region with loaded or reserved data:

1. Create a segment header with:
    - Load Address: The final resolved address.
    - Memory Size: Total bytes occupied in memory.
    - File Size: Total bytes of initialized data (0 for BSS).
    - Type and Flags: Based on section types from object files.

2. Collect and concatenate the patched section data for that segment.

### Program File Assembly

The final program file is assembled as follows:

1. Write the 64-byte program header.
2. Write all segment headers (16 bytes each).
3. Write segment data for all LOAD segments, in segment order.
4. Optionally write the Program Info section.
5. Update header offsets and sizes.

## Example Program File

This section provides an example of a complete G10 program file structure.

### Source Assembly (Linked)

```asm
; Combined from multiple object files
.org 0x00001000
interrupt_table:
    ; Interrupt vector 0 (exception handler)
    jpb nc, exception_handler
    ; ... (remaining vectors)

.org 0x00002000
main:
    di                      ; Disable interrupts during setup
    ld d0, 0x00             ; Initialize counter
    ei                      ; Enable interrupts
loop:
    inc d0                  ; Increment counter
    st [counter], d0        ; Store to RAM
    jpb nc, loop            ; Loop forever

exception_handler:
    reti                    ; Return from exception

.org 0x80000000
counter:
    .dword 1                ; Reserve 4 bytes in BSS
```

### Resulting Program File

**File Header (64 bytes at offset 0x00):**

| Offset    | Value             | Field                         |
|-----------|-------------------|-------------------------------|
| 0x00      | `0x47313050`      | Magic Number ("G10P")         |
| 0x04      | `0x01000000`      | Version 1.0.0                 |
| 0x08      | `0x00000003`      | Flags (HAS_ENTRY, HAS_STACK)  |
| 0x0C      | `0x00002000`      | Entry Point (main)            |
| 0x10      | `0xFFFFFFFC`      | Initial Stack Pointer         |
| 0x14      | `0x00000003`      | Segment Count (3)             |
| 0x18      | `0x00000000`      | Program Info Offset (none)    |
| 0x1C      | `0x00000000`      | Program Info Size (none)      |
| 0x20      | (32 bytes of 0)   | Reserved                      |

**Segment Headers (48 bytes at offset 0x40):**

Segment 0 (INTERRUPT):

| Offset    | Value             | Field                         |
|-----------|-------------------|-------------------------------|
| 0x40      | `0x00001000`      | Load Address                  |
| 0x44      | `0x00000006`      | Memory Size (6 bytes)         |
| 0x48      | `0x00000006`      | File Size (6 bytes)           |
| 0x4C      | `0x0005`          | Type (INTERRUPT)              |
| 0x4E      | `0x0005`          | Flags (LOAD | EXEC)           |

Segment 1 (CODE):

| Offset    | Value             | Field                         |
|-----------|-------------------|-------------------------------|
| 0x50      | `0x00002000`      | Load Address                  |
| 0x54      | `0x0000001A`      | Memory Size (26 bytes)        |
| 0x58      | `0x0000001A`      | File Size (26 bytes)          |
| 0x5C      | `0x0001`          | Type (CODE)                   |
| 0x5E      | `0x0005`          | Flags (LOAD | EXEC)           |

Segment 2 (BSS):

| Offset    | Value             | Field                         |
|-----------|-------------------|-------------------------------|
| 0x60      | `0x80000000`      | Load Address                  |
| 0x64      | `0x00000004`      | Memory Size (4 bytes)         |
| 0x68      | `0x00000000`      | File Size (0 - no file data)  |
| 0x6C      | `0x0003`          | Type (BSS)                    |
| 0x6E      | `0x000A`          | Flags (ZERO_FILL | WRITE)     |

**Segment Data (at offset 0x70):**

```
; Segment 0 data (interrupt table entry, 6 bytes)
Offset 0x70: XX XX  YY YY YY YY     ; jpb nc, exception_handler

; Segment 1 data (main code, 26 bytes)
Offset 0x76: XX XX                   ; di
Offset 0x78: XX XX  00 00 00 00     ; ld d0, 0x00
Offset 0x7E: XX XX                   ; ei
Offset 0x80: XX XX                   ; inc d0
Offset 0x82: XX XX  00 00 00 80     ; st [0x80000000], d0
Offset 0x88: XX XX  YY YY YY YY     ; jpb nc, loop
Offset 0x8E: XX XX                   ; reti

; No segment 2 data (BSS - zero-filled at runtime)
```

(Note: `XX XX` represents instruction opcodes; `YY YY YY YY` represents
resolved address/offset values after relocation.)

## File Validation

When loading a G10 program file, the following validations should be performed:

### Header Validation

| Check                             | Error Condition                       |
|-----------------------------------|---------------------------------------|
| Magic number                      | Not equal to `0x47313050`             |
| Version major                     | Greater than supported version        |
| Reserved fields                   | Non-zero values (warning only)        |
| Segment count                     | Zero segments                         |
| Entry point                       | Outside ROM region or not in segment  |
| Stack pointer                     | Outside RAM region                    |
| Info offset                       | Extends beyond file size              |

### Segment Validation

| Check                             | Error Condition                       |
|-----------------------------------|---------------------------------------|
| Segment type                      | Unrecognized type value               |
| Load address                      | Invalid for segment type              |
| Memory size                       | Zero (except for NULL segments)       |
| File size                         | Greater than memory size              |
| Segment overlap                   | Two segments occupy same address range|
| Data bounds                       | Segment data extends beyond file      |

### Memory Region Validation

| Segment Type  | Required Load Address Range                           |
|---------------|-------------------------------------------------------|
| `METADATA`    | `0x00000000` - `0x00000FFF`                           |
| `INTERRUPT`   | `0x00001000` - `0x00001FFF`                           |
| `CODE`        | `0x00002000` - `0x7FFFFFFF`                           |
| `DATA`        | `0x00002000` - `0x7FFFFFFF`                           |
| `BSS`         | `0x80000000` - `0xFFFFFFFF`                           |

### Checksum Validation

If the Program Info section is present and the `HAS_CHECKSUM` flag is set:

1. Calculate CRC-32 over all segment data in the file.
2. Compare with the stored checksum value.
3. If mismatch, the file may be corrupted; the VM should warn or refuse to load.

## Comparison with Object File Format

The following table summarizes the key differences between the G10 object file
format and the G10 program file format:

| Aspect                | Object File (`.g10obj`)       | Program File (`.g10`)         |
|-----------------------|-------------------------------|-------------------------------|
| Magic Number          | `0x4731304F` ("G10O")         | `0x47313050` ("G10P")         |
| Purpose               | Intermediate (for linker)     | Final executable (for VM)     |
| Symbol Table          | Present (for linking)         | Not present (resolved)        |
| Relocation Table      | Present (for patching)        | Not present (applied)         |
| String Table          | Present (symbol names)        | Not present                   |
| Entry Point           | Via symbol flag               | Explicit in header            |
| Stack Pointer         | Not specified                 | Explicit in header            |
| Section/Segment Data  | May need relocation           | Fully resolved, ready to load |
| BSS Handling          | Section size only             | Segment with zero file size   |

## Version History

| Version   | Date          | Changes                                       |
|-----------|---------------|-----------------------------------------------|
| 1.0.0     | 2025-12-26    | Initial specification                         |

## Appendix A: Quick Reference

### File Header (64 bytes)

```
+0x00  Magic (4)       +0x04  Version (4)     +0x08  Flags (4)
+0x0C  EntryPoint (4)  +0x10  StackPtr (4)    +0x14  SegCount (4)
+0x18  InfoOff (4)     +0x1C  InfoSize (4)    +0x20  Reserved (32)
```

### Segment Header (16 bytes)

```
+0x00  LoadAddr (4)    +0x04  MemSize (4)     +0x08  FileSize (4)
+0x0C  Type (2)        +0x0E  Flags (2)
```

### Program Info Header (48 bytes + strings)

```
+0x00  InfoVer (2)     +0x02  Flags (2)       +0x04  NameOff (4)
+0x08  NameLen (4)     +0x0C  VerOff (4)      +0x10  VerLen (4)
+0x14  AuthOff (4)     +0x18  AuthLen (4)     +0x1C  DescOff (4)
+0x20  DescLen (4)     +0x24  BuildDate (4)   +0x28  Checksum (4)
+0x2C  Reserved (4)    +0x30  StringData...
```

### Segment Types

| Value     | Name          | Region                                    |
|-----------|---------------|-------------------------------------------|
| `0x0000`  | NULL          | (unused)                                  |
| `0x0001`  | CODE          | `0x00002000` - `0x7FFFFFFF`               |
| `0x0002`  | DATA          | `0x00002000` - `0x7FFFFFFF`               |
| `0x0003`  | BSS           | `0x80000000` - `0xFFFFFFFF`               |
| `0x0004`  | METADATA      | `0x00000000` - `0x00000FFF`               |
| `0x0005`  | INTERRUPT     | `0x00001000` - `0x00001FFF`               |

### Segment Flags

| Bit   | Name          | Value     |
|-------|---------------|-----------|
| 0     | LOAD          | `0x0001`  |
| 1     | ZERO_FILL     | `0x0002`  |
| 2     | EXEC          | `0x0004`  |
| 3     | WRITE         | `0x0008`  |

### Header Flags

| Bit   | Name          | Value     |
|-------|---------------|-----------|
| 0     | HAS_ENTRY     | `0x0001`  |
| 1     | HAS_STACK_INIT| `0x0002`  |
| 2     | HAS_INFO      | `0x0004`  |
| 3     | DEBUG_BUILD   | `0x0008`  |
| 4     | DOUBLE_SPEED  | `0x0010`  |
