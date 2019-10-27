#ifndef HASH_H
#define HASH_H

#if defined(WINDOWS)
#include <wincrypt.h>

#define MD5_DIGEST_LENGTH  16
#define SHA1_DIGEST_LENGTH 20

typedef struct {
	HCRYPTPROV provider;
	HCRYPTHASH hash;
	unsigned long int hashLen;
} SHA_CTX, MD5_CTX, HASH_CTX;

bool SHA1_Init(SHA_CTX* ctx);
bool SHA1_Update(SHA_CTX* ctx, const void* data, size_t len);
bool SHA1_Final(uint8_t* hash, SHA_CTX* ctx);

bool MD5_Init(MD5_CTX* ctx);
bool MD5_Update(MD5_CTX* ctx, const void* data, size_t len);
bool MD5_Final(uint8_t* hash, MD5_CTX* ctx);
#elif defined(POSIX)
#  include <openssl/md5.h>
#  include <openssl/sha1.h>
#endif

#endif
