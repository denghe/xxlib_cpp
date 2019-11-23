#include <iostream>
#include <atomic>
#include <thread>
#include <unordered_map>
#include <vector>
#include <chrono>

// 得到当前 system 时间点的 epoch (精度为 ms)
inline int64_t NowSystemEpochMS() noexcept {
	return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}

int main(int argc, char* argv[]) {
	auto begin = NowSystemEpochMS();
	std::atomic<std::int64_t> n(0);
	std::vector<std::thread> ts;
	for (int j = 0; j < 2; j++) {
		ts.emplace_back([&, j = j] {
			for (std::size_t i = 0; i < 100000000; i++) {
				++n;
				if (i % 10000000 == 0) {
					std::cout << j << " " << n << std::endl;
				}
			}
		});
	}
	for (auto&& t : ts) {
		t.join();
	}
	auto end = NowSystemEpochMS() - begin;
	std::cout << "ms = " << end << ", n = " << n << std::endl;
	return 0;
}









//#define WIN32_LEAN_AND_MEAN
//
//#include <windows.h>
//#include <winsock2.h>
//#include <ws2tcpip.h>
//#include <stdlib.h>
//#include <stdio.h>
//
//
//// Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib
//#pragma comment (lib, "Ws2_32.lib")
//#pragma comment (lib, "Mswsock.lib")
//#pragma comment (lib, "AdvApi32.lib")
//
//
//
//
//int Run(char const* const& ip, char const* const& port)
//{
//	WSADATA wsaData;
//	SOCKET ConnectSocket = INVALID_SOCKET;
//	struct addrinfo* result = NULL,
//		* ptr = NULL,
//		hints;
//	char* sendbuf = "this is a test";
//	char recvbuf[512];
//	int iResult;
//	int recvbuflen = 512;
//
//	// Initialize Winsock
//	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
//	if (iResult != 0) {
//		printf("WSAStartup failed with error: %d\n", iResult);
//		return 1;
//	}
//
//	ZeroMemory(&hints, sizeof(hints));
//	hints.ai_family = AF_UNSPEC;
//	hints.ai_socktype = SOCK_STREAM;
//	hints.ai_protocol = IPPROTO_TCP;
//
//	// Resolve the server address and port
//	iResult = getaddrinfo(ip, port, &hints, &result);
//	if (iResult != 0) {
//		printf("getaddrinfo failed with error: %d\n", iResult);
//		WSACleanup();
//		return 1;
//	}
//
//	// Attempt to connect to an address until one succeeds
//	for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {
//
//		// Create a SOCKET for connecting to server
//		ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype,
//			ptr->ai_protocol);
//		if (ConnectSocket == INVALID_SOCKET) {
//			printf("socket failed with error: %ld\n", WSAGetLastError());
//			WSACleanup();
//			return 1;
//		}
//
//		// Connect to server.
//		iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
//		if (iResult == SOCKET_ERROR) {
//			closesocket(ConnectSocket);
//			ConnectSocket = INVALID_SOCKET;
//			continue;
//		}
//		break;
//	}
//
//	freeaddrinfo(result);
//
//	if (ConnectSocket == INVALID_SOCKET) {
//		printf("Unable to connect to server!\n");
//		WSACleanup();
//		return 1;
//	}
//
//	//while (true) {
//
//		// Send an initial buffer
//		iResult = send(ConnectSocket, sendbuf, (int)strlen(sendbuf), 0);
//		if (iResult == SOCKET_ERROR) {
//			printf("send failed with error: %d\n", WSAGetLastError());
//			closesocket(ConnectSocket);
//			WSACleanup();
//			return 1;
//		}
//
//		printf("Bytes Sent: %ld\n", iResult);
//
//		// shutdown the connection since no more data will be sent
//		iResult = shutdown(ConnectSocket, SD_SEND);
//		if (iResult == SOCKET_ERROR) {
//			printf("shutdown failed with error: %d\n", WSAGetLastError());
//			closesocket(ConnectSocket);
//			WSACleanup();
//			return 1;
//		}
//
//		// Receive until the peer closes the connection
//		//do {
//
//			iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);
//			if (iResult > 0)
//				printf("Bytes received: %d\n", iResult);
//			else if (iResult == 0)
//				printf("Connection closed\n");
//			else
//				printf("recv failed with error: %d\n", WSAGetLastError());
//
//		//} while (iResult > 0);
//
//	//}
//
//	// cleanup
//	closesocket(ConnectSocket);
//	WSACleanup();
//
//	return 0;
//}
//
//int main()
//{
//	while(true)
//	Run("10.0.0.26", "12345");
//	return 0;
//}

//#include <iostream>
//#include <string>
//#include <winsock2.h> 
//#pragma comment(lib, "ws2_32.lib") 
//
//typedef struct
//{
//	WSAOVERLAPPED Overlapped;
//	SOCKET Socket;
//	WSABUF wsaBuf;
//	char Buffer[1024];
//	DWORD Flags;
//} PER_IO_DATA, * LPPER_IO_DATA;
//
//static DWORD WINAPI ClientWorkerThread(LPVOID lpParameter)
//{
//	HANDLE hCompletionPort = (HANDLE)lpParameter;
//	DWORD NumBytesRecv = 0;
//	unsigned __int64 CompletionKey;
//	LPPER_IO_DATA PerIoData;
//
//	while (GetQueuedCompletionStatus(hCompletionPort, &NumBytesRecv, &CompletionKey, (LPOVERLAPPED*)& PerIoData, INFINITE))
//	{
//		if (!PerIoData)
//			continue;
//
//		if (NumBytesRecv == 0)
//		{
//			std::cout << "Server disconnected!\r\n\r\n";
//		}
//		else
//		{
//			// use PerIoData->Buffer as needed...
//			std::cout << std::string(PerIoData->Buffer, NumBytesRecv);
//
//			PerIoData->wsaBuf.len = sizeof(PerIoData->Buffer);
//			PerIoData->Flags = 0;
//
//			if (WSARecv(PerIoData->Socket, &(PerIoData->wsaBuf), 1, &NumBytesRecv, &(PerIoData->Flags), &(PerIoData->Overlapped), NULL) == 0)
//				continue;
//
//			if (WSAGetLastError() == WSA_IO_PENDING)
//				continue;
//		}
//
//		closesocket(PerIoData->Socket);
//		delete PerIoData;
//	}
//
//	return 0;
//}
//
//int main(void)
//{
//	WSADATA WsaDat;
//	if (WSAStartup(MAKEWORD(2, 2), &WsaDat) != 0)
//		return 0;
//
//	HANDLE hCompletionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
//	if (!hCompletionPort)
//		return 0;
//
//	SYSTEM_INFO systemInfo;
//	GetSystemInfo(&systemInfo);
//
//	for (DWORD i = 0; i < systemInfo.dwNumberOfProcessors; ++i)
//	{
//		HANDLE hThread = CreateThread(NULL, 0, ClientWorkerThread, hCompletionPort, 0, NULL);
//		CloseHandle(hThread);
//	}
//
//
//	for (int i = 0; i < 100; ++i) {
//
//	}
//
//	SOCKET Socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
//	if (Socket == INVALID_SOCKET)
//		return 0;
//
//	SOCKADDR_IN SockAddr;
//	SockAddr.sin_family = AF_INET;
//	SockAddr.sin_addr.s_addr = inet_addr("10.0.0.26");
//	SockAddr.sin_port = htons(12345);
//
//	CreateIoCompletionPort((HANDLE)Socket, hCompletionPort, 0, 0);
//
//	if (WSAConnect(Socket, (SOCKADDR*)(&SockAddr), sizeof(SockAddr), NULL, NULL, NULL, NULL) == SOCKET_ERROR)
//		return 0;
//
//	PER_IO_DATA* pPerIoData = new PER_IO_DATA;
//	ZeroMemory(pPerIoData, sizeof(PER_IO_DATA));
//
//	pPerIoData->Socket = Socket;
//	pPerIoData->Overlapped.hEvent = WSACreateEvent();
//	pPerIoData->wsaBuf.buf = pPerIoData->Buffer;
//	pPerIoData->wsaBuf.len = 1;//sizeof(pPerIoData->Buffer);
//
//	DWORD dwNumRecv;
//	if (WSARecv(Socket, &(pPerIoData->wsaBuf), 1, &dwNumRecv, &(pPerIoData->Flags), &(pPerIoData->Overlapped), NULL) == SOCKET_ERROR)
//	{
//		if (WSAGetLastError() != WSA_IO_PENDING)
//		{
//			delete pPerIoData;
//			return 0;
//		}
//	}
//
//	DWORD dwNumSent;
//	if (WSASend(Socket, &(pPerIoData->wsaBuf), 1, &dwNumSent, 0, &(pPerIoData->Overlapped), NULL) == SOCKET_ERROR)
//	{
//		if (WSAGetLastError() != WSA_IO_PENDING)
//		{
//			delete pPerIoData;
//			return 0;
//		}
//	}
//
//
//	while (TRUE)
//		Sleep(1000);
//
//
//
//
//	shutdown(Socket, SD_BOTH);
//	closesocket(Socket);
//
//	WSACleanup();
//	return 0;
//}

//#ifdef _MSC_VER
//#pragma execution_character_set("utf-8")
//#endif
//#include "xx_mysql.h"
//#include "xx_queue.h"
//#include "xx_threadpool.h"
//#include <atomic>
//
//int main() {
//	for (int j = 0; j < 10; ++j) {
//		auto&& t = xx::NowSystemEpochMS();
//		std::atomic<int> n(0);
//		{
//			xx::ThreadPool tp;
//			for (int i = 0; i < 10000000; ++i) {
//				tp.Add([&] {
//					++n;
//				});
//			}
//		}
//		xx::CoutN("ms = ", xx::NowSystemEpochMS() - t);
//		std::cout << n << std::endl;
//	}
//	return 0;
//
//	//auto&& t = xx::NowSystemEpochMS();
//	//{
//	//	xx::Queue<size_t> ints;
//	//	for (size_t i = 0; i < 10000000; ++i) {
//	//		ints.Push(i);
//	//		if (i > 100) {
//	//			ints.Pop();
//	//		}
//	//	}
//	//	for (size_t i = 0; i < ints.Count(); ++i) {
//	//		xx::CoutN(ints[i]);
//	//	}
//	//}
//	//xx::CoutN("ms = ", xx::NowSystemEpochMS() - t);
//	//t = xx::NowSystemEpochMS();
//	//{
//	//	std::deque<size_t> ints;
//	//	for (size_t i = 0; i < 10000000; ++i) {
//	//		ints.push_back(i);
//	//		if (i > 100) {
//	//			ints.pop_front();
//	//		}
//	//	}
//	//	for (size_t i = 0; i < ints.size(); ++i) {
//	//		xx::CoutN(ints[i]);
//	//	}
//	//}
//	//xx::CoutN("ms = ", xx::NowSystemEpochMS() - t);
//	//return 0;
//
//	//xx::MySql::Connection conn;
//	//try {
//	//	conn.Open("192.168.1.230", 3306, "root", "Abc123", "test");
//	//	auto&& v1 = conn.MakeEscaped();
//	//	std::string sql;
//	//	v1 = "'\0a\0s\"\"d\"\"f\0'";
//	//	xx::Append(sql, "select '", v1, "'");
//
//	//	auto&& t = xx::NowSystemEpochMS();
//	//	std::string c1;
//	//	for (int i = 0; i < 1000; ++i) {
//	//		conn.Execute(sql);
//	//		conn.Fetch(nullptr, [&](xx::MySql::Reader& r) {
//	//			r.Reads(c1);
//	//			return true;
//	//			});
//	//	}
//	//	xx::CoutN("real_query ms = ", xx::NowSystemEpochMS() - t);
//	//	xx::CoutN(sql);
//	//	xx::CoutN(c1);
//	//}
//	//catch (int const& e) {
//	//	std::cout << conn.lastError << std::endl;
//	//	return e;
//	//}
//	//return 0;
//}
