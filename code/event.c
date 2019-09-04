#include "core.h"
#include "client.h"
#include "event.h"

#define rgPart1 \
for(int pos = 0; pos < MAX_EVENTS; pos++) { \
	if(!Event_List[type][pos]) { \
		EVENT* evt = (EVENT*)Memory_Alloc(1, sizeof(EVENT)); \

#define rgPart2 \
		Event_List[type][pos] = evt; \
		return true; \
	} \
} \
return false; \

bool Event_RegisterBool(EventType type, evtBoolCallback func) {
	rgPart1;
	evt->rtype = EVT_RTBOOL;
	evt->func.fbool = func;
	rgPart2;
}

bool Event_RegisterVoid(EventType type, evtVoidCallback func) {
	rgPart1;
	evt->rtype = EVT_RTVOID;
	evt->func.fvoid = func;
	rgPart2;
}

static bool Event_Call(EventType type, void* param) {
	bool ret = true;

	for(int pos = 0; pos < MAX_EVENTS; pos++) {
		EVENT* evt = Event_List[type][pos];
		if(!evt) continue;

		if(evt->rtype == EVT_RTBOOL)
			ret = evt->func.fbool(param);
		else
			evt->func.fvoid(param);

		if(!ret) break;
	}

	return ret;
}

bool Event_OnMessage(CLIENT* client, char* message, MessageType* id) {
	return Event_Call(EVT_ONMESSAGE, (void*)&client);
}

bool Event_OnBlockPlace(CLIENT* client, ushort* x, ushort* y, ushort* z, BlockID* id) {
	return Event_Call(EVT_ONBLOCKPLACE, (void*)&client);
}

void Event_OnHandshakeDone(CLIENT* client) {
	Event_Call(EVT_ONHANDSHAKEDONE, (void*)client);
}

void Event_OnSpawn(CLIENT* client) {
	Event_Call(EVT_ONSPAWN, (void*)client);
}

void Event_OnDespawn(CLIENT* client) {
	Event_Call(EVT_ONDESPAWN, (void*)client);
}

void Event_OnDisconnect(CLIENT* client) {
	Event_Call(EVT_ONDISCONNECT, (void*)client);
}

void Event_OnHeldBlockChange(CLIENT* client, BlockID* prev, BlockID* curr) {
	Event_Call(EVT_ONHELDBLOCKCHNG, (void*)&client);
}

void Event_OnClick(CLIENT* client, char* button, char* action, short* yaw, short* pitch, ClientID* tgID, ushort* tgBlockX, ushort* tgBlockY, ushort* tgblockZ, char* tgBlockFace) {
	Event_Call(EVT_ONPLAYERCLICK, (void*)&client);
}
