#ifndef DD_GAMEJOLT_H
#define DD_GAMEJOLT_H
//
//#include "dd_json.h"
//
//// base URL
//#define DD_GAMEJOLT_BASE "https://api.gamejolt.com/api/game/v1_2/"
//
//// Scores functionality
//#define DD_GAMEJOLT_SCORES DD_GAMEJOLT_BASE "scores/"
//
//#define DD_GAMEJOLT_SCORES_ADD       DD_GAMEJOLT_SCORES "add/"
//#define DD_GAMEJOLT_SCORES_GETRANK   DD_GAMEJOLT_SCORES "get-rank/"
//#define DD_GAMEJOLT_SCORES_FETCH     DD_GAMEJOLT_SCORES ""
//#define DD_GAMEJOLT_SCORES_GETTABLES DD_GAMEJOLT_SCORES "tables/"
//
//#define DD_GAMEJOLT_SCORES_FETCH_BETTER_THAN  1
//#define DD_GAMEJOLT_SCORES_FETCH_WORSE_THAN   2
//#define DD_GAMEJOLT_SCORES_FETCH_BETTER_WORSE 3
//
///*
// * Pass the game's secret key to activate gamejolt features for that game
// * Passing `0` will deactivate it.
// *
// */
//void dd_gamejolt_activate(char *localGameKey, char *localGameId);
//int dd_gamejolt_isActive();
//
///*
// * Handling user
// */
//void dd_gamejolt_userSet(char *username, char *user_token);
//void dd_gamejolt_guestSet();
//void dd_gamejolt_customGuestSet(char *guestname);
//
//char *dd_gamejolt_getGuestname();
//char *dd_gamejolt_getUsername();
//char *dd_gamejolt_getUsertoken();
//char *dd_gamejolt_getName();
//int dd_gamejolt_hasUser();
//
//extern struct dd_gamejolt_response_score {
//	char score[20];
//	int sort;
//	char user[20];
//	int user_id;
//	char guest[20];
//	char stored[100];
//	int stored_timestamp;
//} x;
//
///*
// * response handling
// */
//extern struct dd_gamejolt_response_struct {
//	int success;
//	char message[DD_JSON_BUFFER_SIZE];
//	int rank;
//	struct dd_gamejolt_response_score scores[10];
//	int scoreCount;
//
//} structReturnData;
//
//void dd_gamejolt_response_struct_create(struct dd_gamejolt_response_struct *);
//
///*
// * Add score to a table
// *
// * Calling `setTableId` will set a specific table for all function calls following.
// * If another function is called before `setTableId`, it will assume the
// * main table of the game, defined by GameJolt.
// *
// * `setTableId` - Set which table is currently active (0 for none)
// * `add` - Add the given score to the active table
// * `getRank` - Get the position of the given score in the active table,
// * 	if no score matching, return the nearest
// */
//void dd_gamejolt_score_setTableId(char *tableId);
//
//void dd_gamejolt_score_add(int score, void (*callback)(void*), void *context, struct dd_gamejolt_response_struct *data);
//void dd_gamejolt_score_getRank(int score, void (*callback)(void*), void *context, struct dd_gamejolt_response_struct *data);
//void dd_gamejolt_score_fetch(int setting, int betterThan, int worseThan, int limit, void (*callback)(void*), void *context, struct dd_gamejolt_response_struct *data);
//
//// Getters for scores
//int dd_gamejolt_response_getSuccess(struct dd_gamejolt_response_struct *);
//char *dd_gamejolt_response_getMessage(struct dd_gamejolt_response_struct *);
//int dd_gamejolt_response_getScoreCount(struct dd_gamejolt_response_struct *);
//char *dd_gamejolt_response_getScore (struct dd_gamejolt_response_struct *, int scoreIndex);
//int   dd_gamejolt_response_getSort  (struct dd_gamejolt_response_struct *, int scoreIndex);
//char *dd_gamejolt_response_getUser  (struct dd_gamejolt_response_struct *, int scoreIndex);
//int   dd_gamejolt_response_getUserId(struct dd_gamejolt_response_struct *, int scoreIndex);
//char *dd_gamejolt_response_getGuest (struct dd_gamejolt_response_struct *, int scoreIndex);
//char *dd_gamejolt_response_getStored(struct dd_gamejolt_response_struct *, int scoreIndex);
//int   dd_gamejolt_response_getStoredTimestamp(struct dd_gamejolt_response_struct *, int scoreIndex);
//int   dd_gamejolt_response_getRank(struct dd_gamejolt_response_struct *);
//char *dd_gamejolt_response_getName(struct dd_gamejolt_response_struct *, int scoreIndex);
//int dd_gamejolt_response_hasUser(struct dd_gamejolt_response_struct *, int scoreIndex);
//
void nofunc();
#endif
