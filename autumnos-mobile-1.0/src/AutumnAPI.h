#ifndef API_H
#define API_H

void AutumnAPI_Request_PowerOff(void);

void AutumnAPI_Request_Reboot(void);

int AutumnAPI_Read_Battery_Level(void);

long AutumnAPI_Read_Uptime(void);

long AutumnAPI_Read_Used_RAM(void);

long AutumnAPI_Read_Free_Disk(void);

void AutumnAPI_Play_Video(const char* source);

int AutumnAPI_SIM_Status(void);

#endif
