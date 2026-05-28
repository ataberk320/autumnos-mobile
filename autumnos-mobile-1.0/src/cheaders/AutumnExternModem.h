#ifndef ATMODEM_H
#define ATMODEM_H

typedef struct {
    int ctrl_fd; // /dev/ttyACM0 için (AT komutlarını göndereceğimiz kontrol kanalı)
    int data_fd; // /dev/cdc-wdm0 için (Veri transferi yapacağımız data kanalı)
} AutumnSIM_t;

int AutumnModem_Int_Init(AutumnSIM_t *modem);

int AutumnModem_Check(AutumnSIM_t *modem);

void AutumnModem_SignalINF(AutumnSIM_t *modem);

void AutumnModem_Call(AutumnSIM_t *modem, const char *number);

void AutumnModem_CloseRsc(AutumnSIM_t *modem);

#endif
