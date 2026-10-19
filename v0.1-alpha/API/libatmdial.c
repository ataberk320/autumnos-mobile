#include "../include/SymbolTable/table.h"

extern SYM_TAB* symb;

void AutumnAPI_SetModem() {
	symb->ModemInit();
}

void AutumnAPI_Dial(const char* number) {
	symb->Dial(number);
}

void AutumnAPI_SendMsg(const char* number, const char* msg) {
	symb->SendMsg(number, msg);
}

void AutumnAPI_Answer() {
	symb->Answer();
}

void AutumnAPI_Decline() {
	symb->Decline();
}
