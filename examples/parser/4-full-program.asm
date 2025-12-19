; Test 4: Full Program - A complete realistic assembly program
; This demonstrates a working program with all parser features.

.ORG 0x2000

.GLOBAL _start, print_message
.EXTERN system_call

; Constants defined as data
message_addr:
    .BYTE "Hello, G10 CPU!", 0x0A, 0x00

counter_value:
    .DWORD 0x00000000

; Program entry point
_start:
    ; Initialize stack pointer
    LD D15, 0xFFFFFFFF
    MV SP, D15

    ; Load message address
    LD D0, message_addr
    CALL print_message

    ; Initialize counter
    LD D1, 0
    ST [counter_value], D1

main_loop:
    ; Increment counter
    LD D1, [counter_value]
    INC D1
    ST [counter_value], D1

    ; Check if counter reached 10
    CMP D0, 10
    JPB ZS, exit_program

    ; Continue loop
    JMP main_loop

exit_program:
    HALT

; Subroutine: Print message
print_message:
    PUSH D0
    PUSH D1

    ; Call external system function
    CALL system_call

    POP D1
    POP D0
    RET
