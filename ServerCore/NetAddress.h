#pragma once


class NetAddress
{
public:
	NetAddress() = default;
	NetAddress(SOCKADDR_IN sockAddr); // SOCKADDR_IN ����ü�� �״�� �޾ƿͼ� �ʱ�ȭ�ϴ� ���.
	NetAddress(wstring ip, uint16 port); // ip�� port�� �޾ƿͼ� �����ϴ� ���.

	SOCKADDR_IN& GetSockAddr() { return _sockAddr; }
	wstring GetIpAddress();
	uint16 GetPort() { return ::ntohs(_sockAddr.sin_port); }

public:
	static IN_ADDR Ip2Address(const WCHAR* ip);

private:
	SOCKADDR_IN _sockAddr = {};

};

