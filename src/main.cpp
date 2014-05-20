#include <cstdlib>
#include <cstdio>
#include <windows.h>
#include "device.h"
#include "packet.h"
#include "tcp.h"

HANDLE hThread, hServer;
Device* device;
TCPSocket* tsocket;
const char* server;
u_char mac[6];
u_long local;

void FormatAddressIP(char* str, const u_char add[4])
{
	sprintf(str, "%d.%d.%d.%d", add[0], add[1], add[2], add[3]);
}

void FormatAddressMAC(char* str, const u_char add[6])
{
	sprintf(str, "%02X:%02X:%02X:%02X:%02X:%02X", 
		add[0], add[1], add[2], add[3], add[4], add[5]);
}

void PrintPacketInfoIP(Packet* packet)
{
	Ethernet* eth_header = packet -> EthernetHeader();
	IP* ip_header = packet -> IPHeader();

	char saddr[25], daddr[25];
	FormatAddressIP(saddr, (u_char*)&(ip_header -> src));
	FormatAddressIP(daddr, (u_char*)&(ip_header -> dest));

	printf("Len: %d\n%s -> %s\n", packet -> DataLength(), saddr, daddr);

	FormatAddressMAC(saddr, eth_header -> src);
	FormatAddressMAC(daddr, eth_header -> dest);
	printf("%s -> %s\n", saddr, daddr);
}

void UDPHandler(UDPPacket* packet)
{
	UDPPacket* another = (UDPPacket*)packet -> Clone();
	another -> CalcIPChecksum();
	printf("IP crc: %04X -> %04X\n", another -> IPHeader() -> crc, packet -> IPHeader() -> crc);
	another -> CalcUDPChecksum();
	printf("UDP crc: %04X -> %04X\n", another -> UDPHeader() -> crc, packet -> UDPHeader() -> crc);
}

void TCPHandler(TCPPacket* packet)
{
	printf("%d -> %d\n", ntohs(packet -> TCPHeader() -> src_port), ntohs(packet -> TCPHeader() -> dest_port));

	TCPPacket* another = (TCPPacket*)packet -> Clone();
	another -> CalcIPChecksum();
	printf("IP crc: %04X -> %04X\n", another -> IPHeader() -> crc, packet -> IPHeader() -> crc);
	another -> CalcTCPChecksum();
	printf("TCP crc: %04X -> %04X\n", another -> TCPHeader() -> crc, packet -> TCPHeader() -> crc);
}

void PacketHandler(Packet* packet)
{
	printf("Packet Received\n");
	PrintPacketInfoIP(packet);

	if (packet -> IPHeader() -> src == local)
	{
		size_t length = packet -> Length() + 10;
		char* buffer = new char[length];
		memcpy(buffer, "zc", 2 * sizeof(u_char));
		memcpy(buffer + 2, &packet -> IPHeader() -> src, 4 * sizeof(u_char));
		memcpy(buffer + 6, &packet -> IPHeader() -> dest, 4 * sizeof(u_char));
		memcpy(buffer + 10, packet -> Raw(), packet -> Length());
		tsocket -> Send(buffer, length);
	}

	if (packet -> IPHeader() -> protocol == PROTO_TCP)
	{
		printf("TCP\n");
		TCPHandler((TCPPacket*)packet);
	} else if (packet -> IPHeader() -> protocol == PROTO_UDP)
	{
		printf("UDP\n");
		UDPHandler((UDPPacket*)packet);
	} else {
		printf("Unrecognized\n");
	}
/*
	FILE *out = fopen("test.bin", "wb");
	fwrite(packet -> Raw(), sizeof(u_char), packet -> Length(), out);
	fclose(out);
*/

	printf("\n");
}

u_long WINAPI Listen(void*)
{
	printf("Listening on udp port 6112\n");
	device -> SetFilter("ip and (net 35.143 or net 255.255.255.255) and (tcp or udp)");
	device -> Listen(PacketHandler);
	printf("Stop listening\n");
	return 0;
}

size_t ReadPacket(const char* path, u_char** ret)
{
	FILE* in = fopen(path, "rb");
	fseek(in, 0, SEEK_END);
	size_t len = ftell(in);
	fseek(in, 0, SEEK_SET);

	*ret = new u_char[len];
	fread(*ret, sizeof(u_char), len, in);
	fclose(in);

	return len;
}

void SendPacket()
{
	u_char* data;
	size_t len = ReadPacket("5.bin", &data);

	printf("Send a packet\n");
	UDPPacket* packet = new UDPPacket(len);
	memcpy(packet -> Raw(), data, len);

	u_char temp[6];
	memcpy(temp, packet -> EthernetHeader() -> src, 6);
	memcpy(packet -> EthernetHeader() -> src, packet -> EthernetHeader() -> dest, 6);
	memcpy(packet -> EthernetHeader() -> dest, temp, 6);
	
	u_long t;
	t = packet -> IPHeader() -> src;
	packet -> IPHeader() -> dest = packet -> IPHeader() -> src;
	packet -> IPHeader() -> src = inet_addr("192.168.56.111");
	
	packet -> CalcIPChecksum();
	packet -> CalcUDPChecksum();

	device -> Send(packet);
	delete packet;
	delete[] data;
}

void InputLoop()
{
	for (;;)
	{
		char a = getchar();
		switch (a)
		{
			case 'q':
				return;
				break;
			case 's':
				SendPacket();
				break;
		}
	}
}

u_long WINAPI ListenServer(void*)
{
	tsocket -> Connect(server, 8888);
	while (tsocket && !tsocket -> Closed())
	{
		int size;
		if ((size = tsocket -> Recv()) != -1)
		{
			const u_char* buffer = (const u_char*)tsocket -> Data();

			char saddr[25], daddr[25];
			FormatAddressIP(saddr, buffer + 2);
			FormatAddressIP(daddr, buffer + 6);
			printf("Receive from server: Len: %d\n", size - 10);
			printf("%s -> %s\n", saddr, daddr);

			const u_char* data = buffer + 10;

			Packet* packet = new Packet(size - 10);
			memcpy(packet -> Raw(), data, size - 10);

			memcpy(packet -> EthernetHeader() -> dest, mac, 6);
			packet -> IPHeader() -> src = *(u_long*)(buffer + 2);
			packet -> IPHeader() -> dest = local;
			packet -> CalcIPChecksum();

			bool flag = true;
			if (packet -> IPHeader() -> protocol == PROTO_TCP)
			{
				((TCPPacket*)packet) -> CalcTCPChecksum();
			} else if (packet -> IPHeader() -> protocol == PROTO_UDP)
			{
				((UDPPacket*)packet) -> CalcUDPChecksum();
			} else {
				printf("Bad server packet\n");
				flag = false;
			}

			if (flag)
				device -> Send(packet);

			delete packet;

			printf("\n");
		}
	}
	printf("Server disconnected\n");
	return 0;
}

void init()
{
	Device::ListDevices();
	for (int i = 0; i < Device::Count(); ++i)
		printf("%d: %s\n", i, Device::GetNameByIndex(i));
	printf("Select Device No:");
	int k;
	scanf("%d", &k);
	device = Device::GetDeviceByIndex(k);
	Device::FreeDevices();

	hThread = CreateThread(0, 0, Listen, NULL, 0, NULL);

	TCPSocket::Startup();
	tsocket = new TCPSocket();

	hServer = CreateThread(0, 0, ListenServer, NULL, 0, NULL);
}

void clean()
{
	device -> Unbind();
	delete device;

	CloseHandle(hThread);

	tsocket -> Close();
	delete tsocket;
	TCPSocket::Clean();

	CloseHandle(hServer);
}

int main(int argc, char** argv)
{
	try
	{
		server = argv[1];

		int tp[6];
		sscanf(argv[2], "%02X:%02X:%02X:%02X:%02X:%02X", 
			&tp[0], &tp[1], &tp[2], &tp[3], &tp[4], &tp[5]);
		for (int i = 0; i < 6; ++i)
			mac[i] = tp[i];

		local = inet_addr(argv[3]);

		init();
		InputLoop();
		clean();
	} catch (const char* str) {
		printf("%s\n", str);
		return -1;
	}


	return 0;
}
