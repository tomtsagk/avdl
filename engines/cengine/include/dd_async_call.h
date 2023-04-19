#ifndef DD_ASYNC_CALL_H
#define DD_ASYNC_CALL_H

#ifdef __cplusplus
extern "C" {
#endif

// handle async calls
struct dd_async_call {
	void (*callback)(void *context);
	void *context;
	int isComplete;
};

#ifdef __cplusplus
}
#endif

#endif
