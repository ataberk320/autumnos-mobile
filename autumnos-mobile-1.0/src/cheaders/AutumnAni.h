#ifndef ANIM_H
#define ANIM_H

#include <stdbool.h>
#include <stdint.h>

typedef enum {
	ANIM_STATE_IDLE,
	ANIM_STATE_COLD,
	ANIM_STATE_WARM
} AnimState;

typedef struct {
	float scale;
	float target_scale;
	AnimState state;
	uint32_t last_tick;
} AutumnAni;

void AutumnUI_Ani_Init(AutumnAni *animation);

void AutumnUI_Ani_Start(AutumnAni *animation, bool is_warm);

void AutumnUI_Ani_Update(AutumnAni *animation);

void Draw_To_Canvas(uint32_t* buffer, float scale);

void Draw_App_On_Session_Ani(AutumnAni *animation, uint32_t* target_buffer, uint32_t* shm_ptr);

#endif
