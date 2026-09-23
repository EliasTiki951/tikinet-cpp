/*
 * ============================================================
 *   Tikinet Wifi C++ - Implementación para Linux
 * ============================================================
 * Usa NetworkManager (nmcli) y lee perfiles de
 * /etc/NetworkManager/system-connections/
 * Autor: EliasTiki
 * Licencia: MIT
 * ============================================================
 */

#include "tikinet_wifi.h"
#include <unistd.h>
#include <iostream>
#include <algorithm>

bool isAdmin() { return (geteuid() == 0); }

void requestAdminIfNeeded()
{
    if (!isAdmin())
    {
        std::cout << "Advertencia: No tienes permisos de root.\n";
        std::cout << "Se necesita sudo para ver contraseñas.\n";
        std::cout << "Intenta: sudo ./bin/tikinet-wifi\n\n";
    }
}

bool copyToClipboard(const std::string &text)
{
    FILE *pipe = popen("xclip -selection clipboard 2>/dev/null", "w");
    if (pipe)
    {
        fwrite(text.c_str(), 1, text.length(), pipe);
        if (pclose(pipe) == 0)
            return true;
    }
    pipe = popen("xsel --clipboard --input 2>/dev/null", "w");
    if (pipe)
    {
        fwrite(text.c_str(), 1, text.length(), pipe);
        if (pclose(pipe) == 0)
            return true;
    }
    return false;
}

std::vector<std::string> getConnectedDevices()
{
    std::vector<std::string> devices;
    std::string output = execCommand("ip neigh show 2>/dev/null || arp -a 2>/dev/null");
    size_t start = 0, end;
    while ((end = output.find('\n', start)) != std::string::npos)
    {
        std::string line = trimString(output.substr(start, end - start));
        start = end + 1;
        if (line.empty())
            continue;

        size_t devPos = line.find(" dev ");
        size_t llPos = line.find("lladdr ");
        if (devPos != std::string::npos && llPos != std::string::npos)
        {
            std::string ip = line.substr(0, devPos);
            std::string mac = line.substr(llPos + 7);
            size_t space = mac.find(' ');
            if (space != std::string::npos)
                mac = mac.substr(0, space);
            devices.push_back(ip + "  ->  " + mac);
        }
    }
    return devices;
}

std::vector<std::string> getSavedProfiles()
{
    std::vector<std::string> profiles;
    std::string output = execCommand("ls /etc/NetworkManager/system-connections/ 2>/dev/null");
    size_t start = 0, end;
    while ((end = output.find('\n', start)) != std::string::npos)
    {
        std::string file = output.substr(start, end - start);
        start = end + 1;
        if (file.find(".nmconnection") != std::string::npos)
        {
            size_t dotPos = file.find('.');
            if (dotPos != std::string::npos)
                profiles.push_back(file.substr(0, dotPos));
        }
    }
    return profiles;
}

std::string getPassword(const std::string &profileName)
{
    std::string cmd = "cat /etc/NetworkManager/system-connections/\"" + profileName + "\".nmconnection | grep -i psk=";
    std::string output = execCommand(cmd);
    size_t equalsPos = output.find('=');
    if (equalsPos != std::string::npos)
        return trimString(output.substr(equalsPos + 1));
    return "(Red abierta o desconocida)";
}

std::vector<std::string> scanNetworks()
{
    std::vector<std::string> networks;
    std::string output = execCommand("nmcli -t -f SSID,SIGNAL dev wifi list");
    size_t start = 0, end;
    while ((end = output.find('\n', start)) != std::string::npos)
    {
        std::string line = output.substr(start, end - start);
        start = end + 1;
        if (!line.empty() && line.find("SSID") != 0)
        {
            size_t colonPos = line.find(':');
            if (colonPos != std::string::npos)
            {
                std::string ssid = line.substr(0, colonPos);
                std::string signal = line.substr(colonPos + 1);
                if (!ssid.empty() && ssid != "--")
                    networks.push_back(ssid + " (Señal: " + signal + "%)");
            }
        }
    }
    std::sort(networks.begin(), networks.end());
    return networks;
}