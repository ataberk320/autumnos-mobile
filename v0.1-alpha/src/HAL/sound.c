#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/soundcard.h>
#include <errno.h>
#include "../../include/HAL/sound.h"
#include <stdio.h>
#include "../../include/HAL/HAL_Symb.h"
#include <stdlib.h>

int hal_sndinit(shw* hw, int channels, int samp_r) {
	hw->fd = open("/dev/dsp", O_WRONLY);
	if (hw->fd < 0) hw->fd = open("/dev/snd/pcmC0D0p", O_WRONLY);
	
	if (hw->fd < 0) return -1;

	int frag = 0x00060008;
	ioctl(hw->fd, SNDCTL_DSP_SETFRAGMENT, &frag);
	
	int fmt = AFMT_S16_LE;
	int st = (channels == 2);

	ioctl(hw->fd, SNDCTL_DSP_SETFMT, &fmt);
	ioctl(hw->fd, SNDCTL_DSP_STEREO, &st);
	ioctl(hw->fd, SNDCTL_DSP_SPEED, &samp_r);
	
	return 0;
}

int hal_sndwrite(shw* hw, const short* buffer, int samples) {
    	audio_buf_info info;
    	if (ioctl(hw->fd, SNDCTL_DSP_GETOSPACE, &info) == -1) return 0;

	int to_write = samples * sizeof(short);
	if (to_write > info.bytes) to_write = info.bytes;

	int bytes_written = write(hw->fd, buffer, to_write);

	return bytes_written / sizeof(short);

	return write(hw->fd, buffer, to_write / sizeof(short));
}

void hal_setvol(shw* hw, int volume) {
	int mixer_fd = open("/dev/mixer", O_RDWR);
	if (mixer_fd < 0) return;

	int vol = (volume << 8) | volume;
	ioctl(mixer_fd, MIXER_WRITE(SOUND_MIXER_VOLUME), &vol);
    	close(mixer_fd);
}

void hal_sndcls(shw* hw) {
	if (hw->fd >= 0) close(hw->fd);
}
//added Modern Sound Subsystem support
int hal_compsndinit(shw* hw, int channels, int samp_r) {
        hw->fd = open("/dev/snd/pcmC0D0p", O_WRONLY); //modern sound device
        if (hw->fd < 0) return -1;
        return 0;
}

int hal_compsndwrite(shw* hw, const short* buf, int samples) {
        if (hw->fd < 0) return 0;

        int to_write = samples * sizeof(short);
        int bytes_written = write(hw->fd, buf, to_write);

        if (bytes_written < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) return 0;
        return -1;
    }

    return bytes_written / sizeof(short);
}

void hal_compsndcls(shw* hw) {
        if (hw->fd >= 0) {
                close(hw->fd);
                hw->fd = -1;
        }
}


typedef struct {
    hal_dev_t base;
    int (*Init)(shw* hw, int channels, int samp_r);
    int (*Write)(shw* hw, const short* buffer, int samples);
    void (*SetVol)(shw* hw, int volume);
    void (*Close)(shw* hw);
    int (*CompInit)(shw* hw, int channels, int samp_r);
    int (*CompWrite)(shw* hw, const short* buf, int samples);
    void (*CompClose)(shw* hw);
} snd_dev_t;

static int snd_bind_dev(const struct hal_mod_t* module, const char* name, struct hal_dev_t** device) {
    snd_dev_t* dev = malloc(sizeof(snd_dev_t));
    if (!dev) return -1;
    
    dev->base.common = (hal_mod_t*)module;
    dev->Init = hal_sndinit;
    dev->Write = hal_sndwrite;
    dev->SetVol = hal_setvol;
    dev->Close = hal_sndcls;
    dev->CompInit = hal_compsndinit;
    dev->CompWrite = hal_compsndwrite;
    dev->CompClose = hal_compsndcls;
    
    *device = (hal_dev_t*)dev;
    return 0;
}

hal_mod_t HAL_MODULE_INFO_SYM = {
    .id = "sound",
    .version = 1,
    .open = snd_bind_dev
};
