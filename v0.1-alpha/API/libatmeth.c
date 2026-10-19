#include <linux/if.h>
#include "../include/SymbolTable/table.h"

extern SYM_TAB* symb;

int AutumnAPI_EthOn(const char *ifname) {
	return symb->InterfaceSetup(ifname, IFF_UP, 1);
}

int AutumnAPI_EthOff(const char *ifname) {
	return symb->InterfaceSetup(ifname, IFF_UP, 0);
}

