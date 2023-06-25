#define MY_DLL 1069

std::vector<char> getResourceData();
void saveResourceDataToFile(const std::filesystem::path& filePath);
std::string getSystemUsername();

namespace fs = std::filesystem;
