#ifndef CONFIG_H
#define CONFIG_H
enum cfgTypes {
	CFG_BOOL = 'b',
	CFG_INT  = 'i',
	CFG_STR  = 's'
};

typedef struct cfgEntry {
	const char* key;
	int type;
	union {
		int vint;
		bool vbool;
		const char* vchar;
	} value;
	const char* commentary;
	struct cfgEntry* next;
} *CFGENTRY;

typedef struct cfgStore {
	const char* path;
	bool modified;
	CFGENTRY firstCfgEntry;
	CFGENTRY lastCfgEntry;
} *CFGSTORE;

CFGENTRY Config_GetEntry(CFGSTORE store, const char* key);
bool Config_AddComment(CFGSTORE store, const char* commentary);

API CFGSTORE Config_Create(const char* filename);
API void Config_EmptyStore(CFGSTORE store);
API void Config_DestroyStore(CFGSTORE store);
API const char* Config_TypeName(int type);
API int Config_TypeNameToInt(const char* name);

API bool Config_Load(CFGSTORE store);
API bool Config_Save(CFGSTORE store);

API int Config_GetInt(CFGSTORE store, const char* key);
API void Config_SetInt(CFGSTORE store, const char* key, int value);

API const char* Config_GetStr(CFGSTORE store, const char* key);
API void Config_SetStr(CFGSTORE store, const char* key, const char* value);

API void Config_SetBool(CFGSTORE store, const char* key, bool value);
API bool Config_GetBool(CFGSTORE store, const char* key);
#endif
