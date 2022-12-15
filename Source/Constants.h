#pragma once

#define GUI_WIDTH		500
#define GUI_HEIGHT		300

#define LEVEL_ID		"level"
#define LEVEL_NAME		"LEVEL"
#define LEVEL_MIN		-12.0f
#define LEVEL_MAX		12.0f
#define LEVEL_DEFAULT	0.0f
#define LEVEL_INTERVAL	0.1f

#define DRIVE_ID		"drive"
#define DRIVE_NAME		"DRIVE"
#define DRIVE_MIN		0.0f
#define DRIVE_MAX		80.0f
#define DRIVE_DEFAULT	40.0f
#define DRIVE_INTERVAL	0.1f

#define XOVER_ID		"xover"
#define XOVER_NAME		"TONE"
#define XOVER_MIN		80.0f
#define XOVER_MAX		750.0f
#define XOVER_DEFAULT	250.0f
#define XOVER_INTERVAL	1.0f

#define HI_LVL_ID		"highlevel"
#define HI_LVL_NAME		"ATTACK"
#define HI_LVL_MIN		-12.0f
#define HI_LVL_MAX		12.0f
#define HI_LVL_DEFAULT	0.0f
#define HI_LVL_INTERVAL	0.1f

#define BLEND_ID		"blend"
#define BLEND_NAME		"BLEND"
#define BLEND_MIN		0.0f
#define BLEND_MAX		1.0f
#define BLEND_DEFAULT	0.5f
#define BLEND_INTERVAL	0.01f

#define UPPER_FREQ		1000.0f
#define BANDPASS_FREQ	650.0f
#define BANDPASS_Q		0.85f