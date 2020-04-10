#ifndef HTTP_H
#define HTTP_H

#if defined(WINDOWS)
#include <wininet.h>
HINTERNET hInternet;

typedef struct Http {
	HINTERNET conn, req;
	cs_bool https;
} Http;
#elif defined(POSIX)

#endif

void Http_Init(void);
void Http_Uninit(void);

API cs_bool Http_Open(Http *http, cs_str domain);
API cs_bool Http_Request(Http *http, cs_str method, cs_str url);
API cs_ulong Http_ReadResponse(Http *http, char *buf, cs_ulong sz);
API void Http_Cleanup(Http *http);
#endif // HTTP_H
