#ifndef AVDL_JSON_H
#define AVDL_JSON_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stddef.h>

/*
#if defined( AVDL_ANDROID ) || defined( AVDL_QUEST2 )
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#endif
*/

// Tokens
enum AVDL_JSON_TOKEN {

	// Objects
	AVDL_JSON_OBJECT_START,
	AVDL_JSON_OBJECT_END,

	AVDL_JSON_ARRAY_START,
	AVDL_JSON_ARRAY_END,

	// Data types
	AVDL_JSON_KEY,
	AVDL_JSON_STRING,
	AVDL_JSON_INT,
	AVDL_JSON_FLOAT,

	// End of buffer
	AVDL_JSON_EOB,
};

#define AVDL_JSON_BUFFER_SIZE 1000

struct avdl_json_object {

	/*
	 * string parsing
	 */
	char buffer[AVDL_JSON_BUFFER_SIZE];
	char *str;
	int size;

	char *start;
	char *end;

	/*
	 * file parsing
	 */
	FILE *file;
	#if defined( AVDL_ANDROID ) || defined( AVDL_QUEST2 )
	AAsset *file2;
	char *str2;
	#endif
	char *current;

	/*
	 * current token
	 */
	int length;
	enum AVDL_JSON_TOKEN token;

	int hasKey;

	int isNumber;
	int isFloat;
	int number;
	float numberf;

};

void avdl_json_init(struct avdl_json_object *o, char *json_string, int size);
#if defined(_WIN32) || defined(WIN32)
int avdl_json_initFile(struct avdl_json_object *o, wchar_t *filename);
#else
int avdl_json_initFile(struct avdl_json_object *o, char *filename);
#endif

void avdl_json_next(struct avdl_json_object *o);
enum AVDL_JSON_TOKEN avdl_json_getToken(struct avdl_json_object *o);
char *avdl_json_getTokenString(struct avdl_json_object *o);
int avdl_json_getTokenNumber(struct avdl_json_object *o);
float avdl_json_getTokenFloat(struct avdl_json_object *o);
int avdl_json_getTokenSize(struct avdl_json_object *o);

void avdl_json_deinit(struct avdl_json_object *o);

#ifdef __cplusplus
}
#endif

#endif
