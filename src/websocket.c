#include "core.h"
#include "str.h"
#include "platform.h"
#include "websocket.h"
#include "lang.h"
#include "hash.h"

#define WS_RESP "HTTP/1.1 101 Switching Protocols\r\nConnection: Upgrade\r\nUpgrade: websocket\r\nSec-WebSocket-Protocol: %s\r\nSec-WebSocket-Accept: %s\r\n\r\n"
#define WS_ERRRESP "HTTP/1.1 %d %s\r\nConnection: Close\r\nContent-Type: text/plain\r\nContent-Length: %d\r\n\r\n%s"

cs_bool WebSock_DoHandshake(WebSock *ws) {
	if(!ws->proto) return false;
	cs_char line[1024], wskey[32], b64[30];
	cs_byte hash[20];
	cs_bool validConnection = false;
	cs_int32 wskeylen = 0, rsplen = 0;

	if(Socket_ReceiveLine(ws->sock, line, 1024)) {
		cs_str httpver = String_LastChar(line, 'H');
		if(!httpver || !String_CaselessCompare(httpver, "HTTP/1.1")) {
			rsplen = String_FormatBuf(line, 1024, WS_ERRRESP, 505, "HTTP Version Not Supported", 0, "");
			Socket_Send(ws->sock, line, rsplen);
			return false;
		}
	}

	while(Socket_ReceiveLine(ws->sock, line, 1024)) {
		if(*line == '\0') break;

		cs_char *value = (cs_char *)String_FirstChar(line, ':');
		if(!value) break;
		*value = '\0';value += 2;

		if(String_CaselessCompare(line, "Sec-WebSocket-Key")) {
			wskeylen = (cs_int32)String_Copy(wskey, 32, value);
		} else if(String_CaselessCompare(line, "Sec-WebSocket-Version")) {
			if(String_ToInt(value) != 13) break;
		} else if(String_CaselessCompare(line, "Sec-WebSocket-Protocol")) {
			if(!String_FindSubstr(value, ws->proto)) {
				validConnection = false;
				break;
			}
		} else if(String_CaselessCompare(line, "Upgrade")) {
			validConnection = String_CaselessCompare(value, "websocket");
			if(!validConnection) break;
		}
	}

	if(validConnection && wskeylen > 0) {
		SHA_CTX ctx;
		SHA1_Init(&ctx);
		SHA1_Update(&ctx, wskey, wskeylen);
		SHA1_Update(&ctx, "258EAFA5-E914-47DA-95CA-C5AB0DC85B11", 36);
		SHA1_Final(hash, &ctx);
		String_ToB64(hash, 20, b64);

		rsplen = String_FormatBuf(line, 1024, WS_RESP, ws->proto, b64);
		return Socket_Send(ws->sock, line, rsplen) == rsplen;
	}

	cs_str str = Lang_Get(Lang_ErrGrp, 4);
	rsplen = String_FormatBuf(line, 1024, WS_ERRRESP, 400, "Bad request", String_Length(str), str);
	Socket_Send(ws->sock, line, rsplen);
	return false;
}

cs_bool WebSock_ReceiveFrame(WebSock *ws) {
	if(ws->state == WS_ST_DONE)
		ws->state = WS_ST_HDR;

	if(ws->state == WS_ST_HDR) {
		cs_int32 len = Socket_Receive(ws->sock, ws->header, 2, MSG_WAITALL);

		if(len == 2) {
			cs_char plen = ws->header[1] & 0x7F;

			if(ws->header[1] & 0x80) {
				ws->opcode = ws->header[0] & 0x0F;
				ws->done = (ws->header[0] >> 0x07) & 1;
				ws->plen = plen;

				if(plen == 126) {
					ws->state = WS_ST_PLEN;
				} else if(plen < 126) {
					ws->state = WS_ST_MASK;
				} else {
					ws->error = WS_ERR_PAYLOAD_TOO_BIG;
					return false;
				}
			} else {
				ws->error = WS_ERR_MASK;
				return false;
			}
		}
	}

	if(ws->state == WS_ST_PLEN) {
		cs_int32 len = Socket_Receive(ws->sock, (cs_char *)&ws->plen, 2, MSG_WAITALL);

		if(len == 2) {
			ws->plen = ntohs(ws->plen);
			if(ws->plen > 131) {
				ws->error = WS_ERR_PAYLOAD_TOO_BIG;
				return false;
			}
			ws->state = WS_ST_MASK;
		}
	}

	if(ws->state == WS_ST_MASK) {
		if(Socket_Receive(ws->sock, ws->mask, 4, MSG_WAITALL) == 4)
			ws->state = WS_ST_RECVPL;
	}

	if(ws->state == WS_ST_RECVPL) {
		if(ws->plen > 0) {
			cs_int32 len = Socket_Receive(ws->sock, ws->recvbuf, ws->plen, MSG_WAITALL);

			if(len == ws->plen) {
				for(cs_int32 i = 0; i < len; i++) {
					ws->recvbuf[i] ^= ws->mask[i % 4];
				}
			} else {
				ws->error = WS_ERR_PAYLOAD_LEN_MISMATCH;
				return false;
			}
		}

		ws->state = WS_ST_DONE;
		return true;
	}

	ws->error = WS_ERR_UNKNOWN;
	return false;
}

cs_bool WebSock_SendFrame(WebSock *ws, cs_byte opcode, const cs_char *buf, cs_uint16 len) {
	cs_byte hdrlen = 2;
	cs_char hdr[4] = {0, 0, 0, 0};
	hdr[0] = 0x80 | opcode;

	if(len < 126) {
		hdr[1] = (cs_char)len;
	} else if(len < 65535) {
		hdrlen = 4;
		hdr[1] = 126;
		*(cs_uint16 *)&hdr[2] = htons((cs_uint16)len);
	} else return false;

	return Socket_Send(ws->sock, hdr, hdrlen) == hdrlen &&
	Socket_Send(ws->sock, buf, len) == len;
}
