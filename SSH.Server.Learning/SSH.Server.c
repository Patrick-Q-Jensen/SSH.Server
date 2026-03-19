// SSH.Server.Learning.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <process.h>

#pragma comment(lib, "Ws2_32.lib")

#define DEFAULT_PORT "27015"
#define DEFAULT_BUFLEN 1024
const char versionDeclaration[] = "SSH-2.0-MySSHServer_1.0\r\n";
WSADATA wsaData;
const int payloadStartIndex = 22;
const char blockSize = 8;
const char minPaddingLength = 4;

const char *supportedKexAlgosList[10] = {
	"diffie-hellman-group14-sha1", // Kex Algorithms
	"rsa-sha2-256", // Server Host Key Algorithms
	"aes128-ctr", // Encryption Algorithms Client to Server
	"aes128-ctr", // Encryption Algorithms Server to Client
	"hmac-sha2-256", // MAC Algorithms Client to Server
	"hmac-sha2-256", // MAC Algorithms Server to Client
	"none", // Compression Algorithms Client to Server
	"none", // Compression Algorithms Server to Client
	"", //Languages Client to Server
	"" //Languages Server to Client
};

typedef struct SshPacket
{
	uint32_t packetLength;
	unsigned char paddingLength;
	char* payload;
	char* mac;
} SshPacket;


static int FirstIndexOfTarget(char data[], int dataLength, char target, int startIndex) {
	for (int i = startIndex; i < dataLength; i++) {
		if (data[i] == target) {
			return i;
		}
	}
	return -1;
}

bool VerifySshVersionDeclaration(char data[], int dataLength) {
	if (data[0] != 'S' || data[1] != 'S' || data[2] != 'H' || data[3] != '-') {
		printf("Received data did not adhere to the the expected SSH version declaration, it did not start with SSH- \n");
		return false;
	}

	printf("Received declation of SSH protocol version: ");
	for (int i = 4; i < dataLength; i++) {
		if (data[i] == '-') {
			break;
		}
		putchar(data[i]);
	}
	printf("\n");

	// We only support 2.0 for now
	if (data[4] != '2' || data[5] != '.' || data[6] != '0')
	{
		printf("Received SSH protocol version declaration was not the expected version, 2.0\n");
		return false;
	}
	
	printf("Received declaration of SSH software version: ");

	for (int i = 8; i < dataLength; i++) {
		if (data[i] == '\n') {
			break;
		}
		putchar(data[i]);
	}

	printf("\n");
	return true;
}
//
//SshPacket* CreateSshPacket(uint32_t packetlength, unsigned char paddingLength, char* payload, char* mac) {
//	SshPacket* sshPacket = malloc(sizeof(SshPacket));
//	if (sshPacket == NULL) {
//		return NULL;
//	}
//	sshPacket->packetLength = packetlength;
//	sshPacket->paddingLength = paddingLength;
//	sshPacket->payload = payload;
//	sshPacket->mac = mac;
//}

//SshPacket* ParsePacket(char data[], int dataLength) {
//	uint32_t packetLength = 
//		((uint32_t)(unsigned char)data[0] << 24) | 
//		((uint32_t)(unsigned char)data[1] << 16) |
//		((uint32_t)(unsigned char)data[2] << 8)  |
//		(uint32_t)(unsigned char)data[3];
//
//	unsigned char paddingLength = (unsigned char)data[4];
//}

//char* concat(const char* a, const char* b) {
//	int lena = strlen(a);
//	int lenb = strlen(b);
//	char* con = malloc(lena + lenb + 1);
//	// copy & concat (including string termination)
//	memcpy(con, a, lena);
//	memcpy(con + lena, b, lenb + 1);
//	return con;
//}

void printCharArrayAsHex(unsigned char* data, int length) {
	for (int i = 0; i < length; i++) {
		printf("%02x ", data[i]);
	}
	printf("\n");
}

typedef struct Payload {
	char* payload;
	int payloadLength;
} Payload;

Payload* CreatePayload(char* payload, int payloadLength) {
	Payload* p = malloc(sizeof(Payload));
	if (p == NULL) {
		return NULL;
	}
	p->payload = payload;
	p->payloadLength = payloadLength;
	return p;
}

Payload* ParseKexInitPayload(const unsigned char* payload, uint32_t payloadLength)
{
	if (payload == NULL || payloadLength < 4u + 1u) /* minimal sanity (Cookie + Algos + First Kex follows */
		return 0;

	uint32_t index = 0;
	//unsigned char buffer[50];

	char* chosenAlgoList[10] = { NULL };
	char* responsePayLoad = NULL;
	int responsePayloadLength = 1 + 16 + 1 + 4; // message type + cookie + kex packet follows + reserved uint32
	memcpy(chosenAlgoList, supportedKexAlgosList, sizeof(chosenAlgoList));

	/* There are 10 name-lists, each encoded as: uint32 length (big-endian) + bytes */
	for (int list = 0; list < 10; ++list) {
		/* ensure 4-byte length available */
		if (index + 4u > payloadLength) return -1;

		uint32_t listLength =
			((uint32_t)payload[index] << 24) |
			((uint32_t)payload[index + 1] << 16) |
			((uint32_t)payload[index + 2] << 8) |
			((uint32_t)payload[index + 3]);
		index += 4u;

		/* bounds check for the list bytes */
		if (index + listLength > payloadLength) return -1;

		//memset(buffer, 0, sizeof buffer);

		/* listLen may be zero (empty list) */
		if (listLength == 0u) {
			printf("list %d: <empty>\n", list);
		}
		else {

			//TODO Go through the received list of supported algos and find the first match with our supported list

			//for (int i = 0; i < listLength; i++) {
			//	if (payload[index + i] == 44)
			//	{
			//		printf("\n");
			//		puts(supportedKexAlgosList[list]);
			//		puts(buffer);
			//		printf("\n");

			//		if (supportedKexAlgosList[list] == buffer)
			//		{
			//			printf("BINGO\n");
			//			memcpy(chosenAlgoList[list], buffer, listLength);
			//		}
			//	}

			//	buffer[i] = payload[index + i];
			//	putchar(payload[index + i]);
			//}

			//printf("\n");
		}

		index += listLength;
	}

	//unsigned char buf[10][4];
	uint32_t sizes[10];

	for (int i = 0; i < 10; i++)
	{
		if (chosenAlgoList[i] == NULL)
		{
			continue;
		}
		sizes[i] = (uint32_t)strlen(chosenAlgoList[i]);
		responsePayloadLength += 4 + sizes[i];
	}


	/* next: byte first_kex_packet_follows, then uint32 reserved */
	if (index + 1u > payloadLength) return -1;

	unsigned char first_kex_packet_follows = payload[index++];
	printf("first_kex_packet_follows: %d\n", first_kex_packet_follows);

	char* returnPayload = malloc(responsePayloadLength * sizeof(char));
	//char* returnPayload;
	//printCharArrayAsHex((unsigned char*)returnPayload, responsePayloadLength);
	returnPayload[0] = 20; // KexInit message type
	srand((unsigned)time(NULL));
	//randomly generate a cookie for the response
	for (int i = 0; i < 16; i++)
	{
		returnPayload[1 + i] = rand() % 256;
	}

	int idx = 17;
	unsigned char buf[4];
	for (int i = 0; i < 10; i++) {
		printf("Handling chosen algo %d \n", i);
		printf("Index: %d \n", idx);
		printf("Size of algo name: %d \n", sizes[i]);
		puts(chosenAlgoList[i]);
		buf[0] = (unsigned char)((sizes[i] >> 24) & 0xFF);
		buf[1] = (unsigned char)((sizes[i] >> 16) & 0xFF);
		buf[2] = (unsigned char)((sizes[i] >> 8) & 0xFF);
		buf[3] = (unsigned char)(sizes[i] & 0xFF);
		memcpy(&returnPayload[idx], buf, 4);
		idx += 4;
		if (sizes[i] != 0) {
			memcpy(&returnPayload[idx], chosenAlgoList[i], sizes[i]);
			idx += sizes[i] + 0;
		}
	}
	printf("Setting first kex packet follows to 0 at index: %d \n", idx);
	returnPayload[idx] = 0;
	idx++;
	buf[0] = 0;
	buf[1] = 0;
	buf[2] = 0;
	buf[3] = 0;
	printf("Setting reserved for future extension at index: %d \n", idx);
	memcpy(&returnPayload[idx], buf, 4);
	idx += 4;

	if (responsePayloadLength != idx) {
		printf("WARN - Calculated response payload length does not match the actual length of the response payload, something went wrong. Calculated length: %d, actual length: %d \n", responsePayloadLength, idx);
	}

	//printCharArrayAsHex((unsigned char*)returnPayload, responsePayloadLength);

	Payload* response = CreatePayload(returnPayload, responsePayloadLength);

	return response;
}

char CalculatePaddingLength(int payloadLength) {
	char paddingLength = minPaddingLength;
	int packetLength = 1 + payloadLength + paddingLength;
	while ((packetLength + 4) % 8 != 0)
	{
		paddingLength++;
		packetLength = 1 + payloadLength + paddingLength;
	}
	return paddingLength;
}

unsigned __stdcall HandleClientConnection(void* arg) {
	SOCKET clientSocket = (SOCKET)(uintptr_t)arg;
	printf("Client handler thread started (socket %llu)\n", (unsigned long long)clientSocket);

	char recvbuf[DEFAULT_BUFLEN];
	int iResult;
	int iSendResult;
	int receivedPacketCount = 0;
	int receivedByteCount = 0;
	int messagePacketNo = 0;
	int recvbuflen = DEFAULT_BUFLEN;
	uint32_t packetLength = NULL;
	char* packet = NULL;
	unsigned char paddingLength;
	unsigned char messageType;

	//char cookie[16];
	
	//When we pass the payload we skip the message type and cookie, so we start at index 22 (4 for packet length, 1 for padding length, 1 for message type, 16 for cookie)
	int payloadStartIndex = 22;

	bool clientSshVersionVerified = false;

	// Receive until the peer shuts down the connection
	do {
		iResult = recv(clientSocket, recvbuf, recvbuflen, 0);
		if (iResult > 0) {
			if (clientSshVersionVerified == false)
			{
				printf("Bytes received: %d\n", iResult);

				clientSshVersionVerified = VerifySshVersionDeclaration(recvbuf, iResult);
				if (clientSshVersionVerified == false)
				{
					puts("The received data could not be verified as a proper SSH header, connection will be closed");
					closesocket(clientSocket);
					//WSACleanup();
					return 1;
				}
				iSendResult = send(clientSocket, versionDeclaration, strlen(versionDeclaration), 0);
				if (iSendResult == SOCKET_ERROR) {
					printf("send failed: %d\n", WSAGetLastError());
					closesocket(clientSocket);
					//WSACleanup();
					return 1;
				}
				printf("Version declaration sent: ");
				printf("%s", versionDeclaration);
				continue;
			}

			if (packetLength == NULL)
			{
				packetLength =
					((uint32_t)(unsigned char)recvbuf[0] << 24) |
					((uint32_t)(unsigned char)recvbuf[1] << 16) |
					((uint32_t)(unsigned char)recvbuf[2] << 8) |
					(uint32_t)(unsigned char)recvbuf[3];
				packetLength += 4;
				if (packet != NULL) {
					free(packet);
					packet = NULL;
				}
				packet = malloc(packetLength * sizeof(char));
				printf("Packet length: %d\n", packetLength);
			}

			if (packet == NULL) {
				printf("Failed to create packet array");
				return 1;
			}

			for (int i = 0; i < iResult; i++) {
				packet[receivedByteCount] = recvbuf[i];
				receivedByteCount++;
			}

			if (receivedByteCount == packetLength) {
				//receivedPacketCount++;
				printf("Recived full packet of length %d\n", receivedByteCount);
				paddingLength = packet[4];
				printf("Padding length: %d\n", paddingLength);
				int payloadLength = packetLength - paddingLength - 1 - 16;

				messageType = packet[5];

				Payload* responsePayload = NULL;

				switch (messageType)
				{
					case 20:
						responsePayload = ParseKexInitPayload((unsigned char*)packet + payloadStartIndex, payloadLength);
						break;
				default:
					break;
				}
				if (responsePayload == NULL)
				{
					puts("WARN - failed to generate response payload");
					return 1;
				}
				
				char paddingLength = CalculatePaddingLength(responsePayload->payloadLength);
				int responsePacketLength = 1 + responsePayload->payloadLength + paddingLength;
				if ((responsePacketLength + 4) % 8 != 0)
				{
					printf("WARN - Response packet length is not divisible by 8, length: %d \n", responsePacketLength + 4);
				}
				unsigned char* responsePacket = malloc((responsePacketLength + 4) * sizeof(char));
				if (responsePacket == NULL)
				{
					puts("WARN - failed to malloc response packet");
					return 1;
				}
				//unsigned char buf[4];
				responsePacket[0] = (unsigned char)((responsePacketLength >> 24) & 0xFF);
				responsePacket[1] = (unsigned char)((responsePacketLength >> 16) & 0xFF);
				responsePacket[2] = (unsigned char)((responsePacketLength >> 8) & 0xFF);
				responsePacket[3] = (unsigned char)(responsePacketLength & 0xFF);

				responsePacket[4] = paddingLength;
				memcpy(&responsePacket[5], responsePayload->payload, responsePayload->payloadLength);
				for (int i = 0; i < paddingLength; i++)
				{
					printf("Setting padding byte at index %d to random value \n", 5 + responsePayload->payloadLength + i);
					responsePacket[responsePayload->payloadLength + 5 + i] = rand() % 256;
				}

				//printCharArrayAsHex(responsePacket, responsePacketLength + 4);

				uint32_t test =
					((uint32_t)(unsigned char)responsePacket[0] << 24) |
					((uint32_t)(unsigned char)responsePacket[1] << 16) |
					((uint32_t)(unsigned char)responsePacket[2] << 8) |
					(uint32_t)(unsigned char)responsePacket[3];

				iSendResult = send(clientSocket, responsePacket, responsePacketLength + 4, 0);
				if (iSendResult == SOCKET_ERROR) {
					printf("send failed: %d\n", WSAGetLastError());
					closesocket(clientSocket);
					return 1;
				}
			}
		}
		else if (iResult == 0)
			printf("Connection closing...\n");
		else {
			printf("recv failed: %d\n", WSAGetLastError());
			closesocket(clientSocket);
			//WSACleanup();
			return 1;
		}
	} while (iResult > 0);
	closesocket(clientSocket);
	return 0;
}

int main()
{
	int iResult;

	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0)
	{
		printf("WSAStartup failed: %d\n", iResult);
		return 1;
	}
	printf("WSAStartup succeeded.\n");


	struct addrinfo* result = NULL, * ptr = NULL, hints;
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	// Resolve the local address and port to be used by the server
	iResult = getaddrinfo("127.0.0.1", DEFAULT_PORT, &hints, &result);
	if (iResult != 0) {
		printf("getaddrinfo failed: %d\n", iResult);
		WSACleanup();
		return 1;
	}
	printf("getaddrinfo succeeded.\n");

	SOCKET ListenSocket = INVALID_SOCKET;
	ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (ListenSocket == INVALID_SOCKET)
	{
		printf("Error at socket(): %ld\n", WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
		return 1;
	}
	printf("socket() succeeded.\n");


	// Setup the TCP listening socket
	iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
	if (iResult == SOCKET_ERROR) {
		printf("bind failed with error: %d\n", WSAGetLastError());
		freeaddrinfo(result);
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}

	printf("Succesfully bound socket\n");
	freeaddrinfo(result);


	if (listen(ListenSocket, SOMAXCONN) == SOCKET_ERROR) {
		printf("Listen failed with error: %ld\n", WSAGetLastError());
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}
	
	printf("Listening on socket\n");
	
	for (;;)
	{
		// Accept a client socket
		SOCKET ClientSocket = accept(ListenSocket, NULL, NULL);
		if (ClientSocket == INVALID_SOCKET) {
			printf("accept failed: %d\n", WSAGetLastError());
			break;
		}

		printf("Accepted client socket %llu\n", (unsigned long long)ClientSocket);
		HANDLE clientThreadHandle = NULL;
		clientThreadHandle = (HANDLE)_beginthreadex(
			NULL,                   // default security attributes
			0,                      // use default stack size  
			HandleClientConnection,       // thread function name
			(void*)(uintptr_t)ClientSocket,          // argument to thread function 
			0,                      // use default creation flags 
			NULL);   // returns the thread identifier

		if (clientThreadHandle == 0) {
			// thread creation failed — close socket and continue/exit
			printf("_beginthreadex failed: %u\n", (unsigned)GetLastError());
			closesocket(ClientSocket);
		}
		else {
			// we don't need the handle; close it and let the thread run
			printf("Created thread, closing handle");
			CloseHandle(clientThreadHandle);
		}
	}



	//HandleClientConnection(ClientSocket);

	//char recvbuf[DEFAULT_BUFLEN];
	//char versionDeclaration[] = "SSH-2.0-MySSHServer_1.0\r\n";
	//int iSendResult;
	//int recvbuflen = DEFAULT_BUFLEN;
	//bool clientSshVersionVerified = false;
	//
	//// Receive until the peer shuts down the connection
	//do {
	//	iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);
	//	if (iResult > 0) {
	//		if (clientSshVersionVerified == false)
	//		{
	//			printf("Bytes received: %d\n", iResult);

	//			clientSshVersionVerified = VerifySshVersionDeclaration(recvbuf, iResult);
	//			if (clientSshVersionVerified == false)
	//			{
	//				puts("The received data could not be verified as a proper SSH header, connection will be closed");
	//				closesocket(ClientSocket);
	//				closesocket(ListenSocket);
	//				WSACleanup();
	//				return 1;
	//			}
	//			iSendResult = send(ClientSocket, versionDeclaration, iResult, 0);
	//			if (iSendResult == SOCKET_ERROR) {
	//				printf("send failed: %d\n", WSAGetLastError());
	//				closesocket(ClientSocket);
	//				WSACleanup();
	//				return 1;
	//			}
	//			printf("Version declaration sent: ");
	//			printf(versionDeclaration);
	//			continue;
	//		}
	//		puts("Received packet from client");

	//		SshPacket* receivedPacket = ParsePacket(recvbuf, iResult);



	//		//char *recSshHeader = malloc(firstIndex * sizeof(char));
	//		// Echo the buffer back to the sender
	//		//
	//		//if (iSendResult == SOCKET_ERROR) {
	//		//	printf("send failed: %d\n", WSAGetLastError());
	//		//	closesocket(ClientSocket);
	//		//	WSACleanup();
	//		//	return 1;
	//		//}
	//		//printf("Bytes sent: %d\n", iSendResult);
	//	}
	//	else if (iResult == 0)
	//		printf("Connection closing...\n");
	//	else {
	//		printf("recv failed: %d\n", WSAGetLastError());
	//		closesocket(ClientSocket);
	//		WSACleanup();
	//		return 1;
	//	}
	//} while (iResult > 0);
}







// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
