#include "avdl_multiplayer.h"

int avdl_multiplayer_isEnabled() {
	return 0;
}

struct avdl_multiplayer *avdl_multiplayer_create() {
	return 0;
}

void avdl_multiplayer_clean(struct avdl_multiplayer *o) {
}

void avdl_multiplayer_createLobby(struct avdl_multiplayer *o, enum avdl_multiplayer_lobbytype type, int maxPlayers, void (*callback)(int)) {
	if (callback) {
		callback(0);
	}
}

void avdl_multiplayer_getLobbies(struct avdl_multiplayer *o, void (*callback)(int numOfLobbies)) {
	if (callback) {
		callback(0);
	}
}

void avdl_multiplayer_joinLobby(struct avdl_multiplayer *o, int index, void (*callback)(void)) {
	if (callback) {
		callback();
	}
}

int avdl_multiplayer_getLobbyMemberCurrent(struct avdl_multiplayer *o, int index) {
	return 0;
}

int avdl_multiplayer_getLobbyMemberLimit(struct avdl_multiplayer *o, int index) {
	return 0;
}

void avdl_multiplayer_sendMessage_internal(struct avdl_multiplayer *o, int playerIndex, void *data, size_t dataSize) {
}

int avdl_multiplayer_receiveMessages(struct avdl_multiplayer *o) {
}

void avdl_multiplayer_leaveLobby(struct avdl_multiplayer *o, void (*callback)(void)) {
}

void avdl_multiplayer_sendMessageToAllPlayers_internal(struct avdl_multiplayer *o, void *data, size_t dataSize) {
}

void avdl_multiplayer_sendMessageToAllPlayersInGame_internal(struct avdl_multiplayer *o, void *data, size_t dataSize) {
}

void avdl_multiplayer_confirmLobby(struct avdl_multiplayer *o) {
}

int avdl_multiplayer_isHost(struct avdl_multiplayer *o) {
	return 0;
}

void avdl_multiplayer_getPlayerId(struct avdl_multiplayer *o, int index, struct avdl_multiplayer_identity *id) {
}

void avdl_multiplayer_getHostId(struct avdl_multiplayer *o, struct avdl_multiplayer_identity *hostId) {
}

void avdl_multiplayer_confirmLobby2(struct avdl_multiplayer *o, int numberOfPlayers, unsigned long *playerIdentities, unsigned long *host) {
}

void avdl_multiplayer_openOverlay(struct avdl_multiplayer *o) {
}

const char *avdl_multiplayer_getSelfName(struct avdl_multiplayer *o) {
	return "";
}

int avdl_multiplayer_getFriendCount(struct avdl_multiplayer *o) {
	return 0;
}

const char *avdl_multiplayer_getFriendNameIndex(struct avdl_multiplayer *o, int index) {
	return "";
}
