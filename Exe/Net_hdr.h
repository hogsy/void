#ifndef VOID_NETWORK_PRIVATE_HDR
#define VOID_NETWORK_PRIVATE_HDR

#include <winsock2.h>
#include <ws2tcpip.h>
#include "Com_defs.h"
#include "Net_util.h"

void PrintSockError(int err=0,const char *msg=0);

#endif