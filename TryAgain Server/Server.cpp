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
	char buf[4096];
	SOCKET acceptSocket;
	SOCKET connectedSocket;
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
	acceptSocket = socket(AF_INET,SOCK_STREAM,0);
	if(acceptSocket == INVALID_SOCKET)
	{
		printf("Fehler: Der Socket konnte nicht erstellt werden, fehler code: %d\n",WSAGetLastError());
		return 1;
	}
	else
	{
		printf("Socket erstellt!\n");
	}

	//Verbindung initialisieren
	memset(&addr,0,sizeof(SOCKADDR_IN));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(12345);
	addr.sin_addr.s_addr = ADDR_ANY;
	returnvalue = bind(acceptSocket,(SOCKADDR*)&addr,sizeof(SOCKADDR_IN));

	if(returnvalue == SOCKET_ERROR)
	{
		printf("Fehler: bind, fehler code: %d\n",WSAGetLastError());
		return 1;
	}
	else
	{
		printf("Socket an port 12345 gebunden\n");
	}

	//Verbindung bereitstellen
	returnvalue = listen(acceptSocket,10);
	if(returnvalue == SOCKET_ERROR)
	{
		printf("Fehler listen, fehler code: %d",WSAGetLastError());
		return 1;
	}
	else
	{
		printf("AcceptSocket ist im Listen-Modus ...\n");
	}

	//Auf Verbindung warten und herstellen
	connectedSocket = accept(acceptSocket,NULL,NULL);
	if(connectedSocket == INVALID_SOCKET)
	{
		printf("Fehler: accept, fehler code: %d\n",WSAGetLastError());
		return 1;
	}
	else
	{
		printf("Neue Verbindung wurde akzeptiert!\n");
	}

	//FUNKTION: Endlos-Chat
	while(1)
	{
		//Warten auf Input
		printf("[-] Warte auf Input ...\n\n");
		memset(buf,0,sizeof(buf));

		
		//Senden von Output
		printf("Senden: ");
		fgets(buf,4095,stdin);
		send(connectedSocket,buf,4095,0);
		printf("\nNachricht gesendet!\n\n");

		/*OVERLAPPED ovl = {0};
		if (!ReadFile((HANDLE)connectedSocket, buf, 4095, NULL, &ovl))
		{
			DWORD err = GetLastError();
			if (ERROR_IO_PENDING == err) {
				if (!GetOverlappedResult((HANDLE)connectedSocket, &ovl, (LPDWORD)&returnvalue, TRUE))
				{
					returnvalue = SOCKET_ERROR;
					printf("Overlapped I/O failed");
				}
			} else {
				returnvalue = SOCKET_ERROR;
				printf("ReadFile failed with last error %d\n", err);
			}
		} */

		returnvalue = recv(connectedSocket,buf,4095,0);
		if(returnvalue == 0 || returnvalue == SOCKET_ERROR)
		{
			printf("Fehler: recv, fehler code: %d\n",WSAGetLastError());
		}
		else
		{
			buf[returnvalue]=0;
			printf("Antwort: %s\n\n",buf);
		}

		//buf[returnvalue]=0;
		//printf("Antwort: %s\n\n",buf);
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