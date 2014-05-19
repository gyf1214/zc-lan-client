#ifndef __PROTOCOL
#define __PROTOCOL
#include <pcap.h>
#include <winsock.h>

#define BTW(x, y) (((x)<<8)|(y))
#define BTDW(x, y, z, w) (((x)<<24)|((y)<<16)|((z)<<8)|(w))

struct Ethernet
{
	u_char	dest[6];
	u_char	src[6];
	u_short	frame_type;
};

struct IP
{
	u_char		ver_ihl;
	u_char		tos;
	u_short		tlen;
	u_short		identifier;
	u_short		flags_fo;
	u_char		ttl;
	u_char		protocol;
	u_short		crc;
	u_long		src;
	u_long		dest;
	u_long		op_pad;
};

struct UDP
{
	u_short	src_port;
	u_short	dest_port;
	u_short	len;
	u_short	crc;
};

struct TCP
{
	u_short	src_port;
	u_short	dest_port;
	u_long	seq;
	u_long	ack;
	u_char	res_fo;
	u_char	flags;
	u_short	window;
	u_short	crc;
	u_short	urg;
	u_long	op_pad;
};

struct PSD
{
	u_long	src;
	u_long	dest;
    u_char	zero;
    u_char	protocol;
    u_short	len;
};

#endif
