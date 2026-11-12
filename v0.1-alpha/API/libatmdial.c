#include "../include/SymbolTable/table.h"

extern SYM_TAB *symb;

void AutumnAPI_SetModem(void) {
    if (symb && symb->modem->Init) {
        symb->modem->Init();
    }
}

void AutumnAPI_Dial(const char* number) {
    if (symb && symb->modem->Dial) {
        symb->modem->Dial(number);
    }
}

void AutumnAPI_SendMsg(const char* number, const char* msg) {
    if (symb && symb->modem->SendM) {
        symb->modem->SendM(number, msg);
    }
}

void AutumnAPI_Answer(void) {
    if (symb && symb->modem->Answer) {
        symb->modem->Answer();
    }
}

void AutumnAPI_Decline(void) {
    if (symb && symb->modem->Decline) {
        symb->modem->Decline();
    }
}
