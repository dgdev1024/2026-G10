; Test file for .global and .extern directives

; Single symbol
.global main

; Multiple symbols
.global start, loop, end

; External symbols
.extern helper
.extern printf, malloc, free

; Mixed with labels
start:
loop:
end:
