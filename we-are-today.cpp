#include <iostream>
#include <string>
#include <Windows.h>
#include <ctime>
#include <sstream>

bool setWallpaper(const std::string& imagename);
bool fileExists(const std::string& filename);
bool setWallpaperStyle(int style);
void PopupMessage (const std::wstring& message);
void ErrorMessage (const std::wstring& message);

int main() {
    const std::string images[7] = {"sunday", "monday", "tuesday", "wednesday", "thursday", "friday", "saturday"};
    
    time_t rawtime;
    time(&rawtime);
    struct tm* timeinfo = localtime(&rawtime);
    
    int wday = timeinfo->tm_wday;
    std::cout << "Today is " << images[wday] << "\n";
    
    if (!setWallpaperStyle(6)) { // 6 is for "Fit"
    ErrorMessage(L"Failed to set wallpaper style. ");
        std::cerr << "Failed to set wallpaper style." << "\n";
    }
    
    if (!setWallpaper(images[wday])) {
        ErrorMessage(L"Failed to set the wallpaper. ");
        std::cerr << "Failed to set the wallpaper." << "\n";
        return 1;
    }
    
    return 0;
}

void PopupMessage (const std::wstring& message) {
    MessageBoxW(NULL, message.c_str(), L"Information from Wallpaper Changer", MB_OK | MB_ICONINFORMATION);
}
void ErrorMessage (const std::wstring& message) {
    MessageBoxW(NULL, message.c_str(), L"Error from Wallpaper Changer", MB_OK | MB_ICONERROR);
}

bool fileExists(const std::string& filename) {
    DWORD fileAttributes = GetFileAttributesA(filename.c_str());
    return (fileAttributes != INVALID_FILE_ATTRIBUTES && !(fileAttributes & FILE_ATTRIBUTE_DIRECTORY));
}

bool setWallpaperStyle(int style) {
    HKEY hKey;
    LONG result;

    // Open the registry key
    result = RegOpenKeyExA(HKEY_CURRENT_USER, "Control Panel\\Desktop", 0, KEY_SET_VALUE, &hKey);
    if (result != ERROR_SUCCESS) {
        ErrorMessage(L"Failed to open registry key.  Error code: " + std::to_wstring(result));
        std::cerr << "Failed to open registry key.  Error code: " << result << "\n";
        return false;
    }

    // Convert style to string
    std::string styleStr = std::to_string(style);
 
    // Set the wallpaper style
    result = RegSetValueExA(hKey, "WallpaperStyle", 0, REG_SZ, (BYTE*)styleStr.c_str(), styleStr.length() + 1);
    if (result != ERROR_SUCCESS) {
        ErrorMessage(L"Failed to set WallpaperStyle.  Error code: " + std::to_wstring(result));
        std::cerr << "Failed to set WallpaperStyle.  Error code: " << result << "\n";
        RegCloseKey(hKey);
        return false;
    }

    // Set TileWallpaper to 0
    std::string tileValue = "0";
    result = RegSetValueExA(hKey, "TileWallpaper", 0, REG_SZ, (BYTE*)tileValue.c_str(), tileValue.length() + 1);
    if (result != ERROR_SUCCESS) {
        ErrorMessage(L"Failed to set TileWallpaper.  Error code: " + std::to_wstring(result));
        std::cerr << "Failed to set TileWallpaper.  Error code: " << result << "\n";
        RegCloseKey(hKey);
        return false;
    }

    // Close the registry key
    RegCloseKey(hKey);
    return true;
}

bool setWallpaper(const std::string& imagename) {
    const std::string formats[] = {".jpg", ".png", ".bmp"};
    std::string wallpaperPath;
    
    // Use absolute path
    std::string basePath = R"(C:\Users\Public\wallpaper-changer\wallpapers\)";
    
    // Try different image formats
    for (const auto& format : formats) {
        wallpaperPath = basePath + imagename + format;
        if (fileExists(wallpaperPath)) {
            break;
        }
    }
    
    if (!fileExists(wallpaperPath)) {
        std::wostringstream errormsg;
        errormsg << L"Wallpaper file not found for " << imagename.c_str() << L" in any supported format. ";
        ErrorMessage(errormsg.str());
        std::cerr << "Wallpaper file not found for " << imagename << " in any supported format. \n";
        return false;
    }
    
    std::cout << "Attempting to set wallpaper: " << wallpaperPath << "\n";
    
    // Use Windows API function
    if (!SystemParametersInfoA(SPI_SETDESKWALLPAPER, 0, (void*)wallpaperPath.c_str(), SPIF_UPDATEINIFILE | SPIF_SENDCHANGE)) {
        DWORD error = GetLastError();
        ErrorMessage(L"Failed to set wallpaper. Error code: " + std::to_wstring(error));
        std::cerr << "Failed to set wallpaper. Error code: " << error << "\n";
        
        if (error == ERROR_FILE_NOT_FOUND) {
            std::cerr << "The specified file was not found.\n";
        } else if (error == ERROR_ACCESS_DENIED) {
            std::cerr << "Access denied. Check if you have the necessary permissions.\n";
        } else if (error == ERROR_INVALID_PARAMETER) {
            std::cerr << "Invalid parameter. The file might not be a valid image format.\n";
        }
        
        return false;
    }
    
    std::cout << "Wallpaper set to: " << wallpaperPath << "\n";
    return true;
}
