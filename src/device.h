#ifndef __DEVICE
#define __DEVICE
#include <pcap.h>
#include <winsock.h>
#include "packet.h"

#ifdef _LIB
	#define DLL_EXPORT __declspec(dllexport)
#else
	#define DLL_EXPORT __declspec(dllimport)
#endif

class DLL_EXPORT Device
{
public:
	typedef void (*Listener) (Packet*);

private:
	pcap_t *dev;
	Listener listener;

	static pcap_if_t *alldevs;
	static char errbuf[PCAP_ERRBUF_SIZE];
	static pcap_if_t* GetRawByIndex(int);
	static void ip_listener(u_char*, const pcap_pkthdr*, const u_char*);
	static void udp_listener(u_char*, const pcap_pkthdr*, const u_char*);

public:
	typedef void (*Listener) (Packet*);
	Device(pcap_t *);
	Device(const char *);
	~Device();

	void SetFilter(const char*);
	void Listen(Listener);
	void ListenUDP(Listener);
	void Unbind();

	void Send(Packet*);

	static void ListDevices();
	static void FreeDevices();
	static int Count();
	static Device* GetDeviceByIndex(int);
	static const char* GetNameByIndex(int);
};

#endif
