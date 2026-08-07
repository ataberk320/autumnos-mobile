//definition of ioctl commands of AutumnOS drivers

#ifndef AIOC_C
#define AIOC_C


#define G2D_IOC_MAGIC 'g'
#define SIM_IOC_MAGIC 's'
#define IOC_M 'a'
#define IOCG2DBITBLT _IOW(G2D_IOC_MAGIC, 0x40, unsigned long)
#define IOCMODSCAN       _IO(SIM_IOC_MAGIC, 1)
#define IOCMODGETSTAT     _IOR(SIM_IOC_MAGIC, 2, int)
#define IOCMODSNDCMD   _IOW(SIM_IOC_MAGIC, 3, char*)
#define IOCMODRDRESP  _IOR(SIM_IOC_MAGIC, 4, char*)
#define IOCTBLREGSRV _IOW(IOC_M, 1, unsigned long)
#define IOCTBLGETSRV _IOR(IOC_M, 2, unsigned long)

#endif
