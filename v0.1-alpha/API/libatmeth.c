#include <linux/if.h>
#include "table.h"

extern ETHERNET_HAL* eth;

int AutumnAPI_EthOn(const char *ifname) {
	return eth->InterfaceSetup(ifname, IFF_UP, 1);
}

int AutumnAPI_EthOff(const char *ifname) {
	return eth->InterfaceSetup(ifname, IFF_UP, 0);
}

