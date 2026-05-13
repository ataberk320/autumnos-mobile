#ifndef API_H
#define API_H

typedef struct {
	int x;
	int y;
	int pressed;
} MouseData;

void AutumnAPI_Request_PowerOff(void);

void AutumnAPI_Emergency_PowerOff();

int AutumnAPI_Set_MsPipe();

int AutumnAPI_Show_Toast(const char *msg);

void AutumnAPI_Request_Reboot(void);

void AutumnAPI_Read_Mouse(int fd, MouseData *data);

int AutumnAPI_Read_Battery_Level(void);

long AutumnAPI_Read_Uptime(void);

long AutumnAPI_Read_Used_RAM(void);

long AutumnAPI_Read_Free_Disk(void);

void AutumnAPI_Play_Video(const char* source);

int AutumnAPI_SIM_Status(void);

#endif
