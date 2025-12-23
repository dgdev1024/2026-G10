; Comprehensive test for .global and .extern directives

; Test 1: Single symbol declarations
.global main
.extern helper

; Test 2: Multiple symbols on one line
.global func1, func2, func3
.extern lib1, lib2, lib3

; Test 3: Mixing with labels and comments
.global start       ; Export start symbol
start:              ; Define start label
    .extern cleanup ; Import cleanup symbol

.global loop, done
loop:
done:

; Test 4: Many symbols
.global sym1, sym2, sym3, sym4, sym5, sym6, sym7, sym8
.extern ext1, ext2, ext3, ext4
