; Define a variable named 'the_answer' and set it to '0x21'.
.let $the_answer = 0x21

; Define a constant named 'HALF_THE_ANSWER" and set it to the current value
; of 'the_answer'.
.const $HALF_THE_ANSWER = $the_answer

; Use an arithmetic assignment operator (in this case, a multiplication), to
; mutate the value of 'the_answer', so that it becomes '0x42'.
;
; Throw in a division expression here, as well!
$the_answer *= (8 / 4)

; Since '$HALF_THE_ANSWER' is marked as a constant, any attempt to mutate it
; shall result in an assembly error, like the commented line below:
; $HALF_THE_ANSWER = 0x42               ; No, no, no!

; Expected RAM:
;   `0x80000000` = `0x21`
;   `0x80000001` = `0x42`

; Program
.global main
.ram
    half_the_answer:    .byte 1
    the_answer:         .byte 1
.rom
    main:
        ld l0, $HALF_THE_ANSWER         ; Becomes 'ld l0, 0x21'
        ld l1, $the_answer              ; Becomes 'ld l1, 0x42'.
                                        ; Note that all variables and constants begin with a dollar sign '$'.
        st [half_the_answer], l0
        st [the_answer], l1             ; Note that address label 'the_answer' is different from variable '$the_answer'.
        stop
