#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "io.h"

#define MINIMP4_IMPLEMENTATION
#include "minimp4.h"

extern int _mp4_RdCallback(int64_t offset, void *buffer, size_t size, void *token);

static struct {
	int fd;
	void *buf;
	MP4D_demux_t mp4;
	unsigned int cur_frm;
	unsigned int tot_frm;
	int playing;
	int paused;
	int width;
	int height;
} vctx;

int AutumnAPI_LoadVid(const char *path) {
	memset(&vctx, 0, sizeof(vctx));

	vctx.fd = _AutumnSys_ioOpen(path, 0, 0);
	if (vctx.fd < 0) {
		perror("Video - _AutumnSys_ioOpen");
		return -1;
	}

	off_t size = _AutumnSys_ioLseek(vctx.fd, 0, SEEK_END);
	if (size < 0) {
		_AutumnSys_ioClose(vctx.fd);
		return -2;
	}

	_AutumnSys_ioLseek(vctx.fd, 0, SEEK_SET);

	vctx.buf = malloc(size);
	if (!vctx.buf) {
		_AutumnSys_ioClose(vctx.fd);
		return -3;
	}

	ssize_t tot_rd = 0;
	while (tot_rd < size) {
		ssize_t r = _AutumnSys_ioRead(vctx.fd, (char*)vctx.buf + tot_rd, size - tot_rd);
        	if (r <= 0) break;
        	tot_rd += r;
    	}
	
	if (!MP4D_open(&vctx.mp4, _mp4_RdCallback, &vctx.fd, size)) {
		perror("MP4 Parsing");
		free(vctx.buf);
		_AutumnSys_ioClose(vctx.fd);
		return -4;
	}

	vctx.cur_frm = 0;
    	vctx.tot_frm = vctx.mp4.track_count > 0 ? vctx.mp4.track[0].sample_count : 0;
	vctx.width = vctx.mp4.track_count > 0 ? vctx.mp4.track[0].SampleDescription.video.width : 0;
	vctx.height = vctx.mp4.track_count > 0 ? vctx.mp4.track[0].SampleDescription.video.height : 0;
    	vctx.playing = 1;
    	vctx.paused = 0;

    	printf("Video Resolution: %dx%d\n", vctx.width, vctx.height);
    	printf("Total Frames: %u\n", vctx.tot_frm);
}

void AutumnAPI_GetVidPlayTime(int* elapsed, int* total) {
	int fps = 30;
	if (fps > 0) {
		*elapsed = vctx.cur_frm / fps;
		*total = vctx.tot_frm / fps;
	}
	
	else {
		*elapsed = 0;
		*total = 0;
	}
}

int AutumnAPI_GetNextVidFrm(void *buf, size_t frm_siz, size_t *out_size) {
        if (!vctx.playing || vctx.paused) return 0;

        if (vctx.cur_frm < vctx.tot_frm) {
                unsigned int frame_bytes, timestamp, duration;
                MP4D_file_offset_t offset = MP4D_frame_offset(&vctx.mp4, 0, vctx.cur_frm, &frame_bytes, &timestamp, &duration);

                if (frame_bytes > 0) {
                        if (frame_bytes > frm_siz) return -1;

                        if (vctx.fd < 0) return -1;

                        if (_AutumnSys_ioLseek(vctx.fd, offset, SEEK_SET) < 0) return 0;
                        ssize_t r = _AutumnSys_ioRead(vctx.fd, buf, frame_bytes);
                        
                        if (r <= 0) return -1;

                        *out_size = (size_t)r;
                        vctx.cur_frm++;
                        return 1;
                }
        }
        else {
                vctx.playing = 0;
                printf("\nVideo playback completed!\n");
        }
        return 0;
}

int AutumnAPI_StopVid(void) {
	vctx.playing = 0;
	vctx.paused = 0;
	vctx.cur_frm = 0;

	MP4D_close(&vctx.mp4);

	if (vctx.buf) {
		free(vctx.buf);
		vctx.buf = NULL;
	}

	if (vctx.fd >= 0) {
		_AutumnSys_ioClose(vctx.fd);
		vctx.fd = -1;
	}

	printf("Video stopped!\n");
	return 0;
}

int AutumnAPI_PauseVid(void) {
    	if (vctx.playing) vctx.paused = 1;
    	return 0;
}

int AutumnAPI_ResumeVid(void) {
    	if (vctx.playing) vctx.paused = 0;
    	return 0;
}

int AutumnAPI_ReplayVid(void) {
    	if (!vctx.buf) return -1;
    	vctx.cur_frm = 0;
    	vctx.paused = 0;
    	vctx.playing = 1;
    	return 0;
}

int AutumnAPI_GetVidStat(void) {
    	return vctx.playing && !vctx.paused;
}
