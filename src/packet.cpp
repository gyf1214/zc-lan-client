#define _LIB
#include "packet.h"

Packet::Packet(void *s, size_t l)
{
	data = (u_char*)s;
	len = l;
	release = false;
}

Packet::Packet(size_t l)
{
	data = new u_char[l];
	len = l;
	release = true;
}

Packet::~Packet()
{
	if (release)
		delete[] data;
}

void* Packet::Raw()
{
	return data;
}

Ethernet* Packet::EthernetHeader()
{
	return (Ethernet*) data;
}

IP* Packet::IPHeader()
{
	return (IP*) (data + EthernetLength);
}

void* Packet::Data()
{
	return data + HeaderLength();
}

size_t Packet::Length()
{
	return len;
}

size_t Packet::IPLength()
{
	return ((IPHeader() -> ver_ihl) & 0xf) * 4;
}

size_t Packet::HeaderLength()
{
	return EthernetLength + IPLength();
}

size_t Packet::DataLength()
{
	return len - HeaderLength();
}

u_short Packet::CalcIPChecksum()
{
	u_short ret = 0;
	for (int i = 14; i < 34; i += 2)
	{
		u_short tmp = BTW(data[i], data[i + 1]);
		u_short diff = 65535 - ret;
		ret += tmp;
		if (tmp > diff) ++ret;
	}
	return ~ret;
}

UDPPacket::UDPPacket(void* s, size_t l) : Packet(s, l)
{}

UDPPacket::UDPPacket(size_t l) : Packet(l)
{}

UDP* UDPPacket::UDPHeader()
{
	return (UDP*) (data + EthernetLength + IPLength());
}

size_t UDPPacket::HeaderLength()
{
	return EthernetLength + IPLength() + UDPLength;
}

size_t UDPPacket::DataLength()
{
	return len - HeaderLength();
}

void* UDPPacket::Data()
{
	return data + HeaderLength();
}

void UDPPacket::CreatePacket(
	u_char*	src_mac,
	u_char*	dest_mac,
	u_char*	src_ip,
	u_char*	dest_ip,
	u_short src_port,
	u_short dest_port,
	void*	user_data
) {
	Ethernet* eth_header = EthernetHeader();
	memcpy(eth_header -> dest_addr, dest_mac, 6);
	memcpy(eth_header -> src_addr, src_mac, 6);
	eth_header -> frame_type = 0x8;

	IP* ip_header = IPHeader();
	ip_header -> ver_ihl = 0x45;
	ip_header -> tos = 0;
	ip_header -> tlen = htons(len - EthernetLength);
	ip_header -> identifier = ID;
	ip_header -> flags_fo = 0;
	ip_header -> ttl = 0x80;
	ip_header -> protocol = 0x11;
	ip_header -> crc = 0;
	memcpy(ip_header -> src_addr, src_ip, 4);
	memcpy(ip_header -> dest_addr, dest_ip, 4);

	UDP* udp_header = UDPHeader();
	udp_header -> src_port = htons(src_port);
	udp_header -> dest_port = htons(dest_port);
	udp_header -> len = htons(len - EthernetLength - IPLength());
	memcpy(data + HeaderLength(), user_data, DataLength());

	ip_header -> crc = htons(CalcIPChecksum());
	udp_header -> crc = CalcUDPChecksum();
}

u_short UDPPacket::CalcUDPChecksum()
{
	u_short CheckSum = 0;
	u_short PseudoLength = DataLength() + 8 + 9;

	PseudoLength += PseudoLength % 2;
	u_short Length = DataLength() + 8;

	u_char* PseudoHeader = new u_char [PseudoLength]; 
	for(int i = 0;i < PseudoLength;i++)
		PseudoHeader[i] = 0x00;

	PseudoHeader[0] = 0x11;

	memcpy((void*)(PseudoHeader+1),(void*)(data+26),8);

	Length = htons(Length);
	memcpy((void*)(PseudoHeader+9),(void*)&Length,2);
	memcpy((void*)(PseudoHeader+11),(void*)&Length,2); 

	memcpy((void*)(PseudoHeader+13),(void*)(data+34),2);
	memcpy((void*)(PseudoHeader+15),(void*)(data+36),2);

	memcpy((void*)(PseudoHeader+17),(void*)Data(),DataLength());


	for(int i = 0;i < PseudoLength;i+=2)
    {
        u_short Tmp = BTW(PseudoHeader[i],PseudoHeader[i+1]);
        u_short Difference = 65535 - CheckSum;
        CheckSum += Tmp;
        if(Tmp > Difference){CheckSum += 1;}
    }
    delete[] PseudoHeader;
	CheckSum = ~CheckSum;
	return CheckSum;
}
