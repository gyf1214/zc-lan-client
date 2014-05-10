#include <cstdlib>
#include <cstdio>
#include <windows.h>
#include "device.h"
#include "packet.h"
#include "tcp.h"

HANDLE hThread, hServer;
Device* device;
TCPSocket* tsocket;

void FormatAddressIP(char* str, const u_char add[4], u_short port)
{
	sprintf(str, "%d.%d.%d.%d:%d", add[0], add[1], add[2], add[3], port);
}

void FormatAddressMAC(char* str, const u_char add[6])
{
	sprintf(str, "%02X:%02X:%02X:%02X:%02X:%02X", 
		add[0], add[1], add[2], add[3], add[4], add[5]);
}

void PrintPacketInfoUDP(UDPPacket* packet)
{
	Ethernet* eth_header = packet -> EthernetHeader();
	IP* ip_header = packet -> IPHeader();
	UDP* udp_header = packet -> UDPHeader();

	char saddr[25], daddr[25];
	FormatAddressIP(saddr, ip_header -> src_addr, ntohs(udp_header -> src_port));
	FormatAddressIP(daddr, ip_header -> dest_addr, ntohs(udp_header -> dest_port));

	printf("Len: %d\n%s -> %s\n", packet -> DataLength(), saddr, daddr);

	FormatAddressMAC(saddr, eth_header -> src_addr);
	FormatAddressMAC(daddr, eth_header -> dest_addr);
	printf("%s -> %s\n", saddr, daddr);
}

void PacketHandler(Packet* pac)
{
	UDPPacket* packet = (UDPPacket*) pac;

	printf("Packet Received\n");
	PrintPacketInfoUDP(packet);
	printf("\n");

	size_t length = packet -> DataLength() + 10;
	char* buffer = new char[length];
	memcpy(buffer, "zc", 2 * sizeof(u_char));
	memcpy(buffer + 2, packet -> IPHeader() -> src_addr, 4 * sizeof(u_char));
	memcpy(buffer + 6, packet -> IPHeader() -> dest_addr, 4 * sizeof(u_char));
	memcpy(buffer + 10, packet -> Data(), packet -> DataLength());
	tsocket -> Send(buffer, length);

	FILE *out = fopen("test.bin", "wb");
	fwrite(packet -> Data(), sizeof(u_char), packet -> DataLength(), out);
	fclose(out);
}

u_long WINAPI Listen(void*)
{
	printf("Listening on UDP port 6112\n");
	device -> SetFilter("ip and udp and port 6112");
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

void SendPacket()
{
	u_char* data;
	size_t len = ReadPacket("3.bin", &data);

	printf("Send a packet\n");
	UDPPacket* packet = new UDPPacket(len + 42);

	u_char buf[20] = {
		0x22, 0x22, 0x22, 0x22, 0x22, 0x22,
		0x33, 0x33, 0x33, 0x33, 0x33, 0x33,
		192, 168, 2, 3,
		127, 0, 0, 1
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
	while (tsocket && !tsocket -> Closed())
	{
		if (tsocket -> Recv() != -1)
		{
			//TODO
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
	tsocket -> Connect("192.168.0.103", 8888);

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

int main()
{
	try
	{
		init();
		InputLoop();
		clean();
	} catch (const char* str) {
		printf("%s\n", str);
		return -1;
	}


	return 0;
}
