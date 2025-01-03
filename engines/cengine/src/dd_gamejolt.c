//#include "dd_gamejolt.h"
//#include <stdio.h>
//#include <stdlib.h>
//#include <curl/curl.h>
//#include <string.h>
//#include <openssl/sha.h>
//#include "dd_math.h"
//#include "dd_json.h"
//#include <pthread.h>
//#include "dd_async_call.h"
//#include "avdl_log.h"
//
///*
// * Data needed for GameJolt functionality
// */
//static char *gameSecretKey = 0;
//static char *gameId = 0;
//static char *gameTableId = 0;
//
//static char *gameUsername = 0;
//static char *gameUsertoken = 0;
//static char *gameGuestname = 0;
//
//static char guestNameRandom[] = "Guest00000";
//
//static void randomise_guest_name() {
//	gameGuestname = guestNameRandom;
//	for (int i = 5; i < 10; i++) {
//		guestNameRandom[i] = '0' +dd_math_rand(10);
//	}
//}
//
//static int isGuestActive = 1;
//
///*
// * Functions to activate GameJolt, and check if its active
// */
//void dd_gamejolt_activate(char *localGameKey, char *localGameId) {
//	gameSecretKey = localGameKey;
//	gameId = localGameId;
//
//	// When activated, assume guest mode until a user is given
//	randomise_guest_name();
//	dd_gamejolt_guestSet();
//}
//
//void dd_gamejolt_userSet(char *username, char *user_token) {
//	gameUsername = username;
//	gameUsertoken = user_token;
//	isGuestActive = 0;
//}
//
//void dd_gamejolt_guestSet() {
//	isGuestActive = 1;
//}
//
//void dd_gamejolt_customGuestSet(char *guestname) {
//	gameGuestname = guestname;
//	isGuestActive = 1;
//}
//
//int dd_gamejolt_isActive() {
//	return gameSecretKey != 0;
//}
//
///*
// * user getters
// */
//char *dd_gamejolt_getGuestname() {
//	return gameGuestname;
//}
//
//char *dd_gamejolt_getUsername() {
//	if (!isGuestActive) {
//		return gameUsername;
//	}
//	else {
//		return "No username";
//	}
//}
//
//char *dd_gamejolt_getUsertoken() {
//	if (gameUsertoken) {
//		return gameUsertoken;
//	}
//	else {
//		return "No user token";
//	}
//}
//
//char *dd_gamejolt_getName() {
//	if (!isGuestActive) {
//		return gameUsername;
//	}
//	else {
//		return gameGuestname;
//	}
//}
//
//int dd_gamejolt_hasUser() {
//	return !isGuestActive;
//}
//
///*
// * Functions for the GameJolt's Score system
// */
//void dd_gamejolt_score_setTableId(char *tableId) {
//	gameTableId = tableId;
//}
//
//struct dd_gamejolt_response_struct structReturnData;
//
//static size_t returnData(char *buffer, size_t size, size_t nmemb, void *userp) {
//
//	// Json object
//	struct dd_json_object jsonObj;
//	dd_json_init(&jsonObj, buffer, nmemb);
//
//	// First master object
//	dd_json_next(&jsonObj);
//	if (dd_json_getToken(&jsonObj) != DD_JSON_OBJECT_START) {
//		return 0;
//	}
//
//	// Response key
//	dd_json_next(&jsonObj);
//	if (dd_json_getToken(&jsonObj) != DD_JSON_KEY
//	||  strcmp(dd_json_getTokenString(&jsonObj), "response")) {
//		return 0;
//	}
//
//	// Response's object
//	dd_json_next(&jsonObj);
//	if (dd_json_getToken(&jsonObj) != DD_JSON_OBJECT_START) {
//		return 0;
//	}
//
//	// Response data
//	while (dd_json_next(&jsonObj), dd_json_getToken(&jsonObj) == DD_JSON_KEY) {
//
//		// Success
//		if (strcmp(dd_json_getTokenString(&jsonObj), "success") == 0) {
//
//			// Get success value, ensure its valid
//			dd_json_next(&jsonObj);
//			if (dd_json_getToken(&jsonObj) != DD_JSON_STRING) {
//				return 0;
//			}
//
//			// check if true or false
//			structReturnData.success = strcmp(dd_json_getTokenString(&jsonObj), "true") == 0;
//		}
//		else
//		// Message
//		if (strcmp(dd_json_getTokenString(&jsonObj), "message") == 0) {
//
//			// Get the message itself, ensure its valid
//			dd_json_next(&jsonObj);
//			if (dd_json_getToken(&jsonObj) != DD_JSON_STRING) {
//				return 0;
//			}
//
//			// Pass the value
//			strcpy(structReturnData.message, dd_json_getTokenString(&jsonObj));
//		}
//		else
//		// Rank
//		if (strcmp(dd_json_getTokenString(&jsonObj), "rank") == 0) {
//
//			// Get the rank, ensure its valid
//			dd_json_next(&jsonObj);
//			if (dd_json_getToken(&jsonObj) != DD_JSON_INT) {
//				return 0;
//			}
//
//			// Pass the value
//			structReturnData.rank = atoi(dd_json_getTokenString(&jsonObj));
//		}
//		else
//		// Scores
//		if (strcmp(dd_json_getTokenString(&jsonObj), "scores") == 0) {
//
//			dd_json_next(&jsonObj);
//
//			// No scores returns
//			if (dd_json_getToken(&jsonObj) == DD_JSON_STRING) {
//				structReturnData.scoreCount = 0;
//				continue;
//			}
//
//			// array of scores
//			if (dd_json_getToken(&jsonObj) != DD_JSON_ARRAY_START) {
//				return 0;
//			}
//
//			// parse each score
//			int currentScore = 0;
//			while (dd_json_next(&jsonObj), dd_json_getToken(&jsonObj) == DD_JSON_OBJECT_START) {
//
//				if (currentScore >= 10) {
//					return 0;
//				}
//
//				// parse each score's values
//				while (dd_json_next(&jsonObj), dd_json_getToken(&jsonObj) == DD_JSON_KEY) {
//
//					// Human-readable score
//					if (strcmp(dd_json_getTokenString(&jsonObj), "score") == 0) {
//						dd_json_next(&jsonObj);
//						if (dd_json_getToken(&jsonObj) != DD_JSON_STRING) {
//							return 0;
//						}
//						strncpy(structReturnData.scores[currentScore].score, dd_json_getTokenString(&jsonObj), 20);
//						structReturnData.scores[currentScore].score[19] = '\0';
//					}
//					// Integer score
//					else
//					if (strcmp(dd_json_getTokenString(&jsonObj), "sort") == 0) {
//						dd_json_next(&jsonObj);
//						if (dd_json_getToken(&jsonObj) != DD_JSON_STRING) {
//							return 0;
//						}
//						structReturnData.scores[currentScore].sort = atoi(dd_json_getTokenString(&jsonObj));
//					}
//					else
//					// Username
//					if (strcmp(dd_json_getTokenString(&jsonObj), "user") == 0) {
//						dd_json_next(&jsonObj);
//						if (dd_json_getToken(&jsonObj) != DD_JSON_STRING) {
//							return 0;
//						}
//						strncpy(structReturnData.scores[currentScore].user, dd_json_getTokenString(&jsonObj), 20);
//						structReturnData.scores[currentScore].user[19] = '\0';
//					}
//					else
//					// User Id
//					if (strcmp(dd_json_getTokenString(&jsonObj), "user_id") == 0) {
//						dd_json_next(&jsonObj);
//						if (dd_json_getToken(&jsonObj) != DD_JSON_STRING) {
//							return 0;
//						}
//						structReturnData.scores[currentScore].user_id = atoi(dd_json_getTokenString(&jsonObj));
//					}
//					else
//					// Guest name
//					if (strcmp(dd_json_getTokenString(&jsonObj), "guest") == 0) {
//						dd_json_next(&jsonObj);
//						if (dd_json_getToken(&jsonObj) != DD_JSON_STRING) {
//							return 0;
//						}
//						strncpy(structReturnData.scores[currentScore].guest, dd_json_getTokenString(&jsonObj), 20);
//						structReturnData.scores[currentScore].guest[19] = '\0';
//					}
//					else
//					// Human readable stored date
//					if (strcmp(dd_json_getTokenString(&jsonObj), "stored") == 0) {
//						dd_json_next(&jsonObj);
//						if (dd_json_getToken(&jsonObj) != DD_JSON_STRING) {
//							return 0;
//						}
//						strncpy(structReturnData.scores[currentScore].stored, dd_json_getTokenString(&jsonObj), 20);
//						structReturnData.scores[currentScore].stored[99] = '\0';
//					}
//					else
//					// Int stored date
//					if (strcmp(dd_json_getTokenString(&jsonObj), "stored_timestamp") == 0) {
//						dd_json_next(&jsonObj);
//						if (dd_json_getToken(&jsonObj) != DD_JSON_INT) {
//							return 0;
//						}
//						structReturnData.scores[currentScore].stored_timestamp = atoi(dd_json_getTokenString(&jsonObj));
//					}
//					// Unknown data
//					else {
//						dd_json_next(&jsonObj);
//					}
//				} // for each score's keys
//
//				if (dd_json_getToken(&jsonObj) != DD_JSON_OBJECT_END) {
//					return 0;
//				}
//
//				currentScore++;
//			} // for each score
//
//			structReturnData.scoreCount = currentScore;
//
//			if (dd_json_getToken(&jsonObj) != DD_JSON_ARRAY_END) {
//				return 0;
//			}
//		}
//		// Unknown key - ignore it and its value
//		else {
//			dd_json_next(&jsonObj);
//		}
//
//	} // while parsing response keys
//
//	// Response's end
//	if (dd_json_getToken(&jsonObj) != DD_JSON_OBJECT_END) {
//		return 0;
//	}
//	dd_json_next(&jsonObj);
//
//	// JSON's end
//	if (dd_json_getToken(&jsonObj) != DD_JSON_OBJECT_END) {
//		return 0;
//	}
//
//	return nmemb;
//}
//
//static struct dd_gamejolt_thread_data {
//	void (*callback)(void *);
//	void *context;
//	struct dd_gamejolt_response_struct *respData;
//	char url[256];
//	char data[256];
//} threadData;
//
//extern struct dd_async_call dd_asyncCall;
//extern int dd_isAsyncCallActive;
//extern pthread_mutex_t asyncCallMutex;
//static void *submit_thread(void *srcdata) {
//	struct dd_gamejolt_thread_data *data = (struct dd_gamejolt_thread_data*) srcdata;
//
//	CURL *curl = curl_easy_init();
//	if (curl) {
//		// supply url and data
//		curl_easy_setopt(curl, CURLOPT_URL, data->url);
//		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data->data);
//		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, returnData); 
//		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
//
//		// send them
//		CURLcode res = curl_easy_perform(curl);
//		if (res != CURLE_OK) {
//			avdl_log("avdl: gamejolt_submit(): curl_easy_perform() failed: %s",
//				curl_easy_strerror(res));
//		}
//		else {
//			//memcpy(data->respData, &structReturnData, sizeof(struct dd_gamejolt_response_struct));
//			data->respData[0] = structReturnData;
//			/*
//			for (int i = 0; i < 10; i++) {
//				avdl_log("date: %d", data->respData->scores[i].stored_timestamp);
//				//data->respData->scores[i] = structReturnData.scores[i];
//			}
//			*/
//		}
//		curl_easy_cleanup(curl);
//	}
//
//	// Pass the callback results to the main thread
//	pthread_mutex_lock(&asyncCallMutex);
//	dd_asyncCall.callback = (void (*)(void*)) data->callback;
//	dd_asyncCall.context = data->context;
//	dd_asyncCall.isComplete = 1;
//	pthread_mutex_unlock(&asyncCallMutex);
//
//	pthread_exit(NULL);
//
//	return 0;
//}
//
///*
// * Send formatted data to gamejolt, using threads
// */
//static void dd_gamejolt_submit(const char *url, const char *data, void (*callback)(void*), void *context, struct dd_gamejolt_response_struct *respData) {
//	pthread_t thread;
//	threadData.callback = callback;
//	threadData.context = context;
//	threadData.respData = respData;
//	strncpy(threadData.url, url, 256);
//	threadData.url[255] = '\0';
//	strncpy(threadData.data, data, 256);
//	threadData.data[255] = '\0';
//	dd_asyncCall.isComplete = 0;
//	dd_isAsyncCallActive = 1;
//	pthread_create(&thread, NULL, submit_thread, &threadData);
//	pthread_detach(thread);
//}
//
///*
// * Hash the given string, and save the result to `buffer`
// */
//static void dd_gamejolt_sha1(unsigned char *buffer, char *src) {
//	SHA_CTX context;
//
//	if(!SHA1_Init(&context)) {
//		avdl_log("avdl: error initialising SHA1");
//		exit(-1);
//	}
//	SHA1_Update(&context, src, strlen(src));
//	if(!SHA1_Final(buffer, &context)) {
//		avdl_log("avdl: error finalising SHA1");
//		exit(-1);
//	}
//}
//
///*
// * Format the URL and the parameters, hash them to get the signature,
// * and submit everyting to the server
// */
//static void dd_gamejolt_presubmit(const char *url, char *parameters[], int parameters_count, void (*callback)(void*), void *context, struct dd_gamejolt_response_struct *respData) {
//
//	/*
//	 * `postFields` - contains all the parameters to be sent as POST data
//	 * `postSignature` - contains the base url and all the parameters together, in order to generate the signature
//	 */
//	char postFields[1024];
//	postFields[0] = '\0';
//	char postSignature[1024];
//	postSignature[0] = '\0';
//	strcat(postSignature, url);
//
//	/*
//	 * For each couple of parameters,
//	 * add them to `postFields` as "(&)param1=param2"
//	 * add them to `postSignature` as "param1param2"
//	 */
//	int includeAmbersand = 0;
//	for (int i = 0; i < parameters_count; i += 2) {
//
//		if (!parameters[i+1]) {
//			continue;
//		}
//
//		// First the fields
//		if (includeAmbersand) {
//			strcat(postFields, "&");
//		}
//		else {
//			includeAmbersand = 1;
//		}
//		strcat(postFields, parameters[i]);
//		strcat(postFields, "=");
//		strcat(postFields, parameters[i+1]);
//
//		// Then the signature
//		strcat(postSignature, parameters[i]);
//		strcat(postSignature, parameters[i+1]);
//	}
//
//	// add secret key to generate the signature
//	strcat(postSignature, gameSecretKey);
//
//	/*
//	 * Calculate sha1 hash of signature string
//	 */
//	unsigned char sha1_digest[SHA_DIGEST_LENGTH];
//	dd_gamejolt_sha1(sha1_digest, postSignature);
//
//	/*
//	 * Add the hashed signature as the last field in `postFields`
//	 */
//	strcat(postFields, "&signature=");
//	for(int n = 0; n < SHA_DIGEST_LENGTH; n++) {
//		char hex[10];
//		sprintf(hex, "%02x", sha1_digest[n]);
//		strcat(postFields, hex);
//	}
//
//	/*
//	avdl_log("post fields: %s", postFields);
//	avdl_log("url %s", url);
//	avdl_log("post signat: %s", postSignature);
//	*/
//
//	// Submit request
//	dd_gamejolt_submit(url, postFields, callback, context, respData);
//}
//
//void dd_gamejolt_score_add(int score, void (*callback)(void*), void *context, struct dd_gamejolt_response_struct *respData) {
//
//	/*
//	 * Calculate the parameters, based on the given score,
//	 * and submit them
//	 */
//	char *parameters[] = {
//		"game_id", gameId,
//		"guest", isGuestActive ? gameGuestname : 0,
//		"score", 0,
//		"sort", 0,
//		"table_id", gameTableId,
//		"user_token", !isGuestActive ? gameUsertoken : 0,
//		"username", !isGuestActive ? gameUsername : 0,
//	};
//	const unsigned int parameters_count = sizeof(parameters) /sizeof(char *);
//
//	// Convert score's sort value to string
//	char sortString[100];
//	snprintf(sortString, 100, "%d", score);
//
//	// Convert score to readable string format
//	char scoreString[100];
//	snprintf(scoreString, 100, "%d", score);
//
//	// Modify parameters
//	parameters[1] = gameId; // GameId
//	parameters[5] = scoreString; // score string
//	parameters[7] = sortString; // sort int
//
//	dd_gamejolt_presubmit(DD_GAMEJOLT_SCORES_ADD, parameters, parameters_count, callback, context, respData);
//}
//
//void dd_gamejolt_score_getRank(int score, void (*callback)(void*), void *context, struct dd_gamejolt_response_struct *respData) {
//
//	/*
//	 * Calculate the parameters, based on the given score,
//	 * and submit them
//	 */
//	char *parameters[] = {
//		"game_id", gameId,
//		"sort", 0,
//		"table_id", gameTableId,
//	};
//	const unsigned int parameters_count = sizeof(parameters) /sizeof(char *);
//
//	// Convert score's sort value to string
//	char sortString[100];
//	snprintf(sortString, 100, "%d", score);
//
//	// Modify parameters
//	parameters[3] = sortString; // sort int
//
//	dd_gamejolt_presubmit(DD_GAMEJOLT_SCORES_GETRANK, parameters, parameters_count, callback, context, respData);
//}
//
//void dd_gamejolt_score_fetch(int setting, int betterThan, int worseThan, int limit, void (*callback)(void*), void *context, struct dd_gamejolt_response_struct *respData) {
//
//	char betterString[20];
//	char worseString[20];
//	snprintf(betterString, 20, "%d", betterThan);
//	snprintf(worseString , 20, "%d", worseThan);
//	char limitString[20];
//	snprintf(limitString, 20, "%d", limit);
//
//	/*
//	 * Calculate the parameters and submit them
//	 */
//	char *parameters[] = {
//		"better_than", (setting & DD_GAMEJOLT_SCORES_FETCH_BETTER_THAN) ? betterString : 0,
//		"game_id", gameId,
//		//"guest", gameGuestname,
//		"limit", limitString,
//		"table_id", gameTableId,
//		//"user_token", gameUsertoken,
//		//"username", gameUsername,
//		"worse_than", (setting & DD_GAMEJOLT_SCORES_FETCH_WORSE_THAN) ? worseString : 0,
//	};
//	const unsigned int parameters_count = sizeof(parameters) /sizeof(char *);
//
//	dd_gamejolt_presubmit(DD_GAMEJOLT_SCORES_FETCH, parameters, parameters_count, callback, context, respData);
//}
//
//int dd_gamejolt_response_getScoreCount(struct dd_gamejolt_response_struct *o) {
//	return o->scoreCount;
//}
//
//char *dd_gamejolt_response_getScore(struct dd_gamejolt_response_struct *o, int scoreIndex) {
//	if (scoreIndex >= 0 && scoreIndex < 10) {
//		return o->scores[scoreIndex].score;
//	}
//	else {
//		return 0;
//	}
//}
//
//int dd_gamejolt_response_getSort(struct dd_gamejolt_response_struct *o, int scoreIndex) {
//	if (scoreIndex >= 0 && scoreIndex < 10) {
//		return o->scores[scoreIndex].sort;
//	}
//	else {
//		return 0;
//	}
//}
//
//char *dd_gamejolt_response_getUser(struct dd_gamejolt_response_struct *o, int scoreIndex) {
//	if (scoreIndex >= 0 && scoreIndex < 10) {
//		return o->scores[scoreIndex].user;
//	}
//	else {
//		return 0;
//	}
//}
//
//int dd_gamejolt_response_getUserId(struct dd_gamejolt_response_struct *o, int scoreIndex) {
//	if (scoreIndex >= 0 && scoreIndex < 10) {
//		return o->scores[scoreIndex].user_id;
//	}
//	else {
//		return 0;
//	}
//}
//
//char *dd_gamejolt_response_getGuest(struct dd_gamejolt_response_struct *o, int scoreIndex) {
//	if (scoreIndex >= 0 && scoreIndex < 10) {
//		return o->scores[scoreIndex].guest;
//	}
//	else {
//		return 0;
//	}
//}
//
//char *dd_gamejolt_response_getStored(struct dd_gamejolt_response_struct *o, int scoreIndex) {
//	if (scoreIndex >= 0 && scoreIndex < 10) {
//		return o->scores[scoreIndex].stored;
//	}
//	else {
//		return 0;
//	}
//}
//
//int dd_gamejolt_response_getStoredTimestamp(struct dd_gamejolt_response_struct *o, int scoreIndex) {
//	if (scoreIndex >= 0 && scoreIndex < 10) {
//		return o->scores[scoreIndex].stored_timestamp;
//	}
//	else {
//		return 0;
//	}
//}
//
//char *dd_gamejolt_response_getName(struct dd_gamejolt_response_struct *o, int scoreIndex) {
//	if (scoreIndex >= 0 && scoreIndex < 10) {
//		if (strcmp(o->scores[scoreIndex].user, "") != 0) {
//			return o->scores[scoreIndex].user;
//		}
//		else {
//			return o->scores[scoreIndex].guest;
//		}
//	}
//	return "Incorrect index";
//}
//
//int dd_gamejolt_response_hasUser(struct dd_gamejolt_response_struct *o, int scoreIndex) {
//	if (scoreIndex >= 0 && scoreIndex < 10) {
//		if (strcmp(o->scores[scoreIndex].user, "") != 0) {
//			return 1;
//		}
//	}
//	return 0;
//}
//
//int dd_gamejolt_response_getRank(struct dd_gamejolt_response_struct *o) {
//	return o->rank;
//}
//
//void dd_gamejolt_response_struct_create(struct dd_gamejolt_response_struct *o) {
//	o->success = 0;
//}
//
//int dd_gamejolt_response_getSuccess(struct dd_gamejolt_response_struct *o) {
//	return o->success;
//}
//
//char *dd_gamejolt_response_getMessage(struct dd_gamejolt_response_struct *o) {
//	return o->message;
//}
void nofunc() {}
