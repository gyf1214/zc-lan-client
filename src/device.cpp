#define _LIB
#include "device.h"

pcap_if_t* Device::alldevs;
char Device::errbuf[PCAP_ERRBUF_SIZE];

Device::Device(pcap_t* t)
{
	dev = t;
}

Device::Device(const char* name)
{
	if ((dev = pcap_open(name,
		65536,
		PCAP_OPENFLAG_PROMISCUOUS,
		1000,
		NULL,
		errbuf)) == NULL)
		throw "Open Device Failed!";
}

Device::~Device()
{
	if (dev)
		pcap_close(dev);
}

void Device::ListDevices()
{
	if (pcap_findalldevs(&alldevs, errbuf) == -1)
		throw "List Device Failed!";
}

void Device::FreeDevices()
{
	if (alldevs)
		pcap_freealldevs(alldevs);
}

int Device::Count()
{
	int ans = 0;
	for (pcap_if_t *j = alldevs; j; j = j -> next)
		++ans;
	return ans;
}

pcap_if_t* Device::GetRawByIndex(int index)
{
	pcap_if_t* j = alldevs;
	for (; index > 0 && j; --index)
		j = j -> next;
	if (!j)
		throw "Index Overflow!";
	return j;
}

Device* Device::GetDeviceByIndex(int index)
{
	return new Device(GetRawByIndex(index) -> name);
}

const char* Device::GetNameByIndex(int index)
{
	return GetRawByIndex(index) -> description;
}

void Device::Listen(Listener lis)
{
	listener = lis;
	pcap_loop(dev, 0, ip_listener, (u_char*)this);
}

void Device::ip_listener(u_char* device, const pcap_pkthdr* header, const u_char* data)
{
	Packet* packet = new Packet((void*)data, header -> len);
	((Device*)device) -> listener(packet);
	delete packet;
}

void Device::SetFilter(const char* filter)
{
	u_long netmask;
	netmask = 0xFFFFFF;

	bpf_program fc;
	if (pcap_compile(dev, &fc, filter, 1, netmask) < 0)
		throw "Set Filter Failed!";
	if (pcap_setfilter(dev, &fc) < 0)
		throw "Set Filter Failed!";
}

void Device::Send(Packet* packet)
{
	pcap_sendpacket(dev, (const u_char*)packet -> Raw(), packet -> Length());
}

void Device::Unbind()
{
	pcap_breakloop(dev);
}
