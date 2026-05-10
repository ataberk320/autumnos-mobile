#include "cheaders/AutumnAni.h"
#include <stdbool.h>
#include <string.h>
void Draw_To_Canvas(uint32_t* buffer, float scale) {
	int w = (int)(480 * scale);
	int h = (int)(768 * scale);
	int start_x = (480 - w) / 2;
	int start_y = (768 - h) / 2 + (15360 / 480);
	
	for (int y = 0; y < h; y++) {
		for (int x = 0; x < w; x++) {
			buffer[(start_y + y) * 480 + (start_x + x)] = 0xFF000000;
		}
	}
}

void Draw_App_On_Session_Ani(AutumnAni *animation, uint32_t* target_buffer, uint32_t* shm_ptr) {
	if (animation->state == ANIM_STATE_IDLE || shm_ptr ==  NULL) return;
	float s = animation->scale;
	int w = (int)(480 * s);
	int h = (int)(768 * s);
	int start_x = (480 - w) / 2;
	int start_y = (768 - h) / 2 + 32;
	float inv_s = 1.0f / s;

	for (int y = 0; y < h; y++) {
		for (int x = 0; x < w; x++) {
			int src_x = (int)(x * inv_s);
			int src_y = (int)(y * inv_s);
			if (src_x < 480 && src_y < 768) {
				uint32_t pixel = shm_ptr[src_y * 480 + src_x];
				if ((pixel & 0xFF000000) != 0) {
					target_buffer[(start_y + y) * 480 + (start_x + x)] = pixel;
				}
			}
		}
	}
}

void AutumnUI_Ani_Init(AutumnAni *animation) {
	animation->scale = 0.1f;
	animation->target_scale = 1.0f;
	animation->state = ANIM_STATE_IDLE;
}

void AutumnUI_Ani_Start(AutumnAni *animation, bool is_warm) {
	animation->scale = 0.1f;
	animation->state = is_warm ? ANIM_STATE_WARM : ANIM_STATE_COLD;
}

void AutumnUI_Ani_Update(AutumnAni *animation) {
	if (animation->state == ANIM_STATE_IDLE) return;

	if (animation->scale < animation->target_scale) {
		animation->scale += 0.05f;
	}

	if (animation->state == ANIM_STATE_COLD && animation->scale >= 0.8f) {
		animation->scale = 0.8f;
	}

	if (animation->scale > 1.0f) animation->scale = 1.0f;
}
