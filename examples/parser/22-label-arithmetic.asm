; Test 22: Label Arithmetic and Address Calculations
; Tests expressions involving label references and address calculations.

.org 0x2000

; Code section with labels
code_start:
    nop
    nop
    nop
func1:
    nop
    nop
func2:
    nop
    nop
    nop
    nop
func3:
    nop
code_end:

; Jump using label references
test_label_jumps:
    jmp nc, code_start
    jmp nc, func1
    jmp nc, func2
    jmp nc, func3
    jmp nc, code_end

; Load label addresses
test_load_labels:
    ld d0, code_start
    ld d0, func1
    ld d0, func2
    ld d0, func3
    ld d0, code_end

; Label arithmetic - offset calculations
test_label_offsets:
    ld d0, code_start + 4           ; Offset by 4 bytes
    ld d0, func1 + 0x10             ; Offset by 16 bytes
    ld d0, code_end - 2             ; 2 bytes before end

; Label arithmetic - distance/size calculations
test_label_sizes:
    ld d0, code_end - code_start    ; Total code size
    ld d0, func2 - func1            ; Size of func1
    ld d0, func3 - func2            ; Size of func2

; Load from memory using label arithmetic
test_load_label_memory:
    ld d0, [data_base]
    ld d0, [data_base + 4]
    ld d0, [data_base + 8]
    ld d0, [data_base + array_offset]

; Store using label arithmetic
test_store_label_memory:
    st [data_base], d0
    st [data_base + 4], d0
    st [data_base + element_size * 2], d0

; Constants for calculations
element_size:
    .dword 4
array_offset:
    .dword 16

; Data section
.org 0x80000000

data_base:
.dword 0, 0, 0, 0                   ; Array of 4 dwords

; Pointer table using label addresses
pointer_table:
.dword code_start
.dword func1
.dword func2
.dword func3
.dword code_end

; Size table using label arithmetic
size_table:
.dword func1 - code_start           ; Offset to func1
.dword func2 - code_start           ; Offset to func2
.dword func3 - code_start           ; Offset to func3
.dword code_end - code_start        ; Total code size
