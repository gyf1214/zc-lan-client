#ifndef __TCP
#define __TCP

#include <windows.h>
#include <winsock.h>


#ifdef _LIB
	#define DLL_EXPORT __declspec(dllexport)
#else
	#define DLL_EXPORT __declspec(dllimport)
#endif

class DLL_EXPORT TCPSocket
{
public:
	static const int BufferSize = 65536;
	static const int MaxConnection = 10;
	static const u_short Version = 0x0101;

private:
	static WSADATA wsa_data;
	SOCKET sock;
	char buffer[BufferSize];
	bool exit;

public:
	typedef void(*Listener)(TCPSocket*);

	static void Startup();
	static void Clean();
	static SOCKADDR_IN GetAddress(const char*, u_short);

	TCPSocket();
	TCPSocket(SOCKET);
	~TCPSocket();

	void Listen(const char*, u_short, Listener);
	void Connect(const char*, u_short);
	void Send(const char*, size_t);
	int Recv();
	void Close();
	bool Closed();
	const char* Data();
};

#endif
