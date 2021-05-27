#ifndef BLOCK_H
#define BLOCK_H

#include "../Hash/sha256.h"
#include "transactions.h"

#include <stdbool.h>  


typedef struct
{
	BYTE previusHash[SHA256_BLOCK_SIZE];
	TRANSACTIONS_LIST tl;
	BYTE blockHash[SHA256_BLOCK_SIZE];
} BLOCK;

typedef struct
{
	BYTE *bin;
	size_t nbBytes;
} BLOCK_BIN;

void getMerkleHash(BLOCK *b, BYTE merkleHash[SHA256_BLOCK_SIZE]);
void getHash(BLOCK *b, BYTE hash[SHA256_BLOCK_SIZE]);
char *blockToJson(BLOCK *b);
BLOCK_BIN blockToBin(BLOCK *b);
BLOCK binToBlock(BYTE *bin);
size_t getSizeOf_blockbin(size_t nbTxs);
bool isGenesis(BLOCK *b);

#endif
