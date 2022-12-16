#include "avdl_multiplayer.h"
#include "steam_api.h"
#include <stdlib.h>
#include <cstddef>

static CSteamID slobbyId;
static CSteamID lobbyMembers[10];
static int lobbyMembersCount = 0;
static int isInGame = 0;
static SteamNetworkingMessage_t *msg;
static CSteamID host;

class CLobbyListManager {

	public:

	int isInLobby = 0;
	int lobbyId = 0;
	int numberOfLobbies = 0;

	CCallResult< CLobbyListManager, LobbyMatchList_t > m_CallResultLobbyMatchList;
	CCallResult< CLobbyListManager, LobbyCreated_t > m_CallResultLobbyCreated;
	CCallResult< CLobbyListManager, LobbyEnter_t > m_CallResultLobbyEntered;
	CCallResult< CLobbyListManager, LobbyDataUpdate_t > m_CallResultLobbyDataUpdated;

	void (*OnLobbyListRequestCallback)(int) = 0;
	void (*OnLobbyCreatedCallback)(int) = 0;
	void (*OnLobbyEnteredCallback)() = 0;

	void OnLobbyListReceived( LobbyMatchList_t *lobby, bool bIOFailure) {
		printf("avdl: lobby list received: %d\n", lobby->m_nLobbiesMatching);
		numberOfLobbies = lobby->m_nLobbiesMatching;
		if (OnLobbyListRequestCallback) {
			OnLobbyListRequestCallback(lobby->m_nLobbiesMatching);
			OnLobbyListRequestCallback = 0;
		}
	}

	void CreateLobby(enum avdl_multiplayer_lobbytype type, int max_players) {
		printf("creating lobby\n");
		ELobbyType steamType;

		if (type == AVDL_MULTIPLAYER_LOBBYTYPE_PRIVATE) {
			steamType = k_ELobbyTypePrivate;
		}
		// default public mode for all new lobbies
		else {
			steamType = k_ELobbyTypePublic;
		}

		SteamAPICall_t callback = SteamMatchmaking()->CreateLobby(steamType, max_players);
		m_CallResultLobbyCreated.Set(callback, this, &CLobbyListManager::OnLobbyCreated);
		m_CallResultLobbyEntered.Set(callback, this, &CLobbyListManager::OnLobbyEntered);
		m_CallResultLobbyDataUpdated.Set(callback, this, &CLobbyListManager::OnLobbyDataUpdated);
	}

	void OnLobbyCreated( LobbyCreated_t *lobby, bool bIOFailure ) {
		int success = 0;
		printf("avdl: lobby created\n");
		if (lobby->m_eResult == k_EResultOK) {
			isInLobby = 1;
			lobbyId = lobby->m_ulSteamIDLobby;
			slobbyId = lobby->m_ulSteamIDLobby;
			printf("avdl: lobbyid: %u\n", slobbyId);
			//printf("avdl: lobbyid str: %s\n", slobbyId.Render());
			printf("lobby creation was successful with id: %d\n", lobbyId);
			success = 1;
		}
		else
		if (lobby->m_eResult == k_EResultFail) {
			printf("lobby creation has failed\n");
		}
		else
		if (lobby->m_eResult == k_EResultTimeout) {
			printf("lobby creation timeout\n");
		}
		else
		if (lobby->m_eResult == k_EResultLimitExceeded) {
			printf("too many lobbies created by this client\n");
		}
		else
		if (lobby->m_eResult == k_EResultAccessDenied) {
			printf("lobby creation access denied\n");
		}
		else
		if (lobby->m_eResult == k_EResultNoConnection) {
			printf("lobby creation no connection\n");
		}

		if (OnLobbyCreatedCallback) {
			OnLobbyCreatedCallback(success);
			OnLobbyCreatedCallback = 0;
		}
	}

	void OnLobbyEntered( LobbyEnter_t *lobby, bool bIOFailure ) {
		printf("avdl: lobby entered\n");
		printf("avdl: chat room: %d\n", lobby->m_EChatRoomEnterResponse);
		if (lobby->m_EChatRoomEnterResponse != k_EChatRoomEnterResponseSuccess) {
			printf("error entering lobby: %d\n", lobby->m_EChatRoomEnterResponse);
			return;
		}
		isInLobby = 1;
		slobbyId = lobby->m_ulSteamIDLobby;
		printf("avdl: lobbyid: %u\n", slobbyId);
		//printf("avdl: lobbyid str: %s\n", slobbyId.Render());
		// lobby id
		//lobby->m_ulSteamIDLobby;

		// invite only
		//lobby->m_bLocked;

		// lobby chat
		//lobby->m_EChatRoomEnterResponse;

		if (lobby->m_EChatRoomEnterResponse == k_EChatRoomEnterResponseSuccess) {
			printf("join chat room - success\n");
		}
		else
		if (lobby->m_EChatRoomEnterResponse == k_EChatRoomEnterResponseDoesntExist) {
			printf("join chat room - doesn't exist\n");
		}
		else
		if (lobby->m_EChatRoomEnterResponse == k_EChatRoomEnterResponseNotAllowed) {
			printf("join chat room - not allowed\n");
		}
		else
		if (lobby->m_EChatRoomEnterResponse == k_EChatRoomEnterResponseFull) {
			printf("join chat room - full\n");
		}
		else
		if (lobby->m_EChatRoomEnterResponse == k_EChatRoomEnterResponseError) {
			printf("join chat room - error\n");
		}
		else
		if (lobby->m_EChatRoomEnterResponse == k_EChatRoomEnterResponseBanned) {
			printf("join chat room - you are banned from this chat room\n");
		}
		else
		if (lobby->m_EChatRoomEnterResponse == k_EChatRoomEnterResponseLimited) {
			printf("join chat room - Limited user, can't join\n");
		}
		else
		if (lobby->m_EChatRoomEnterResponse == k_EChatRoomEnterResponseClanDisabled) {
			printf("join chat room - Attempt to join a clan chat, when clan is locked/disabled\n");
		}
		else
		if (lobby->m_EChatRoomEnterResponse == k_EChatRoomEnterResponseCommunityBan) {
			printf("join chat room - can't join community lock\n");
		}
		else
		if (lobby->m_EChatRoomEnterResponse == k_EChatRoomEnterResponseMemberBlockedYou) {
			printf("join chat room - A user in the chat has blocked you\n");
		}
		else
		if (lobby->m_EChatRoomEnterResponse == k_EChatRoomEnterResponseYouBlockedMember) {
			printf("join chat room - A user you've blocked is in chat room\n");
		}

		if (OnLobbyEnteredCallback) {
			OnLobbyEnteredCallback();
			OnLobbyEnteredCallback = 0;
		}
	}

	void OnLobbyDataUpdated( LobbyDataUpdate_t *lobby, bool bIOFailure ) {
		printf("avdl: lobby data updated\n");
		if (!lobby->m_bSuccess) {
			printf("avdl: error updating lobby\n");
			return;
		}
		//slobbyId = lobby->m_ulSteamIDLobby;
		printf("avdl: lobbyid: %u\n", slobbyId);
		//slobbyId.SetFromUint64(lobby->m_ulSteamIDLobby);
		//printf("avdl: lobbyid: %lu\n", lobby->m_ulSteamIDLobby);
		//printf("avdl: lobbyid str: %s\n", slobbyId.Render());
	}

	STEAM_CALLBACK( CLobbyListManager, OnMessageRequestReceived, SteamNetworkingMessagesSessionRequest_t );

	//void OnMessageRequestReceived(SteamNetworkingMessagesSessionRequest_t *t);
};

void CLobbyListManager::OnMessageRequestReceived(SteamNetworkingMessagesSessionRequest_t *t) {
	printf("avdl: message request received and will be accepted\n");
	SteamNetworkingMessages()->AcceptSessionWithUser( t->m_identityRemote );
}

static CLobbyListManager *m = 0;

int avdl_multiplayer_isEnabled() {
	return 1;
}

struct avdl_multiplayer *avdl_multiplayer_create() {
	/*
	if (!SteamUserStats()->RequestCurrentStats()) {
		printf("avdl: failed to initialise achievements\n");
	}
	*/
	//printf("avdl: getting lobby list\n");
	//SteamAPICall_t callback = SteamMatchmaking()->RequestLobbyList();
	//m_CallResultLobbyMatchList.Set(callback, 0, OnLobbyListReceived);
	printf("avdl: multiplayer_create\n");
	slobbyId = CSteamID();
	if (!m) {
		m = new CLobbyListManager();
	}
	isInGame = 0;
	msg = 0;
	host = k_steamIDNil;
	return 0;
}

void OnLobbyListReceived( LobbyMatchList_t *lobby, bool bIOFailure) {
	printf("avdl: lobby list received\n");
}

void avdl_multiplayer_clean(struct avdl_multiplayer *o) {
}

void avdl_multiplayer_createLobby(struct avdl_multiplayer *o, enum avdl_multiplayer_lobbytype type, int maxPlayers, void (*callback)(int)) {
	if (callback) {
		m->OnLobbyCreatedCallback = callback;
	}
	m->CreateLobby(type, maxPlayers);
}

void avdl_multiplayer_getLobbies(struct avdl_multiplayer *o, void (*callback)(int numOfLobbies)) {
	if (callback) {
		m->OnLobbyListRequestCallback = callback;
	}
	printf("avdl: getting lobby list\n");
	SteamAPICall_t call = SteamMatchmaking()->RequestLobbyList();
	m->m_CallResultLobbyMatchList.Set(call, m, &CLobbyListManager::OnLobbyListReceived);
}

void avdl_multiplayer_joinLobby(struct avdl_multiplayer *o, int index, void (*callback)(void)) {

	// first get lobby id from its index
	CSteamID lobbyId = SteamMatchmaking()->GetLobbyByIndex(index);
	if (lobbyId == k_steamIDNil) {
		printf("avdl: error joining lobby, can't get id of lobby: %d\n", index);
		return;
	}

	if (callback) {
		m->OnLobbyEnteredCallback = callback;
	}
	printf("avdl: attempting to join lobby with index/id: %d/%u\n", index, lobbyId);
	SteamAPICall_t call = SteamMatchmaking()->JoinLobby(lobbyId);
	m->m_CallResultLobbyEntered.Set(call, m, &CLobbyListManager::OnLobbyEntered);
	m->m_CallResultLobbyDataUpdated.Set(call, m, &CLobbyListManager::OnLobbyDataUpdated);
}

int avdl_multiplayer_getLobbyMemberCurrent(struct avdl_multiplayer *o, int index) {

	/*
	// first get lobby id from its index
	CSteamID lobbyId = SteamMatchmaking()->GetLobbyByIndex(index);
	if (lobbyId == k_steamIDNil) {
		printf("avdl: error getting lobby member current, can't get id of lobby: %d\n", index);
		return 0;
	}
	*/

	if (!isInGame) {
		printf("avdl: lobbyid: %u\n", slobbyId);
		return SteamMatchmaking()->GetNumLobbyMembers(slobbyId);
	}
	else {
		return lobbyMembersCount;
	}

}

int avdl_multiplayer_getLobbyMemberLimit(struct avdl_multiplayer *o, int index) {

	// first get lobby id from its index
	CSteamID lobbyId = SteamMatchmaking()->GetLobbyByIndex(index);
	if (lobbyId == k_steamIDNil) {
		printf("avdl: error getting lobby limit, can't get id of lobby: %d\n", index);
		return 0;
	}

	return SteamMatchmaking()->GetLobbyMemberLimit(lobbyId);

}

//void avdl_multiplayer_sendMessage(struct avdl_multiplayer *o) {
//
//	/*
//	// first get lobby id from its index
//	CSteamID lobbyId = SteamMatchmaking()->GetLobbyByIndex(0);
//	if (lobbyId == k_steamIDNil) {
//		printf("avdl: error sending message, can't get id of lobby: %d\n");
//		return;
//	}
//	*/
//
//	int members = SteamMatchmaking()->GetNumLobbyMembers(slobbyId);
//	printf("avdl: Members: %d\n", members);
//
//	printf("avdl: about to send messages\n");
//	for (int i = 0; i < members; i++) {
//		printf("avdl: message %d\n", i);
//		//SendMessageToUser( user, pubData, cubData, flags, channel );
//		CSteamID id = SteamMatchmaking()->GetLobbyMemberByIndex(slobbyId, i);
//		printf("avdl: sending to %d\n", id);
//		if (id == k_steamIDNil) {
//			printf("avdl: skipping message as it's null\n");
//			continue;
//		}
//		if (id == SteamUser()->GetSteamID()) {
//			printf("avdl: skipping message to self\n");
//			continue;
//		}
//		SteamNetworkingIdentity toUser;
//		toUser.SetSteamID(id);
//
//		char *data = "Hello world steam";
//		SteamNetworkingMessages()->SendMessageToUser( toUser, data, strlen(data) +1, 0, 0 );
//	}
//}

void avdl_multiplayer_sendMessageToAllPlayers_internal(struct avdl_multiplayer *o, void *data, size_t dataSize) {

	/*
	// first get lobby id from its index
	CSteamID lobbyId = SteamMatchmaking()->GetLobbyByIndex(0);
	if (lobbyId == k_steamIDNil) {
		printf("avdl: error sending message, can't get id of lobby: %d\n");
		return;
	}
	*/

	if (!isInGame) {
		int members = SteamMatchmaking()->GetNumLobbyMembers(slobbyId);
		printf("avdl: Members: %d\n", members);

		printf("avdl: about to send messages\n");
		for (int i = 0; i < members; i++) {
			printf("avdl: message %d\n", i);
			//SendMessageToUser( user, pubData, cubData, flags, channel );
			CSteamID id = SteamMatchmaking()->GetLobbyMemberByIndex(slobbyId, i);
			printf("avdl: sending to %d\n", id);
			if (id == k_steamIDNil) {
				printf("avdl: skipping message as it's null\n");
				continue;
			}
			if (id == SteamUser()->GetSteamID()) {
				printf("avdl: skipping message to self\n");
				continue;
			}
			SteamNetworkingIdentity toUser;
			toUser.SetSteamID(id);

			SteamNetworkingMessages()->SendMessageToUser( toUser, data, dataSize, 0, 0 );
		}
	}
	else {
		printf("avdl: about to send messages\n");
		for (int i = 0; i < lobbyMembersCount; i++) {
			printf("avdl: message %d\n", i);
			CSteamID id = lobbyMembers[i];
			if (id == SteamUser()->GetSteamID()) {
				printf("avdl: skipping message to self\n");
				continue;
			}
			printf("avdl: sending to %d\n", id);
			SteamNetworkingIdentity toUser;
			toUser.SetSteamID(id);
			//SendMessageToUser( user, pubData, cubData, flags, channel );
			SteamNetworkingMessages()->SendMessageToUser( toUser, data, dataSize, 0, 0 );
		}
	}
}

void avdl_multiplayer_sendMessage_internal(struct avdl_multiplayer *o, int playerIndex, void *data, size_t dataSize) {

	int members;
	if (!isInGame) {
		members = avdl_multiplayer_getLobbyMemberCurrent(0, 0);
	}
	else {
		members = lobbyMembersCount;
	}
	printf("avdl: Members: %d\n", members);
	if (playerIndex >= members) {
		printf("avdl: failed to send single message, invalid player id: %d | members total: %d\n", playerIndex, members);
	}
	printf("avdl: about to send single message\n");
	CSteamID id;
	if (!isInGame) {

		id = SteamMatchmaking()->GetLobbyMemberByIndex(slobbyId, playerIndex);
		if (id == k_steamIDNil) {
			printf("avdl: skipping message as it's null\n");
			return;
		}
		if (id == SteamUser()->GetSteamID()) {
			printf("avdl: skipping message to self\n");
			return;
		}
	}
	else {
		id = lobbyMembers[playerIndex];
		if (id == SteamUser()->GetSteamID()) {
			printf("avdl: skipping message to self\n");
			return;
		}
	}
	printf("avdl: sending to %d\n", id);
	SteamNetworkingIdentity toUser;
	toUser.SetSteamID(id);
	//SendMessageToUser( user, pubData, cubData, flags, channel );
	SteamNetworkingMessages()->SendMessageToUser( toUser, data, dataSize, 0, 0 );
}

void avdl_multiplayer_sendMessageToHost_internal(struct avdl_multiplayer *o, void *data, size_t dataSize) {

	if (!isInGame) {
	}
	else {
		if (host == SteamUser()->GetSteamID()) {
			printf("avdl: skipping message to self\n");
			return;
		}
		printf("avdl: sending msg to host: %d\n", host);
		SteamNetworkingIdentity toUser;
		toUser.SetSteamID(host);
		SteamNetworkingMessages()->SendMessageToUser( toUser, data, dataSize, 0, 0 );
	}

}

void avdl_multiplayer_sendMessageToAllPlayersInGame_internal(struct avdl_multiplayer *o, void *data, size_t dataSize) {

}

int avdl_multiplayer_receiveMessages(struct avdl_multiplayer *o) {
	avdl_multiplayer_releaseMessage(0);
	//SteamNetworkingMessages()->ReceiveMessagesOnChannel(channel, steamMessage, maxMessages);
	//SteamNetworkingMessage_t *msg;// = (SteamNetworkingMessage_t*) malloc(sizeof(SteamNetworkingMessage_t) *5);
	int messages = SteamNetworkingMessages()->ReceiveMessagesOnChannel(0, &msg, 1);

	printf("avdl: members in lobby: %d\n", avdl_multiplayer_getLobbyMemberCurrent(0, 0));

	if (messages <= 0) {
		//printf("avdl: no messages: %d\n", messages);
		return 0;
	}

	// host syncs message with everyone else
	if (avdl_multiplayer_isHost(0)) {

		if (!isInGame) {
			/*
			int members = SteamMatchmaking()->GetNumLobbyMembers(slobbyId);
			printf("avdl: Members: %d\n", members);
	
			printf("avdl: about to send messages\n");
			for (int i = 0; i < members; i++) {
				printf("avdl: message %d\n", i);
				//SendMessageToUser( user, pubData, cubData, flags, channel );
				CSteamID id = SteamMatchmaking()->GetLobbyMemberByIndex(slobbyId, i);
				printf("avdl: sending to %d\n", id);
				if (id == k_steamIDNil) {
					printf("avdl: skipping message as it's null\n");
					continue;
				}
				if (id == SteamUser()->GetSteamID()) {
					printf("avdl: skipping message to self\n");
					continue;
				}
				SteamNetworkingIdentity toUser;
				toUser.SetSteamID(id);
	
				SteamNetworkingMessages()->SendMessageToUser( toUser, data, dataSize, 0, 0 );
			}
			*/
		}
		else {
			printf("avdl: about to sync messages\n");
			for (int i = 0; i < lobbyMembersCount; i++) {
				printf("avdl: message %d\n", i);
				CSteamID id = lobbyMembers[i];
				if (id == SteamUser()->GetSteamID()) {
					printf("avdl: skipping message to self\n");
					continue;
				}
				printf("avdl: syncing to %d\n", id);
				SteamNetworkingIdentity toUser;
				toUser.SetSteamID(id);
				if (msg) {
					if (id == msg->m_identityPeer.GetSteamID()) {
						printf("avdl: skipping sync message to same user\n");
						continue;
					}
				}
				//SendMessageToUser( user, pubData, cubData, flags, channel );
				SteamNetworkingMessages()->SendMessageToUser( toUser, avdl_multiplayer_getMessageData(0), avdl_multiplayer_getMessageSize(0), 0, 0 );
			}
		}

	}

	printf("avdl: got messages: %d\n", messages);
	return messages;

	/*
	do {
		printf("avdl: got messages: %d\n", messages);
		//printf("reading message: %s\n", msg->m_pData);
		//msg->Release();
		//messages = SteamNetworkingMessages()->ReceiveMessagesOnChannel(0, &msg, 1);
	} while (messages > 0);
	*/
	//free(msg);
}

void *avdl_multiplayer_getMessageData(struct avdl_multiplayer *o) {
	if (msg) {
		return msg->m_pData;
	}
	return 0;
}

int avdl_multiplayer_getMessageSize(struct avdl_multiplayer *o) {
	if (msg) {
		return msg->m_cbSize;
	}
	return 0;
}

void avdl_multiplayer_releaseMessage(struct avdl_multiplayer *o) {
	if (msg) {
		msg->Release();
		msg = 0;
	}
}

void avdl_multiplayer_leaveLobby(struct avdl_multiplayer *o, void (*callback)(void)) {
	if (slobbyId != k_steamIDNil) {
		SteamMatchmaking()->LeaveLobby(slobbyId);
		slobbyId = k_steamIDNil;
		printf("just left lobby\n");
	}
}

void avdl_multiplayer_confirmLobby(struct avdl_multiplayer *o) {

	int members = SteamMatchmaking()->GetNumLobbyMembers(slobbyId);
	printf("avdl: Members when confirming lobby: %d\n", members);

	if (members > 10) {
		printf("avdl: error confirming lobby - too many members: %d", lobbyMembersCount);
		return;
	}

	isInGame = 1;
	lobbyMembersCount = 0;
	for (int i = 0; i < members; i++) {
		printf("avdl: confirm id: %d\n", i);
		CSteamID id = SteamMatchmaking()->GetLobbyMemberByIndex(slobbyId, i);
		printf("avdl: adding member %d\n", id);
		if (id == k_steamIDNil) {
			printf("avdl: skip adding null member to game\n");
			continue;
		}
		/*
		if (id == SteamUser()->GetSteamID()) {
			printf("avdl: skip adding self to game\n");
			continue;
		}
		*/
		lobbyMembers[lobbyMembersCount] = SteamMatchmaking()->GetLobbyMemberByIndex(slobbyId, i);
		lobbyMembersCount++;
	}
	host = SteamMatchmaking()->GetLobbyOwner(slobbyId);
	printf("avdl: setting host: %d\n", host);
	avdl_multiplayer_leaveLobby(o, 0);
}

int avdl_multiplayer_isHost(struct avdl_multiplayer *o) {
	return host != k_steamIDNil && host == SteamUser()->GetSteamID();
}

void avdl_multiplayer_getPlayerId(struct avdl_multiplayer *o, int index, struct avdl_multiplayer_identity *identity) {
	if (!isInGame) {
		int members = SteamMatchmaking()->GetNumLobbyMembers(slobbyId);
		CSteamID id = SteamMatchmaking()->GetLobbyMemberByIndex(slobbyId, index);
		//printf("avdl: returning player id: %u\n", id.ConvertToUint64());
		identity->id = id.ConvertToUint64();
	}
	else {
		CSteamID id = lobbyMembers[index];
		//printf("avdl: returning player id: %u\n", id.ConvertToUint64());
		identity->id = id.ConvertToUint64();
	}
}

void avdl_multiplayer_getHostId(struct avdl_multiplayer *o, struct avdl_multiplayer_identity *hostId) {
	if (!isInGame) {
		/*
		int members = SteamMatchmaking()->GetNumLobbyMembers(slobbyId);
		CSteamID id = SteamMatchmaking()->GetLobbyMemberByIndex(slobbyId, index);
		//printf("avdl: returning player id: %u\n", id.ConvertToUint64());
		identity->id = id.ConvertToUint64();
		*/
	}
	else {
		hostId->id = host.ConvertToUint64();
	}
}

void avdl_multiplayer_confirmLobby2(struct avdl_multiplayer *o, int numberOfPlayers, unsigned long *playerIdentities, unsigned long *hostId) {

	int members = lobbyMembersCount;
	printf("avdl: Members when confirming lobby: %d\n", members);

	if (members > 10) {
		printf("avdl: error confirming lobby - too many members: %d", members);
		return;
	}

	isInGame = 1;
	lobbyMembersCount = 0;
	for (int i = 0; i < members; i++) {
		printf("avdl: confirm id: %d\n", i);
		CSteamID id;
		id.SetFromUint64(playerIdentities[i]);
		printf("avdl: adding member %d\n", id);
		if (id == k_steamIDNil) {
			printf("avdl: skip adding null member to game\n");
			continue;
		}
		/*
		if (id == SteamUser()->GetSteamID()) {
			printf("avdl: skip adding self to game\n");
			continue;
		}
		*/
		lobbyMembers[lobbyMembersCount] = id;
		lobbyMembersCount++;
	}
	host.SetFromUint64(*hostId);
	printf("avdl: setting host: %d\n", host);
	printf("avdl: setting hostId: %d\n", *hostId);
	avdl_multiplayer_leaveLobby(o, 0);
}

void avdl_multiplayer_openOverlay(struct avdl_multiplayer *o) {
	printf("avdl: about to open steam overlay\n");
	SteamFriends()->ActivateGameOverlay("friends");
}

const char *avdl_multiplayer_getSelfName(struct avdl_multiplayer *o) {
	return SteamFriends()->GetPersonaName();
}

int avdl_multiplayer_getFriendCount(struct avdl_multiplayer *o) {
	return SteamFriends()->GetFriendCount(k_EFriendFlagImmediate);
}

const char *avdl_multiplayer_getFriendNameIndex(struct avdl_multiplayer *o, int index) {
	if (index > avdl_multiplayer_getFriendCount(o)) {
		printf("avdl: get friend name index: invalid index: %d\n", index);
		return "";
	}
	CSteamID friendId = SteamFriends()->GetFriendByIndex(index, k_EFriendFlagImmediate);
	const char *friendName = SteamFriends()->GetFriendPersonaName(friendId);
	printf("avdl: friend name: %s\n", friendName);
	return SteamFriends()->GetFriendPersonaName(friendId);
}
