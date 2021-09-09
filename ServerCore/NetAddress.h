#pragma once


class NetAddress
{
public:
	NetAddress() = default;
	NetAddress(SOCKADDR_IN sockAddr); // SOCKADDR_IN 구조체를 그대로 받아와서 초기화하는 방식.
	NetAddress(wstring ip, uint16 port); // ip와 port를 받아와서 저장하는 방식.

	SOCKADDR_IN& GetSockAddr() { return _sockAddr; }
	wstring GetIpAddress();
	uint16 GetPort() { return ::ntohs(_sockAddr.sin_port); }

public:
	static IN_ADDR Ip2Address(const WCHAR* ip);

private:
	SOCKADDR_IN _sockAddr = {};

};

