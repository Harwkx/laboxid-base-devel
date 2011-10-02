#include <winsock2.h>
#include <windows.h>
#include <stdio.h>

#pragma comment( lib, "ws2_32.lib" )

//Prototypen
int StartWinsock(void);

int main()
{
	//Testausgabe
	printf("Socket Client\n");

	//Variablen initialisieren
	long returnvalue;
	char buf[256];
	SOCKET sock;
	SOCKADDR_IN addr;

	//Socket Verfügbarkeit prüfen
	returnvalue = StartWinsock();
	if(returnvalue != 0)
	{
		printf("Fehler: StartWinsock Fehlercode: %d!\n",returnvalue);
		return 1;
	}
	else
	{
		printf("Winsock gestartet!\n");
	}

	//Socket initialisieren
	sock = socket(AF_INET,SOCK_STREAM,0);
	if(sock == INVALID_SOCKET)
	{
		printf("Fehler: Der Socket konnte nicht erstellt werden, fehler code: %d\n",WSAGetLastError());
		return 1;
	}
	else
	{
		printf("Socket erstellt!\n");
	}

	//Port und IP übergabe
	memset(&addr,0,sizeof(SOCKADDR_IN));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(12345);
	addr.sin_addr.s_addr = inet_addr("127.0.0.1");

	//Verbindungsaufbau
	returnvalue = connect(sock, (SOCKADDR*)&addr, sizeof(SOCKADDR));
	if(returnvalue == SOCKET_ERROR)
	{
		printf("Fehler: connect gescheitert, fehler code: %d\n",WSAGetLastError());
		return 1;
	}
	else
	{
		printf("Verbindung hergestellt mit 127.0.0.1\n");
	}

	//FUNKTION: Endlos-Chat
	while(1)
	{
		//Senden von Output
		printf("\nSenden: ");
		fgets(buf,255,stdin);
		send(sock,buf,255,0);
		printf("\nNachricht gesendet!\n\n");

		//Warten auf Input
		printf("[-] Warte auf Input ...\n\n");
		memset(buf,0,sizeof(buf));
		returnvalue = recv(sock,buf,255,0);
		if(returnvalue == 0 || returnvalue == SOCKET_ERROR)
		{
			printf("Fehler: recv, fehler code: %d\n",WSAGetLastError());
		}
		else
		{
			buf[returnvalue]=0;
			printf("Antwort: %s\n\n",buf);
		}
	}


	//Programmende
	printf("\n");
	return 0;
}

int StartWinsock()
{
	WSADATA wsa;
	return WSAStartup(MAKEWORD(2,0),&wsa);
}