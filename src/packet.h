#ifndef __PACKET
#define __PACKET
#include "protocol.h"

#ifdef _LIB
	#define DLL_EXPORT __declspec(dllexport)
#else
	#define DLL_EXPORT __declspec(dllimport)
#endif

class DLL_EXPORT Packet
{
protected:
	size_t len;
	u_char* data;
	bool release;
public:
	static const size_t EthernetLength = sizeof(Ethernet);
	static const u_long ID = 0x48D8;

	Packet(void*, size_t);
	Packet(size_t);
	~Packet();

	void* Raw();
	Ethernet* EthernetHeader();
	IP* IPHeader();
	void* Data();

	size_t Length();
	size_t IPLength();
	size_t HeaderLength();
	size_t DataLength();

	u_short CalcIPChecksum();
};

class DLL_EXPORT UDPPacket : public Packet
{
public:
	static const size_t UDPLength = sizeof(UDP);

	UDPPacket(void*, size_t);
	UDPPacket(size_t);
	UDP* UDPHeader();
	void* Data();

	size_t HeaderLength();
	size_t DataLength();

	u_short CalcUDPChecksum();
	void CreatePacket(u_char[6], u_char[6], u_char[4], u_char[4], u_short, u_short, void*);
};

#endif
