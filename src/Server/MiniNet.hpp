//
// A drop-in stand-in for the handful of SFML types the server uses.
//
// The server needs SFML for exactly twelve references: a non-blocking UDP
// socket, an IP address, a stopwatch, and a status enum. Requiring a 30 MB
// dependency for that is a poor trade when the point is for people to
// self-host a relay -- every extra build step is someone who does not run one.
// So this provides the same names over plain winsock/BSD sockets, and
// Server.hpp picks it when SFML is absent.
//
// It is deliberately NOT a general SFML reimplementation. It covers what
// Server.cpp actually calls and nothing else, so that anything it does not
// support fails at compile time rather than quietly behaving differently.
//

#ifndef INC_4PSOKU_MININET_HPP
#define INC_4PSOKU_MININET_HPP

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <cerrno>
#include <chrono>
#include <string>

#ifdef _WIN32
// windef.h, reached through winsock2.h, defines min/max as macros and breaks
// every std::min in the server. SFML's headers took care of this; the shim has
// to as well.
# ifndef NOMINMAX
#  define NOMINMAX
# endif
# ifndef WIN32_LEAN_AND_MEAN
#  define WIN32_LEAN_AND_MEAN
# endif
# include <winsock2.h>
# include <ws2tcpip.h>
# pragma comment(lib, "ws2_32.lib")
using mininet_socket_t = SOCKET;
# define MININET_INVALID INVALID_SOCKET
#else
# include <arpa/inet.h>
# include <fcntl.h>
# include <netinet/in.h>
# include <sys/socket.h>
# include <unistd.h>
using mininet_socket_t = int;
# define MININET_INVALID (-1)
#endif

namespace sf {

class Time {
public:
	explicit Time(std::chrono::steady_clock::duration d = {}) : _d(d) {}

	float asSeconds() const
	{
		return std::chrono::duration<float>(_d).count();
	}

	int32_t asMilliseconds() const
	{
		return (int32_t)std::chrono::duration_cast<std::chrono::milliseconds>(_d).count();
	}

private:
	std::chrono::steady_clock::duration _d;
};

class Clock {
public:
	Clock() : _start(std::chrono::steady_clock::now()) {}

	Time getElapsedTime() const
	{
		return Time(std::chrono::steady_clock::now() - _start);
	}

	Time restart()
	{
		auto now = std::chrono::steady_clock::now();
		Time elapsed(now - _start);
		_start = now;
		return elapsed;
	}

private:
	std::chrono::steady_clock::time_point _start;
};

class IpAddress {
public:
	// Host byte order throughout, so ordering is stable and readable. Only
	// converted at the socket boundary.
	IpAddress() : _addr(0) {}
	explicit IpAddress(uint32_t hostOrder) : _addr(hostOrder) {}

	static const IpAddress Any;

	static IpAddress fromNetworkOrder(uint32_t netOrder)
	{
		return IpAddress(ntohl(netOrder));
	}

	uint32_t toNetworkOrder() const
	{
		return htonl(_addr);
	}

	// Host order, matching sf::IpAddress::toInteger, so callers work against
	// either backend.
	uint32_t toInteger() const
	{
		return _addr;
	}

	std::string toString() const
	{
		char buf[16];
		snprintf(buf, sizeof(buf), "%u.%u.%u.%u",
			(_addr >> 24) & 0xFF, (_addr >> 16) & 0xFF,
			(_addr >> 8) & 0xFF, _addr & 0xFF);
		return buf;
	}

	// Needed because clients are keyed by (address, port) in a std::map.
	bool operator<(const IpAddress &o) const { return _addr < o._addr; }
	bool operator==(const IpAddress &o) const { return _addr == o._addr; }
	bool operator!=(const IpAddress &o) const { return _addr != o._addr; }

private:
	uint32_t _addr;
};

inline const IpAddress IpAddress::Any{0};

class Socket {
public:
	enum Status {
		Done,
		NotReady,
		Partial,
		Disconnected,
		Error
	};
};

class UdpSocket : public Socket {
public:
	UdpSocket()
	{
#ifdef _WIN32
		// Safe to call repeatedly; refcounted, and the server never unloads.
		WSADATA wsa;
		WSAStartup(MAKEWORD(2, 2), &wsa);
#endif
		_sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	}

	~UdpSocket()
	{
		if (_sock != MININET_INVALID) {
#ifdef _WIN32
			closesocket(_sock);
#else
			close(_sock);
#endif
		}
	}

	UdpSocket(const UdpSocket &) = delete;
	UdpSocket &operator=(const UdpSocket &) = delete;

	void setBlocking(bool blocking)
	{
#ifdef _WIN32
		u_long mode = blocking ? 0 : 1;
		ioctlsocket(_sock, FIONBIO, &mode);
#else
		int flags = fcntl(_sock, F_GETFL, 0);
		fcntl(_sock, F_SETFL, blocking ? (flags & ~O_NONBLOCK) : (flags | O_NONBLOCK));
#endif
	}

	Status bind(unsigned short port, const IpAddress &address = IpAddress::Any)
	{
		if (_sock == MININET_INVALID)
			return Error;

		sockaddr_in addr{};
		addr.sin_family = AF_INET;
		addr.sin_port = htons(port);
		addr.sin_addr.s_addr = address.toNetworkOrder();
		if (::bind(_sock, (sockaddr *)&addr, sizeof(addr)) != 0)
			return Error;
#ifdef _WIN32
		// Windows reports an ICMP "port unreachable" for an earlier send as a
		// failure of the NEXT recvfrom on the same UDP socket. For a relay that
		// is routine -- a player closes their game and the next packet to them
		// bounces -- and it used to kill the whole relay with "Unknown socket
		// error" for everyone else (2026-09-26). giuroll turns this off on the
		// game's socket for the same reason. SIO_UDP_CONNRESET, from mstcpip.h.
		{
			const DWORD SIO_UDP_CONNRESET_ = 0x9800000C;
			BOOL report = FALSE;
			DWORD unused = 0;

			WSAIoctl(_sock, SIO_UDP_CONNRESET_, &report, sizeof(report), nullptr, 0, &unused, nullptr, nullptr);
		}
#endif
		return Done;
	}

	Status receive(void *data, std::size_t size, std::size_t &received,
		IpAddress &remoteAddress, unsigned short &remotePort)
	{
		received = 0;

		sockaddr_in from{};
#ifdef _WIN32
		int fromLen = sizeof(from);
#else
		socklen_t fromLen = sizeof(from);
#endif
		int r = recvfrom(_sock, (char *)data, (int)size, 0, (sockaddr *)&from, &fromLen);
		if (r < 0) {
#ifdef _WIN32
			// No datagram waiting is the normal case on a non-blocking socket,
			// not a failure -- the server polls this in a loop.
			switch (WSAGetLastError()) {
			case WSAEWOULDBLOCK:
				return NotReady;
			// Per-datagram failures: a bounced earlier send, or a datagram
			// larger than the buffer. The socket is fine and the next datagram
			// is readable, so report "skip this one", not a dead socket.
			case WSAECONNRESET:
			case WSAEMSGSIZE:
				return Partial;
			default:
				return Error;
			}
#else
			return (errno == EAGAIN || errno == EWOULDBLOCK) ? NotReady : Error;
#endif
		}

		received = (std::size_t)r;
		remoteAddress = IpAddress::fromNetworkOrder(from.sin_addr.s_addr);
		remotePort = ntohs(from.sin_port);
		return Done;
	}

	Status send(const void *data, std::size_t size, const IpAddress &remoteAddress,
		unsigned short remotePort)
	{
		sockaddr_in to{};
		to.sin_family = AF_INET;
		to.sin_port = htons(remotePort);
		to.sin_addr.s_addr = remoteAddress.toNetworkOrder();

		int r = sendto(_sock, (const char *)data, (int)size, 0, (sockaddr *)&to, sizeof(to));
		return r == (int)size ? Done : Error;
	}

private:
	mininet_socket_t _sock;
};

}

#endif //INC_4PSOKU_MININET_HPP
