#define MY_DLL 1069

std::vector<char> getResourceData();
void saveResourceDataToFile(const std::filesystem::path& filePath);
std::string getSystemUsername();
void copyDll(const std::filesystem::path& sourcePath, const std::filesystem::path& destinationPath);

namespace fs = std::filesystem;
