#define _LIB
#include "tcp.h"

extern WSADATA TCPSocket::wsa_data;

void TCPSocket::Startup()
{
	WSAStartup(Version, &wsa_data);
}

void TCPSocket::Clean()
{
	WSACleanup();
}

TCPSocket::TCPSocket()
{
	sock = socket(AF_INET, SOCK_STREAM, 0);
}

TCPSocket::~TCPSocket()
{
	Close();
}

TCPSocket::TCPSocket(SOCKET s) : sock(s)
{}

SOCKADDR_IN TCPSocket::GetAddress(const char* ip, u_short port)
{
	SOCKADDR_IN ret;
	ret.sin_addr.S_un.S_addr = inet_addr(ip);
	ret.sin_family = AF_INET;
	ret.sin_port = htons(port);
	return ret;
}

void TCPSocket::Listen(const char* ip, u_short port, Listener listener)
{
	int len = sizeof(SOCKADDR_IN);
	SOCKADDR_IN addr = GetAddress(ip, port);

	bind(sock, (LPSOCKADDR)&addr, len);
	listen(sock, MaxConnection);
	exit = false;
	while (!exit)
	{
		TCPSocket* conn = new TCPSocket(accept(sock, (LPSOCKADDR)&addr, &len));
		while (conn -> Recv())
			listener(conn);
		delete conn;
	}
}

void TCPSocket::Connect(const char* ip, u_short port)
{
	exit = false;
	SOCKADDR_IN addr = GetAddress(ip, port);
	connect(sock, (LPSOCKADDR)&addr, sizeof(SOCKADDR_IN));
}

int TCPSocket::Recv()
{
	if (exit)
		return -1;
	ZeroMemory(buffer, BufferSize);
	return recv(sock, buffer, BufferSize, 0);
}

void TCPSocket::Send(const char* data, size_t len)
{
	send(sock, data, len, 0);
}

void TCPSocket::Close()
{
	exit = true;
	closesocket(sock);
}

bool TCPSocket::Closed()
{
	return exit;
}

const char* TCPSocket::Data()
{
	return buffer;
}
