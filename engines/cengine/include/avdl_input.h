#ifndef AVDL_INPUT_H
#define AVDL_INPUT_H

struct AvdlMouseInput {
	int button;
	int state;
};

struct AvdlInput {
	int input_mouse;
	struct AvdlMouseInput mouse_input[10];
	int mouse_input_total;
	int loc_x;
	int loc_y;
};

void avdl_input_Init(struct AvdlInput *);
int avdl_input_GetInputTotal(struct AvdlInput *);
int avdl_input_GetButton(struct AvdlInput *, int index);
int avdl_input_GetState(struct AvdlInput *, int index);
int avdl_input_GetX(struct AvdlInput *);
int avdl_input_GetY(struct AvdlInput *);
int avdl_input_ClearInput(struct AvdlInput *);

int avdl_input_AddInput(struct AvdlInput *, int button, int state, int x, int y);
int avdl_input_AddPassiveMotion(struct AvdlInput *, int x, int y);

#endif
