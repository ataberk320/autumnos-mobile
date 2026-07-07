#include "table.h"

extern MODEM_HAL* modem;

void AutumnAPI_SetModem() {
	modem->Init();
}

void AutumnAPI_Dial(const char* number) {
	modem->Dial(number);
}

void AutumnAPI_SendMsg(const char* number, const char* msg) {
	modem->SendM(number, msg);
}

void AutumnAPI_Answer() {
	modem->Answer();
}

void AutumnAPI_Decline() {
	modem->Decline();
}
