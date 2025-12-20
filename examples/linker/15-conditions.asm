; =============================================================================
; Test 15: Conditional Branching Tests (Single File)
; =============================================================================
; Tests all branch conditions: NC, ZS, ZC, CS, CC, VS, VC
; =============================================================================

.org $80000000
test_results:
    .fill 16, $00

.org $2000

main:
    ld d4, test_results         ; D4 = results pointer
    ld l0, 1                    ; Success marker
    
    ; === Test NC (No Condition - always branch) ===
    jpb test_nc_target
    ld l0, 0                    ; Should NOT execute
test_nc_target:
    st [d4], l0                 ; Should be 1
    inc d4
    
    ; === Test ZS (Zero Set) ===
    ld l1, 0                    ; Set L1 = 0
    cmp l1, 0                   ; Compare with 0 -> Z = 1
    ld l0, 0
    jpb zs, test_zs_target      ; Should branch (Z is set)
    jpb test_zs_fail
test_zs_target:
    ld l0, 1
test_zs_fail:
    st [d4], l0
    inc d4
    
    ; === Test ZC (Zero Clear) ===
    ld l1, 5                    ; Set L1 = 5
    cmp l1, 0                   ; Compare with 0 -> Z = 0
    ld l0, 0
    jpb zc, test_zc_target      ; Should branch (Z is clear)
    jpb test_zc_fail
test_zc_target:
    ld l0, 1
test_zc_fail:
    st [d4], l0
    inc d4
    
    ; === Test CS (Carry Set) ===
    scf                         ; Set carry flag
    ld l0, 0
    jpb cs, test_cs_target      ; Should branch (C is set)
    jpb test_cs_fail
test_cs_target:
    ld l0, 1
test_cs_fail:
    st [d4], l0
    inc d4
    
    ; === Test CC (Carry Clear) ===
    ccf                         ; Toggle carry (now clear)
    ld l0, 0
    jpb cc, test_cc_target      ; Should branch (C is clear)
    jpb test_cc_fail
test_cc_target:
    ld l0, 1
test_cc_fail:
    st [d4], l0
    inc d4
    
    ; === Test VS (Overflow Set) ===
    sev                         ; Set overflow flag
    ld l0, 0
    jpb vs, test_vs_target      ; Should branch (V is set)
    jpb test_vs_fail
test_vs_target:
    ld l0, 1
test_vs_fail:
    st [d4], l0
    inc d4
    
    ; === Test VC (Overflow Clear) ===
    clv                         ; Clear overflow flag
    ld l0, 0
    jpb vc, test_vc_target      ; Should branch (V is clear)
    jpb test_vc_fail
test_vc_target:
    ld l0, 1
test_vc_fail:
    st [d4], l0
    
    halt
