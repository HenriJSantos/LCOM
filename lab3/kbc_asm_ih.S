#include <machine/asm.h>
#include "i8042.h"

.intel_syntax

.global _C_LABEL(kbc_asm_ih)
.global _C_LABEL(scancode)

.data
scancode:
    .byte 0 
.text
_C_LABEL(kbc_asm_ih):
        in      al,STATUS_REG                   // Read status into al register
        mov     bl, al                          // Move status to bl register to avoid eax register changes
        
        test    bl, OBF                         // Check status for OBF (if output buffer has anything)
        jz      invalid                         // If it doesn't, there's no scancode to be read
        in      al, OUTPUT_BUFFER               // If it does, read it

        mov     dl, PARITY_ERR                  // Set parity error bit in dl
        or      dl, TIMEOUT_ERR                 // Set timeout error bit in dl
        test    bl, dl                          // Check if either parity or timeout error bits are set in status
        jnz     invalid                         // If any error is detected, discard scancode read
        mov     scancode, al                    // If no errors are detected, move scancode into global variable
        jmp     end                             // And jump to the end

invalid:mov     al, 0                           // If any errors reading the scancode occur, set scancode as "0"
        mov     scancode, al                    // And move that 0 into global variable

end:    ret
