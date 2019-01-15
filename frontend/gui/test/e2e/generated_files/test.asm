    .ORIG x3000

; Check Palindrome
; This program checks if the string at PTR is a palindrome.
    LEA R0, PTR
    ADD R1, R0, #0

AGAIN LDR R2, R1, #0
    BRz CONT
    ADD R1, R1, #1
    BRnzp AGAIN

CONT  ADD R1, R1, #-1
LOOP  LDR R3, R0, #0 ; Main Loop
    LDR R4, R1, #0
    NOT R4, R4
    ADD R4, R4, #1
    ADD R3, R3, R4
    BRnp NO
    ADD R0, R0, #1
    ADD R1, R1, #-1
    NOT R2, R0
    ADD R2, R2, #1
    ADD R2, R1, R2
    BRnz YES
    BR LOOP

YES   AND R5, R5, #0
    ADD   R5, R5, #1
    BRnzp DONE

NO    AND R5, R5, #0
DONE  HALT
PTR   .FILL X4000
    .END
    