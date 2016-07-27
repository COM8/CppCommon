/*!
    \file environment.cpp
    \brief Environment management implementation
    \author Ivan Shynkarenka
    \date 27.07.2016
    \copyright MIT License
*/

#include "system/environment.h"

#include <regex>
#include <fstream>
#include <sstream>

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#elif defined(linux) || defined(__linux) || defined(__linux__)
#include <sys/stat.h>
#endif

namespace CppCommon {

bool Environment::Is32BitOS()
{
    return !Is64BitOS();
}

bool Environment::Is64BitOS()
{
#if defined(_WIN32) || defined(_WIN64)
#if defined(_WIN64)
    return true;
#elif defined(_WIN32)
    BOOL bWow64Process = FALSE;
    return IsWow64Process(GetCurrentProcess(), &bWow64Process) && bWow64Process;
#endif
#elif defined(linux) || defined(__linux) || defined(__linux__)
    struct stat buffer;
    return (stat("/lib64/ld-linux-x86-64.so.2", &buffer) == 0);
#endif
}

bool Environment::Is32BitProcess()
{
    return !Is64BitProcess();
}

bool Environment::Is64BitProcess()
{
#if defined(_WIN32) || defined(_WIN64)
#if defined(_WIN64)
    return true;
#elif defined(_WIN32)
    return false;
#endif
#elif defined(unix) || defined(__unix) || defined(__unix__)
#if defined(__x86_64__) || defined(__ppc64__)
    return true;
#else
    return false;
#endif
#endif
}

bool Environment::IsDebug()
{
    return !IsRelease();
}

bool Environment::IsRelease()
{
#if defined(NDEBUG)
    return true;
#else
    return false;
#endif
}

std::string Environment::OSVersion()
{
#if defined(_WIN32) || defined(_WIN64)
    typedef void (WINAPI *PGNSI)(LPSYSTEM_INFO);
    typedef BOOL (WINAPI *PGPI)(DWORD, DWORD, DWORD, DWORD, PDWORD);

    OSVERSIONINFOEX osvi;
    ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);

    BOOL bOsVersionInfoEx = GetVersionEx((OSVERSIONINFO*)&osvi);
    if (bOsVersionInfoEx == 0)
        return "<windows>";

    SYSTEM_INFO si;
    ZeroMemory(&si, sizeof(SYSTEM_INFO));

    PGNSI pGNSI = (PGNSI)GetProcAddress(GetModuleHandle("kernel32.dll"), "GetNativeSystemInfo");
    if (pGNSI != nullptr)
        pGNSI(&si);
    else
        GetSystemInfo(&si);

    if ((osvi.dwPlatformId != VER_PLATFORM_WIN32_NT) || (osvi.dwMajorVersion <= 4))
    {
        return "<windows>";
    }

    std::stringstream os;

    // Windows version
    os << "Microsoft ";
    if (osvi.dwMajorVersion >= 6)
    {
        if (osvi.dwMajorVersion == 10)
        {
            if (osvi.dwMinorVersion == 0)
            {
                if (osvi.wProductType == VER_NT_WORKSTATION)
                    os << "Windows 10 ";
                else
                    os << "Windows Server 2016 ";
            }
        }
        else if (osvi.dwMajorVersion == 6)
        {
            if (osvi.dwMinorVersion == 3)
            {
                if (osvi.wProductType == VER_NT_WORKSTATION)
                    os << "Windows 8.1 ";
                else
                    os << "Windows Server 2012 R2 ";
            }
            else if (osvi.dwMinorVersion == 2)
            {
                if (osvi.wProductType == VER_NT_WORKSTATION)
                    os << "Windows 8 ";
                else
                    os << "Windows Server 2012 ";
            }
            else if (osvi.dwMinorVersion == 1)
            {
                if (osvi.wProductType == VER_NT_WORKSTATION)
                    os << "Windows 7 ";
                else
                    os << "Windows Server 2008 R2 ";
            }
            else if (osvi.dwMinorVersion == 0)
            {
                if (osvi.wProductType == VER_NT_WORKSTATION)
                    os << "Windows Vista ";
                else
                    os << "Windows Server 2008 ";
            }
        }

        DWORD dwType;
        PGPI pGPI = (PGPI)GetProcAddress(GetModuleHandle("kernel32.dll"), "GetProductInfo");
        pGPI(osvi.dwMajorVersion, osvi.dwMinorVersion, 0, 0, &dwType);
        switch (dwType)
        {
            case PRODUCT_ULTIMATE:
                os << "Ultimate Edition";
                break;
            case PRODUCT_PROFESSIONAL:
                os << "Professional";
                break;
            case PRODUCT_HOME_PREMIUM:
                os << "Home Premium Edition";
                break;
            case PRODUCT_HOME_BASIC:
                os << "Home Basic Edition";
                break;
            case PRODUCT_ENTERPRISE:
                os << "Enterprise Edition";
                break;
            case PRODUCT_BUSINESS:
                os << "Business Edition";
                break;
            case PRODUCT_STARTER:
                os << "Starter Edition";
                break;
            case PRODUCT_CLUSTER_SERVER:
                os << "Cluster Server Edition";
                break;
            case PRODUCT_DATACENTER_SERVER:
                os << "Datacenter Edition";
                break;
            case PRODUCT_DATACENTER_SERVER_CORE:
                os << "Datacenter Edition (core installation)";
                break;
            case PRODUCT_ENTERPRISE_SERVER:
                os << "Enterprise Edition";
                break;
            case PRODUCT_ENTERPRISE_SERVER_CORE:
                os << "Enterprise Edition (core installation)";
                break;
            case PRODUCT_ENTERPRISE_SERVER_IA64:
                os << "Enterprise Edition for Itanium-based Systems";
                break;
            case PRODUCT_SMALLBUSINESS_SERVER:
                os << "Small Business Server";
                break;
            case PRODUCT_SMALLBUSINESS_SERVER_PREMIUM:
                os << "Small Business Server Premium Edition";
                break;
            case PRODUCT_STANDARD_SERVER:
                os << "Standard Edition";
                break;
            case PRODUCT_STANDARD_SERVER_CORE:
                os << "Standard Edition (core installation)";
                break;
            case PRODUCT_WEB_SERVER:
                os << "Web Server Edition";
                break;
        }
    }
    else if ((osvi.dwMajorVersion == 5) && (osvi.dwMinorVersion == 2))
    {
        if (GetSystemMetrics(SM_SERVERR2))
            os << "Windows Server 2003 R2, ";
        else if (osvi.wSuiteMask & VER_SUITE_STORAGE_SERVER)
            os << "Windows Storage Server 2003";
        else if (osvi.wSuiteMask & VER_SUITE_WH_SERVER)
            os << "Windows Home Server";
        else if ((osvi.wProductType == VER_NT_WORKSTATION) && (si.wProcessorArchitecture==PROCESSOR_ARCHITECTURE_AMD64))
            os << "Windows XP Professional x64 Edition";
        else
            os << "Windows Server 2003, ";
        if (osvi.wProductType != VER_NT_WORKSTATION)
        {
            if (si.wProcessorArchitecture==PROCESSOR_ARCHITECTURE_IA64)
            {
                if (osvi.wSuiteMask & VER_SUITE_DATACENTER)
                    os << "Datacenter Edition for Itanium-based Systems";
                else if (osvi.wSuiteMask & VER_SUITE_ENTERPRISE)
                    os << "Enterprise Edition for Itanium-based Systems";
            }
            else if (si.wProcessorArchitecture==PROCESSOR_ARCHITECTURE_AMD64)
            {
                if (osvi.wSuiteMask & VER_SUITE_DATACENTER)
                    os << "Datacenter x64 Edition";
                else if (osvi.wSuiteMask & VER_SUITE_ENTERPRISE)
                    os << "Enterprise x64 Edition";
                else
                    os << "Standard x64 Edition";
            }
            else
            {
                if (osvi.wSuiteMask & VER_SUITE_COMPUTE_SERVER)
                    os << "Compute Cluster Edition";
                else if (osvi.wSuiteMask & VER_SUITE_DATACENTER)
                    os << "Datacenter Edition";
                else if (osvi.wSuiteMask & VER_SUITE_ENTERPRISE)
                    os << "Enterprise Edition";
                else if (osvi.wSuiteMask & VER_SUITE_BLADE)
                    os << "Web Edition";
                else
                    os << "Standard Edition";
            }
        }
    }
    else if ((osvi.dwMajorVersion == 5) && (osvi.dwMinorVersion == 1))
    {
        os << "Windows XP ";
        if (osvi.wSuiteMask & VER_SUITE_PERSONAL)
            os << "Home Edition";
        else
            os << "Professional";
    }
    else if ((osvi.dwMajorVersion == 5) && (osvi.dwMinorVersion == 0))
    {
        os << "Windows 2000 ";
        if (osvi.wProductType == VER_NT_WORKSTATION)
            os <<  "Professional";
        else
        {
            if (osvi.wSuiteMask & VER_SUITE_DATACENTER)
                os << "Datacenter Server";
            else if (osvi.wSuiteMask & VER_SUITE_ENTERPRISE)
                os << "Advanced Server";
            else
                os << "Server";
        }
    }

    // Windows Service Pack version
    if (std::strlen(osvi.szCSDVersion) > 0)
        os << " " << osvi.szCSDVersion;

    // Windows build
    os << " (build " << osvi.dwBuildNumber << ")";

    // Windows architecture
    if ( osvi.dwMajorVersion >= 6 )
    {
        if (si.wProcessorArchitecture==PROCESSOR_ARCHITECTURE_AMD64)
            os <<  ", 64-bit";
        else if (si.wProcessorArchitecture==PROCESSOR_ARCHITECTURE_INTEL)
            os << ", 32-bit";
    }

    return os.str();
#elif defined(linux) || defined(__linux) || defined(__linux__)
    static std::regex pattern("DISTRIB_DESCRIPTION=\"(.*)\"");

    std::string line;
    std::ifstream stream("/etc/lsb-release");
    while (getline(stream, line))
    {
        std::smatch matches;
        if (std::regex_match(line, matches, pattern))
            return matches[1];
    }

    return "<linux>";
#elif defined(unix) || defined(__unix) || defined(__unix__)
    return "<unix>";
#endif
}

} // namespace CppCommon
