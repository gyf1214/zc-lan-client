#include <cstdlib>
#include <cstdio>
#include <windows.h>
#include "tcp.h"

void Handler(TCPSocket* conn)
{
	printf("%s\n", conn -> Data());
}

int main()
{
	TCPSocket::Startup();

	char ip[] = "0.0.0.0";
	u_short port = 8888;

	TCPSocket* ts = new TCPSocket();
	ts -> Listen(ip, port, Handler);
	delete ts;

	TCPSocket::Clean();
}
