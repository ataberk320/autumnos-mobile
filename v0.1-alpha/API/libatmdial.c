#include "../include/SymbolTable/table.h"

extern SYM_TAB *symb;

void AutumnAPI_SetModem(void) {
    if (symb && symb->ModemInit) {
        symb->ModemInit();
    }
}

void AutumnAPI_Dial(const char* number) {
    if (symb && symb->Dial) {
        symb->Dial(number);
    }
}

void AutumnAPI_SendMsg(const char* number, const char* msg) {
    if (symb && symb->SendMsg) {
        symb->SendMsg(number, msg);
    }
}

void AutumnAPI_Answer(void) {
    if (symb && symb->Answer) {
        symb->Answer();
    }
}

void AutumnAPI_Decline(void) {
    if (symb && symb->Decline) {
        symb->Decline();
    }
}
