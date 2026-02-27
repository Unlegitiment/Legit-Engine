#include <stdio.h>
#include <stdlib.h>
#include <LITemplates/alloc/Default.h>
/*
	This part below is what I hate about WS2. Weird headers.
*/
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <string>
#pragma comment (lib, "WS2_32")
namespace legit {
	class litFormat {
	public:
		template<typename...T>
		litFormat(const char* fmt, T&&... args) {
			snprintf(m_Buff, sizeof(m_Buff), fmt, args...);
			m_StringSize = strnlen_s(m_Buff, 1028);
		}
		unsigned long long GetStringSize() const {
			return this->m_StringSize;
		}
		const char* GetBuffer() const {
			return this->m_Buff;
		}
		operator const char* () {
			return this->m_Buff;
		}
	private:
		unsigned long long m_StringSize = 0ll;
		char m_Buff[1028]; // could be a templated object btw!
	};
}
namespace legit {
	/*
		Purpose: Basic TCP Connection via WSA. A Simple Demo to go along with a standard option.
	*/
	class wsaLock {
	public:
		wsaLock(WORD Version, LPWSADATA data) {
			ErrorCode = WSAStartup(Version, data);
		}
		int GetError() const {
			return this->ErrorCode;
		}
		~wsaLock() {
			printf(__FUNCTION__"\n");
			WSACleanup();
		}
	private:
		int ErrorCode = 0;
	};
	class netAddrInfo {
	public:
		using AddressT = addrinfo;
		netAddrInfo(const char* IP, const char* Port, const AddressT* pHints) {
			m_Error = GetAddrInfoA(IP, Port, pHints, &m_pResult);
		}
		netAddrInfo(const netAddrInfo&) = delete;
		netAddrInfo& operator=(const netAddrInfo&) = delete;
		netAddrInfo(netAddrInfo&& other) noexcept : m_pResult(other.m_pResult) {
			other.m_pResult = nullptr;
		}
		netAddrInfo& operator=(netAddrInfo&& other) noexcept {
			this->m_pResult = (other.m_pResult);
			other.m_pResult = nullptr;
			return *this;
		}
		int GetError() const {
			return m_Error;
		}
		bool Succeeded() const {
			return m_Error == 0;
		}
		AddressT* GetResult() const {
			return m_pResult;
		}
		~netAddrInfo() {
			printf(__FUNCTION__"\n");
			if (m_pResult)
				FreeAddrInfoA(m_pResult);
		}
	private:
		int m_Error = 0;
		AddressT* m_pResult;
	};
	class netSocket {
	public:
		using SocketT = SOCKET;
		netSocket(const netAddrInfo& addr) {
			m_NameLen = addr.GetResult()->ai_addrlen;
			memcpy(&m_pName, addr.GetResult()->ai_addr, m_NameLen);
			m_Socket = socket(addr.GetResult()->ai_family, addr.GetResult()->ai_socktype, addr.GetResult()->ai_protocol);
		}
		netSocket(const netSocket&) = delete;
		netSocket& operator=(const netSocket&) = delete;
		SocketT GetSocket() const {
			return this->m_Socket;
		}
		operator SocketT() {
			return this->m_Socket;
		}
		int Connect() const {
			return connect(this->m_Socket, reinterpret_cast<const sockaddr*>(&this->m_pName), this->m_NameLen);
		}
		int Send(const void* Data, unsigned long long Size, int flags = 0) const {
			return send(this->m_Socket, (const char*)Data, Size, flags);
		}
		~netSocket() {
			printf(__FUNCTION__"\n");
			closesocket(m_Socket);
		}
	private:
		sockaddr_storage m_pName; // Resolved. ( just didn't know about the storage type!
		int m_NameLen;
		SocketT m_Socket;
	};
	static constexpr const char* LOCALHOST = "127.0.0.1";
	class netLoggerClient {
	public:
		netLoggerClient(const char* IP, const char* Port): m_pSocket(netAddrInfo(IP, Port, &m_Hints)){
			if (m_pSocket == INVALID_SOCKET) {
				return;
			}
			iResult = m_pSocket.Connect();
			while (iResult < 0) {
				iResult = m_pSocket.Connect();
			}
			if (m_pSocket == INVALID_SOCKET) {
				return;
			}
		}
		void Send(const litFormat& fmt) {
			int total = 0;
			int size = fmt.GetStringSize();

			while (total < size) {
				int sent = m_pSocket.Send(fmt.GetBuffer() + total, size - total);
				if (sent == SOCKET_ERROR)
					return;
				total += sent;
			}
		}
	private:
		static addrinfo TCPHints() {
			addrinfo hints;
			ZeroMemory(&hints, sizeof(hints));
			hints.ai_family = AF_INET; // ipv4 or ipv6. Both function in a similar manner however. 
			hints.ai_socktype = SOCK_STREAM; // TCP Or UDP.
			hints.ai_protocol = IPPROTO_TCP; // variant. TCP Or UDP. Dependant on both AF_INET and Sock_Stream || Sock_UGRAM.
			return hints;
		}
		addrinfo m_Hints = TCPHints();
		int iResult = 0l;
		netSocket m_pSocket;
	};
	class netLogger {
	public:
		static void Init(wsaLock* pLock) { 
			// this might not be relevent here. wsa is apart of WINDOW's initialization procedure. 
			// thus its kinda useless here. the only thing that WOULD make it worth it is being able to check against potentially a global for is WSA is initted (specifically inside of the netSocket or netAddrInfo)
			// but even then, it will just hard fail anyways soooooo.
			m_Lock = pLock;
			m_Client = new netLoggerClient(LOCALHOST, "27015");
		}
		static void Update() {
			litFormat Str{"Hello! %f\n", 0.01f};
			Send(Str); // Test!
		}
		static void Send(const litFormat& String) {
			m_Client->Send(String);
		}
		static void Shutdown() {
			delete m_Client;
		}
	private:
		static inline netLoggerClient* m_Client;
		static inline wsaLock* m_Lock = 0;
	};
}
using namespace legit;
int main() {
	WSADATA Data{};
	wsaLock WindowsSecurity{MAKEWORD(2,2), &Data};
	netLogger::Init(&WindowsSecurity);
	netLogger::Update();
	netLogger::Shutdown();
}