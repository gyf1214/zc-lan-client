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

void PacketHandler(Packet* packet)
{
	printf("Packet Received\n");
	PrintPacketInfoIP(packet);
	printf("\n");

	size_t length = packet -> Length() + 10;
	char* buffer = new char[length];
	memcpy(buffer, "zc", 2 * sizeof(u_char));
	memcpy(buffer + 2, &packet -> IPHeader() -> src, 4 * sizeof(u_char));
	memcpy(buffer + 6, &packet -> IPHeader() -> dest, 4 * sizeof(u_char));
	memcpy(buffer + 10, packet -> Raw(), packet -> Length());
	tsocket -> Send(buffer, length);

	Packet* another = new Packet(packet -> Length());
	memcpy(another -> Raw(), packet -> Raw(), packet -> Length());
	another -> CalcIPChecksum();
	printf("%04X -> %04X\n", another -> IPHeader() -> crc, packet -> IPHeader() -> crc);

	FILE *out = fopen("test.bin", "wb");
	fwrite(packet -> Raw(), sizeof(u_char), packet -> Length(), out);
	fclose(out);
}

u_long WINAPI Listen(void*)
{
	printf("Listening on udp port 6112\n");
	device -> SetFilter("ip and port 6112");
	device -> ListenUDP(PacketHandler);
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
/*
void SendPacket()
{
	u_char* data;
	size_t len = ReadPacket("4.bin", &data);

	printf("Send a packet\n");
	UDPPacket* packet = new UDPPacket(len + 42);

	u_char buf[20] = {
		0x22, 0x22, 0x22, 0x22, 0x22, 0x22,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		192, 168, 2, 3,
		255, 255, 255, 255
	};

	u_short p1 = 6112, p2 = 6112;

	packet -> CreatePacket(
		buf,
		buf + 6,
		buf + 12,
		buf + 16,
		p1, p2,
		data
	);

	device -> Send(packet);
	delete packet;
	delete data;
}
*/
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
				//SendPacket();
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
			//TODO

			/*
			const u_char* buffer = (const u_char*)tsocket -> Data();
			char saddr[25], daddr[25];
			FormatAddressIP(saddr, buffer + 2, 6112);
			FormatAddressIP(daddr, buffer + 6, 6112);
			printf("Receive from server:\n");
			printf("%s -> %s\n", saddr, daddr);
			const u_char* data = buffer + 10;
			printf("%s\n", data);

			UDPPacket* packet = new UDPPacket(size - 10 + 42);
			u_char temp[10] = {
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				127, 0, 0, 1
			};
			packet -> CreatePacket(
				temp, mac, buffer + 2, temp + 6, 6112, 6112, data);
			device -> Send(packet);

			delete packet;
			*/
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
		init();
		InputLoop();
		clean();
	} catch (const char* str) {
		printf("%s\n", str);
		return -1;
	}


	return 0;
}
