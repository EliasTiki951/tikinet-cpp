/*
 * ============================================================
 *   Tikinet Wifi C++ - Implementación para Windows
 * ============================================================
 * Escanea y muestra TODOS los puntos de acceso (APs) de cada red,
 * corrigiendo el bug donde "BSSID" era confundido con "SSID".
 * Autor: EliasTiki
 * Licencia: MIT
 * ============================================================
 */
#include "tikinet_wifi.h"
#include <windows.h>
#include <shellapi.h>
#include <cstdlib>
#include <iostream>
#include <algorithm>
#include <map>
#include <iphlpapi.h>

bool isAdmin()
{
    bool admin = false;
    HANDLE hToken = NULL;
    if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken))
    {
        TOKEN_ELEVATION elevation;
        DWORD size = sizeof(TOKEN_ELEVATION);
        if (GetTokenInformation(hToken, TokenElevation, &elevation, sizeof(elevation), &size))
            admin = elevation.TokenIsElevated;
        CloseHandle(hToken);
    }
    return admin;
}

void requestAdminIfNeeded()
{
    if (!isAdmin())
    {
        std::cout << "Solicitando permisos de Administrador...\n";
        char exePath[MAX_PATH];
        GetModuleFileNameA(NULL, exePath, MAX_PATH);
        SHELLEXECUTEINFOA sei = {0};
        sei.cbSize = sizeof(SHELLEXECUTEINFOA);
        sei.lpVerb = "runas";
        sei.lpFile = exePath;
        sei.nShow = SW_NORMAL;
        ShellExecuteExA(&sei);
        exit(0);
    }
}

bool copyToClipboard(const std::string &text)
{
    if (!OpenClipboard(NULL))
        return false;
    EmptyClipboard();
    HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, text.length() + 1);
    if (!hMem)
    {
        CloseClipboard();
        return false;
    }
    memcpy(GlobalLock(hMem), text.c_str(), text.length() + 1);
    GlobalUnlock(hMem);
    SetClipboardData(CF_TEXT, hMem);
    CloseClipboard();
    return true;
}

std::vector<std::string> getLocalIPs()
{
    std::vector<std::string> result;
    ULONG outBufLen = 15000;
    PIP_ADAPTER_INFO pAdapterInfo = (IP_ADAPTER_INFO *)malloc(outBufLen);
    if (pAdapterInfo == NULL)
        return result;

    if (GetAdaptersInfo(pAdapterInfo, &outBufLen) == ERROR_SUCCESS)
    {
        PIP_ADAPTER_INFO pAdapter = pAdapterInfo;
        while (pAdapter != NULL)
        {
            std::string ip = std::string(pAdapter->IpAddressList.IpAddress.String);
            // Filtrar IPs inválidas
            if (!ip.empty() &&
                ip != "0.0.0.0" &&
                ip.find('.') != std::string::npos &&
                ip.find_first_not_of("0123456789.") == std::string::npos)
            {
                result.push_back(ip);
            }
            pAdapter = pAdapter->Next;
        }
    }
    free(pAdapterInfo);
    return result;
}

std::vector<std::string> getConnectedDevices()
{
    std::vector<std::string> result;

    // --- 1. Obtener la salida completa de arp -a ---
    std::string output = execCommand("arp -a");

    /// --- 2. Detectar IPs locales con la API de Windows ---
    std::vector<std::string> localIPs = getLocalIPs();

    // --- 3. Parsear TODAS las entradas de arp -a ---
    std::vector<std::string> reales, bcasts, mcasts;
    std::map<std::string, bool> macVistos;

    size_t s = 0, e;
    while ((e = output.find('\n', s)) != std::string::npos)
    {
        std::string line = trimString(output.substr(s, e - s));
        s = e + 1;
        if (line.empty())
            continue;

        size_t firstSpace = line.find(' ');
        if (firstSpace == std::string::npos)
            continue;

        std::string ip = line.substr(0, firstSpace);
        if (ip.find('.') == std::string::npos)
            continue;
        if (ip.find_first_not_of("0123456789.") != std::string::npos)
            continue;

        size_t macStart = line.find_first_not_of(" \t", firstSpace);
        if (macStart == std::string::npos)
            continue;
        size_t macEnd = line.find_first_of(" \t", macStart);
        if (macEnd == std::string::npos)
            macEnd = line.length();
        std::string mac = line.substr(macStart, macEnd - macStart);

        int dashes = 0;
        for (size_t i = 0; i < mac.length(); i++)
            if (mac[i] == '-')
                dashes++;
        if (dashes < 5)
            continue;

        // Deduplicar por MAC
        if (macVistos.find(mac) != macVistos.end())
            continue;
        macVistos[mac] = true;

        // --- 4. Clasificar por tipo ---
        unsigned int firstOctet = 0;
        size_t dotPos = ip.find('.');
        if (dotPos != std::string::npos)
            firstOctet = atoi(ip.substr(0, dotPos).c_str());

        bool isBroadcast = (ip == "255.255.255.255") ||
                           (ip.length() > 4 && ip.substr(ip.length() - 4) == ".255");
        bool isMulticast = (firstOctet >= 224 && firstOctet <= 239);

        if (isMulticast)
        {
            std::string proto = "Multicast";
            if (ip == "224.0.0.1")
                proto = "All Hosts";
            else if (ip == "224.0.0.2")
                proto = "All Routers";
            else if (ip == "224.0.0.22")
                proto = "IGMPv3";
            else if (ip == "224.0.0.251")
                proto = "mDNS";
            else if (ip == "224.0.0.252")
                proto = "LLMNR";
            else if (ip == "239.255.255.250")
                proto = "SSDP";

            std::string tag = "[" + proto + "]";
            while (tag.length() < 14)
                tag += " ";

            std::string ipPad = ip;
            while (ipPad.length() < 18)
                ipPad += " ";

            mcasts.push_back("  " + tag + ipPad + "  ->  " + mac);
        }
        else if (isBroadcast)
        {
            std::string tag = "[Broadcast]";
            while (tag.length() < 14)
                tag += " ";

            std::string ipPad = ip;
            while (ipPad.length() < 18)
                ipPad += " ";

            bcasts.push_back("  " + tag + ipPad + "  ->  " + mac);
        }
        else
        {
            std::string label = "Dispositivo";
            if (ip.length() >= 2 &&
                (ip.substr(ip.length() - 2) == ".1" ||
                 (ip.length() >= 4 && ip.substr(ip.length() - 4) == ".254")))
            {
                label = "ROUTER";
            }
            for (size_t i = 0; i < localIPs.size(); i++)
            {
                if (localIPs[i] == ip)
                {
                    label = "ESTE EQUIPO";
                    break;
                }
            }

            std::string tag = "[" + label + "]";
            while (tag.length() < 15)
                tag += " ";

            std::string ipPad = ip;
            while (ipPad.length() < 18)
                ipPad += " ";

            reales.push_back("  " + tag + ipPad + "  ->  " + mac);
        }
    }

    // --- 5. Ensamblar el resultado ---
    result.push_back("--- DISPOSITIVOS REALES (" + intToString((int)reales.size()) + ") ---");
    for (size_t i = 0; i < reales.size(); i++)
        result.push_back(reales[i]);

    if (!bcasts.empty())
    {
        result.push_back("");
        result.push_back("--- BROADCAST (" + intToString((int)bcasts.size()) + ") ---");
        result.push_back("  Paquetes enviados a TODOS los dispositivos de la red.");
        result.push_back("  No son dispositivos reales.");
        result.push_back("");
        for (size_t i = 0; i < bcasts.size(); i++)
            result.push_back(bcasts[i]);
    }

    if (!mcasts.empty())
    {
        result.push_back("");
        result.push_back("--- MULTICAST (" + intToString((int)mcasts.size()) + ") ---");
        result.push_back("  Direcciones de GRUPO. No son dispositivos reales.");
        result.push_back("");
        for (size_t i = 0; i < mcasts.size(); i++)
            result.push_back(mcasts[i]);
    }

    return result;
}

std::vector<std::string> getSavedProfiles()
{
    std::vector<std::string> profiles;
    std::string output = execCommand("C:\\Windows\\System32\\netsh.exe wlan show profiles");
    size_t pos = 0;
    while (true)
    {
        size_t findPos = output.find("Perfil de todos los usuarios", pos);
        if (findPos == std::string::npos)
            findPos = output.find("All User Profile", pos);
        if (findPos == std::string::npos)
            break;
        size_t colonPos = output.find(':', findPos);
        if (colonPos != std::string::npos)
        {
            size_t endPos = output.find('\n', colonPos);
            if (endPos == std::string::npos)
                endPos = output.length();
            std::string name = trimString(output.substr(colonPos + 2, endPos - colonPos - 2));
            if (!name.empty())
                profiles.push_back(name);
        }
        pos = colonPos + 1;
    }
    return profiles;
}

std::string getPassword(const std::string &profileName)
{
    std::string cmd = "C:\\Windows\\System32\\netsh.exe wlan show profile name=\"" + profileName + "\" key=clear";
    std::string output = execCommand(cmd);
    size_t keyPos = output.find("Key Content");
    if (keyPos == std::string::npos)
        keyPos = output.find("Contenido de la clave");
    if (keyPos != std::string::npos)
    {
        size_t colonPos = output.find(':', keyPos);
        if (colonPos != std::string::npos)
        {
            size_t endLine = output.find('\n', colonPos);
            std::string pass = trimString(output.substr(colonPos + 2,
                                                        (endLine == std::string::npos) ? std::string::npos : endLine - colonPos - 2));
            if (pass == "Ausente" || pass == "Absent" || pass.empty())
                return "(Sin contraseña)";
            return pass;
        }
    }
    return "(Sin contraseña)";
}

std::vector<std::string> scanNetworks()
{
    std::vector<std::string> result;

    execCommand("C:\\Windows\\System32\\netsh.exe wlan scan");
    Sleep(6000);

    std::string output = execCommand("C:\\Windows\\System32\\netsh.exe wlan show networks mode=bssid");

    if (output.find("permiso de ubicaci") != std::string::npos ||
        output.find("Acceso denegado") != std::string::npos ||
        output.find("location permission") != std::string::npos ||
        output.find("privacy-location") != std::string::npos)
    {
        std::cout << "\n"
                  << C_YELLOW
                  << "[!] ATENCION: El servicio de Ubicacion de Windows esta desactivado.\n"
                  << "    Windows bloquea el escaneo de redes WiFi por privacidad.\n"
                  << "    Activalo en: Configuracion > Privacidad > Ubicacion\n"
                  << C_RESET;
        return result;
    }

    std::vector<std::string> lines;
    size_t start = 0, end;
    while ((end = output.find('\n', start)) != std::string::npos)
    {
        lines.push_back(output.substr(start, end - start));
        start = end + 1;
    }
    if (start < output.length())
        lines.push_back(output.substr(start));

    std::string currentSSID = "";
    std::string currentBSSID = "";
    int currentSignal = -1;
    std::vector<std::string> entries;

    for (size_t i = 0; i < lines.size(); i++)
    {
        std::string line = trimString(lines[i]);
        if (line.empty())
            continue;

        if (line.length() >= 5 && line.compare(0, 5, "SSID ") == 0)
        {
            size_t colonPos = line.find(':');
            if (colonPos != std::string::npos)
                currentSSID = trimString(line.substr(colonPos + 1));
        }
        else if (line.length() >= 6 && line.compare(0, 6, "BSSID ") == 0)
        {
            if (!currentBSSID.empty() && currentSignal >= 0)
            {
                std::string name = currentSSID.empty() ? "(Red oculta)" : currentSSID;
                entries.push_back(name + " (Señal: " + intToString(currentSignal) + "%) [" + currentBSSID + "]");
            }
            size_t colonPos = line.find(':');
            if (colonPos != std::string::npos)
                currentBSSID = trimString(line.substr(colonPos + 1));
            currentSignal = -1;
        }
        else if (line.find("Signal") == 0 || line.find("Señal") == 0)
        {
            size_t colonPos = line.find(':');
            if (colonPos != std::string::npos)
            {
                std::string sigStr = trimString(line.substr(colonPos + 1));
                size_t pctPos = sigStr.find('%');
                if (pctPos != std::string::npos)
                    currentSignal = atoi(sigStr.substr(0, pctPos).c_str());
            }
        }
    }
    if (!currentBSSID.empty() && currentSignal >= 0)
    {
        std::string name = currentSSID.empty() ? "(Red oculta)" : currentSSID;
        entries.push_back(name + " (Señal: " + intToString(currentSignal) + "%) [" + currentBSSID + "]");
    }

    std::sort(entries.begin(), entries.end());
    return entries;
}