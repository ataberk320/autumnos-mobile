#include <stdlib.h>
#include <unistd.h>
#define MINIMP3_IMPLEMENTATION
#include "minimp3_ex.h"
#include "sound.h"
#include "table.h"
#include <tinyalsa/mixer.h>

static struct mixer *mixer = NULL;

static struct {
	mp3dec_file_info_t info;
	int playing;
	int paused;
	short* pcm_cursor;
	long samples_left;
} ctx;

static shw hw;

int AutumnAPI_LoadMp3(const char* path) {
	mp3dec_t mp3d;
	mp3dec_file_info_t new_info;

	if (mp3dec_load(&mp3d, path, &new_info, NULL, NULL) != 0) return -1;

        if (snd->InitSound(&hw, new_info.channels, new_info.hz) != 0) {
		perror("HAL - InitSound");
		return -2;
	}
	ctx.info = new_info;
	ctx.pcm_cursor = ctx.info.buffer;
	ctx.samples_left = ctx.info.samples;
	ctx.playing = 1;
	ctx.paused = 0;

	printf("MP3 sample rate: %d Hz\n", ctx.info.hz);
        printf("channels: %d\n", ctx.info.channels);
        printf("supported total sample: %d\n", (int)ctx.info.samples);

	snd->SetVol(&hw, 70);
	return 0;
}

void AutumnAPI_PlayTime(int* elapsed, int* total) {
	long current_samples = ctx.pcm_cursor - (short*)ctx.info.buffer;
	int samples_per_second = ctx.info.hz * ctx.info.channels;

	if (samples_per_second > 0) {
        	*elapsed = (int)(current_samples / samples_per_second);
        	*total = (int)(ctx.info.samples / samples_per_second);
    	} 	
	else {
        	*elapsed = 0;
        	*total = 0;
    	}
}
//Integrated *sndev parameter for double-compatible audio playing :)
int AutumnAPI_PlayAudio(const char *sndev) {
    if (!ctx.playing || ctx.paused) return 0;

    if (ctx.samples_left > 0) {
                int written = 0;

                if (sndev && strstr(sndev, "dsp") != NULL) {
                        written = snd->WritePCM(&hw, ctx.pcm_cursor, ctx.samples_left);
                }
                else {
                        written = snd->CompSndWrite(&hw, ctx.pcm_cursor, ctx.samples_left);
                }

                if (written > 0) {
                        ctx.pcm_cursor += written;
                        ctx.samples_left -= written;
                        return 0;
                }
                else {
                        return 0;
                }
    }
    else {
        ctx.playing = 0;
        printf("\nPlaying completed!\n");
    }
    return 0;
}

int AutumnAPI_StopAudio(void) {
	ctx.playing = 0;
	ctx.paused = 0;

	ctx.pcm_cursor = ctx.info.buffer;
	ctx.samples_left = ctx.info.samples;
	printf("Playing process returned 0!");
	return 0;
}

int AutumnAPI_Pause(void) {
	if (ctx.playing) ctx.paused = 1;
	return 0;
}


int AutumnAPI_Resume(void) {
	if (ctx.playing) ctx.paused = 0;
	return 0;
}


int AutumnAPI_Replay(void) {
	if (!ctx.playing) return -1;
    	ctx.pcm_cursor = ctx.info.buffer;
    	ctx.samples_left = ctx.info.samples;
    	ctx.paused = 0;
    	return 0;
}

void AutumnAPI_PCMCleanup(void) {
    snd->CloseSound(&hw);
    ctx.playing = 0;
}	

int AutumnAPI_GetPlayingStatus(void) {
	return ctx.playing;
}

void AutumnAPI_SetVolume(int volume) {
	snd->SetVol(&hw, volume);
}
