#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/soundcard.h>
#include <errno.h>
#include "sound.h"
#include <stdio.h>

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


