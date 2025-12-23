; Complex test with mixed labels and directives
.global main, helper
.extern printf

main:
    .global internal_func
    
internal_func:

helper:
    .extern malloc
    
end:
.global end
