; Test 07: Direct Memory Operands
; Tests the parser's ability to parse direct memory addressing operands.
; Direct addressing uses a literal address enclosed in brackets [addr].

.org 0x2000

; Direct addressing with hexadecimal addresses
test_hex_direct:
    ld d0, [0x80000000]
    st [0x80000000], d0
    ld w0, [0xFFFF0000]
    st [0xFFFF0000], w0
    ld l0, [0xFFFFFF00]
    st [0xFFFFFF00], l0

; Direct addressing with decimal addresses
test_dec_direct:
    ld d0, [2147483648]
    st [2147483648], d0

; Direct addressing with label references
data_location:
.org 0x80000000
ram_data:
.dword 0

.org 0x2100
test_label_direct:
    ld d0, [ram_data]
    st [ram_data], d0
    ld d1, [data_location]

; Quick RAM addressing (relative to 0xFFFF0000)
test_quick_ram:
    ldq d0, [0x0000]
    ldq d0, [0x1000]
    stq [0x0000], d0
    stq [0xFFFF], d0

; I/O Port addressing (relative to 0xFFFFFF00)
test_io_ports:
    ldp l0, [0x00]
    ldp l0, [0x10]
    stp [0x00], l0
    stp [0xFF], l0
