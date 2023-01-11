#ifndef AVDL_MULTIPLAYER_H
#define AVDL_MULTIPLAYER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>

enum avdl_multiplayer_lobbytype {
	AVDL_MULTIPLAYER_LOBBYTYPE_PUBLIC,
	AVDL_MULTIPLAYER_LOBBYTYPE_PRIVATE,
};

struct avdl_multiplayer_identity {
	unsigned long id;
};

struct avdl_multiplayer {
	int x;
};

int avdl_multiplayer_isEnabled();

struct avdl_multiplayer *avdl_multiplayer_create();
void avdl_multiplayer_clean(struct avdl_multiplayer *o);

void avdl_multiplayer_getLobbies(struct avdl_multiplayer *o, void (*callback)(int numOfLobbies));
void avdl_multiplayer_createLobby(struct avdl_multiplayer *o, enum avdl_multiplayer_lobbytype type, int maxPlayers, void (*callback)(int));
void avdl_multiplayer_joinLobby(struct avdl_multiplayer *o, int index, void (*callback)(void));
void avdl_multiplayer_leaveLobby(struct avdl_multiplayer *o, void (*callback)(void));
void avdl_multiplayer_confirmLobby(struct avdl_multiplayer *o);
void avdl_multiplayer_confirmLobby2(struct avdl_multiplayer *o, int numberOfPlayers, unsigned long *playerIdentities, unsigned long *host);

#define avdl_multiplayer_sendMessageToAllPlayers(o, data, class) avdl_multiplayer_sendMessageToAllPlayers_internal(o, data, sizeof(struct class))
void avdl_multiplayer_sendMessageToAllPlayers_internal(struct avdl_multiplayer *o, void *data, size_t dataSize);
#define avdl_multiplayer_sendMessage(o, playerIndex, data, class) avdl_multiplayer_sendMessage_internal(o, playerIndex, data, sizeof(struct class))
void avdl_multiplayer_sendMessage_internal(struct avdl_multiplayer *o, int playerIndex, void *data, size_t dataSize);
#define avdl_multiplayer_sendMessageToHost(o, data, class) avdl_multiplayer_sendMessageToHost_internal(o, data, sizeof(struct class))
void avdl_multiplayer_sendMessageToHost_internal(struct avdl_multiplayer *o, void *data, size_t dataSize);
int avdl_multiplayer_getLobbyMemberCurrent(struct avdl_multiplayer *o, int index);
int avdl_multiplayer_getLobbyMemberLimit(struct avdl_multiplayer *o, int index);

//void avdl_multiplayer_sendMessage(struct avdl_multiplayer *o);
int avdl_multiplayer_receiveMessages(struct avdl_multiplayer *o);
void *avdl_multiplayer_getMessageData(struct avdl_multiplayer *o);
int avdl_multiplayer_getMessageSize(struct avdl_multiplayer *o);
void avdl_multiplayer_releaseMessage(struct avdl_multiplayer *o);

int avdl_multiplayer_isHost(struct avdl_multiplayer *o);

void avdl_multiplayer_getPlayerId(struct avdl_multiplayer *o, int index, struct avdl_multiplayer_identity *id);
void avdl_multiplayer_getHostId(struct avdl_multiplayer *o, struct avdl_multiplayer_identity *hostId);

// test
void avdl_multiplayer_openOverlay(struct avdl_multiplayer *o);
const char *avdl_multiplayer_getSelfName(struct avdl_multiplayer *o);

int avdl_multiplayer_getFriendCount(struct avdl_multiplayer *o);
const char *avdl_multiplayer_getFriendNameIndex(struct avdl_multiplayer *o, int index);
//unsigned long int avdl_multiplayer_getFriendIndex(struct avdl_multiplayer *o);

#ifdef __cplusplus
}
#endif

#endif
