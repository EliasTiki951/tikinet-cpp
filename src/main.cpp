/*
 * -------------------------------------
 *   Tikinet Wifi C++ V.4.0 (Multi-OS)
 * -------------------------------------
 * Utilidad de consola para Windows y Linux que permite consultar las contraseñas
 * de las redes WiFi que el propio equipo ya tiene guardadas
 * Pensada para casos como: "me olvidé la contraseña de mi propio router"
 * o "quiero compartirla con una visita".
 *
 * Requisitos:
 *   - Administrador (Windows) / Root (Linux)
 *   - Servicio de Ubicación activo en Windows (para escanear redes)
 *   - NetworkManager (nmcli) en Linux
 *   - Compilador C++11 o superior
 *
 * Características:
 *   - Ver todas las redes WiFi guardadas con sus contraseñas
 *   - Escanear redes WiFi cercanas con intensidad de señal y BSSID
 *   - Modo de escaneo continuo (monitoreo en vivo de cambios)
 *   - Ver dispositivos conectados a la red (dispositivos, broadcast y multicast)
 *   - Generar códigos QR (SVG) para compartir redes fácilmente
 *   - Copiar contraseñas al portapapeles
 *   - Exportar resultados a TXT y CSV
 *   - Interfaz de menú con colores y barra de progreso
 *   - Soporte multi-idioma (español / inglés) en Windows y Linux
 *
 * Compilación:
 *   - Windows: g++ -std=c++11 src/main.cpp src/wifi_windows.cpp src/QrCode.cpp ^
 *                    -o bin/tikinet-wifi.exe -lshell32 -liphlpapi
 *   - Linux:   g++ -std=c++11 src/main.cpp src/wifi_linux.cpp src/QrCode.cpp ^
 *                    -o bin/tikinet-wifi
 *   - Con Makefile:  make
 *
 * Ejecución:
 *   - Windows: bin\tikinet-wifi.exe  (se auto-eleva a Administrador)
 *   - Linux:   sudo ./bin/tikinet-wifi
 *
 * Versiones Anteriores:
 *   - Wifi Scanner C++:
 *   - V.0.5: (Beta): Solo mostraba las redes guardadas y sus contraseñas en Windows.
 *   - V.1.1: Opción para guardar los resultados en un archivo .txt y arreglos menores.
 *   - V.2.0: Escaneo de redes cercanas en Windows. Pequeñas mejoras en la interfaz.
 *   - V.2.1: Pequeñas mejoras con el escaneo de redes cercanas en Windows
 *            y con muestra de intensidad de señal.
 *   - V.3.0: Expandimos la compatibilidad a Linux (NetworkManager).
 *
 *   - Tikinet Wifi C++:
 *   - V.4.0: Interfaz con colores, barra de progreso, exportación a CSV, copia al
 *            portapapeles, escaneo continuo, detección de dispositivos conectados
 *            y generación de códigos QR. Renombrado a "Tikinet Wifi C++".
 *   - V.4.5: Futuras mejoras proximo.
 *
 * REPOSITORIO: https://github.com/EliasTiki951/tikinet-cpp
 *
 * Autor: EliasTiki
 * Licencia: MIT
 * Versión: 4.0
 * Fecha: 2026
 * Compilador: C++11 o superior
 * Sistema: Windows (7, 8, 10, 11) y Linux
 */

#include "tikinet_wifi.h"
#include "QrCode.hpp"

#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <clocale>
#include <map>
#include <stdexcept>

#ifdef _WIN32
#include <windows.h>
#include <conio.h> // Para _kbhit() y _getch()
#define POPEN _popen
#define PCLOSE _pclose
#else
#include <unistd.h>
#define POPEN popen
#define PCLOSE pclose
#endif

using qrcodegen::QrCode;

// UTILIDAD: IMPRIMIR UTF-8 CORRECTAMENTE
void say(const std::string &s)
{
#ifdef _WIN32
    if (s.empty())
        return;
    int wlen = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), (int)s.size(), NULL, 0);
    if (wlen > 0)
    {
        std::vector<wchar_t> wbuf(wlen);
        MultiByteToWideChar(CP_UTF8, 0, s.c_str(), (int)s.size(), &wbuf[0], wlen);
        std::wcout << std::wstring(&wbuf[0], wlen);
        std::wcout.flush();
    }
    else
    {
        std::cout << s;
        std::cout.flush();
    }
#else
    std::cout << s;
    std::cout.flush();
#endif
}

// UTILIDADES COMUNES
void enableColors()
{
#ifdef _WIN32
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut != INVALID_HANDLE_VALUE)
    {
        DWORD dwMode = 0;
        if (GetConsoleMode(hOut, &dwMode))
        {
            dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
            SetConsoleMode(hOut, dwMode);
        }
    }
#endif
}

std::string intToString(int num)
{
    char buffer[50];
    sprintf(buffer, "%d", num);
    return std::string(buffer);
}

void clearScreen()
{
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

std::string getCurrentDateTime()
{
    time_t now = time(0);
    struct tm *timeinfo = localtime(&now);
    char timeStr[100];
    strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", timeinfo);
    return std::string(timeStr);
}

bool saveToFile(const std::string &filename, const std::string &content)
{
    std::ofstream outputFile(filename.c_str());
    if (!outputFile.is_open())
    {
        say(std::string(C_RED) + "ERROR: No se pudo crear " + filename + C_RESET + "\n");
        return false;
    }
    outputFile << content;
    outputFile.close();
    return true;
}

bool saveToCSV(const std::string &filename, const std::vector<std::string> &lines)
{
    std::ofstream f(filename.c_str());
    if (!f.is_open())
    {
        say(std::string(C_RED) + "ERROR: No se pudo crear " + filename + C_RESET + "\n");
        return false;
    }
    for (size_t i = 0; i < lines.size(); i++)
        f << lines[i] << "\n";
    f.close();
    return true;
}

void waitForEnter()
{
    say(std::string(C_GRAY) + "\nPresiona Enter para continuar..." + C_RESET);
    std::cin.ignore();
    std::cin.get();
}

std::string execCommand(const std::string &cmd)
{
    char buffer[4096];
    std::string result = "";
    FILE *pipe = POPEN(cmd.c_str(), "r");
    if (!pipe)
        return "";
    while (fgets(buffer, sizeof(buffer), pipe) != NULL)
        result += buffer;
    PCLOSE(pipe);
    return result;
}

std::string trimString(std::string str)
{
    while (str.size() > 0 && (str[str.size() - 1] == ' ' || str[str.size() - 1] == '\r' || str[str.size() - 1] == '\n'))
        str.erase(str.size() - 1, 1);
    while (str.size() > 0 && str[0] == ' ')
        str.erase(0, 1);
    return str;
}

void showProgressBar(int segundos, const std::string &mensaje)
{
    const int total = 20;
    say(mensaje + "\n");
    for (int s = 0; s <= segundos; s++)
    {
        int filled = (s * total) / segundos;
        std::string bar = "\r  [";
        for (int i = 0; i < total; i++)
        {
            if (i < filled)
                bar += "#";
            else
                bar += "-";
        }
        bar += "] " + intToString(s * 100 / segundos) + "%";
        say(bar);
#ifdef _WIN32
        Sleep(1000);
#else
        sleep(1);
#endif
    }
    say("  [LISTO]\n\n");
}

// SANITIZAR NOMBRE DE ARCHIVO
static std::string sanitizeFilename(const std::string &name)
{
    std::string result = name;
    for (size_t i = 0; i < result.length(); i++)
    {
        char c = result[i];
        if (c == '/' || c == '\\' || c == ':' || c == '*' || c == '?' ||
            c == '"' || c == '<' || c == '>' || c == '|' || c == ' ' || c == '\t')
        {
            result[i] = '_';
        }
    }
    return result;
}

// ESCAPAR CAMPOS WIFI (estándar QR)
static std::string escapeWifiField(const std::string &s)
{
    std::string result;
    for (size_t i = 0; i < s.length(); i++)
    {
        char c = s[i];
        if (c == '\\' || c == ';' || c == ',' || c == ':' || c == '"')
        {
            result += '\\';
        }
        result += c;
    }
    return result;
}

// IMPRIMIR QR EN CONSOLA
static void printQrToConsole(const QrCode &qr, int border)
{
    for (int y = -border; y < qr.getSize() + border; y += 2)
    {
        for (int x = -border; x < qr.getSize() + border; x++)
        {
            bool top = qr.getModule(x, y);
            bool bottom = (y + 1 < qr.getSize() + border) ? qr.getModule(x, y + 1) : false;
            if (top && bottom)
                std::cout << "\xe2\x96\x88";
            else if (top && !bottom)
                std::cout << "\xe2\x96\x80";
            else if (!top && bottom)
                std::cout << "\xe2\x96\x84";
            else
                std::cout << " ";
        }
        std::cout << "\n";
    }
    std::cout << "\n";
}

// MENÚ 1: VER REDES GUARDADAS
void viewAllNetworks()
{
    clearScreen();
    say(std::string(C_BOLD) + C_CYAN + "=== REDES WIFI GUARDADAS ===\n\n" + C_RESET);

    std::vector<std::string> profiles = getSavedProfiles();

    if (profiles.empty())
    {
        say(std::string(C_YELLOW) + "No se encontraron perfiles WiFi guardados.\n" + C_RESET);
        waitForEnter();
        return;
    }

    say("Total: " + intToString((int)profiles.size()) + "\n\n");

    std::string content = "=== TIKINET WIFI - CONTRASENAS ===\n";
    content += "Fecha: " + getCurrentDateTime() + "\n\n";

    std::vector<std::string> csvLines;
    csvLines.push_back("SSID,Password,Fecha");

    for (size_t i = 0; i < profiles.size(); i++)
    {
        std::string pass = getPassword(profiles[i]);
        say("[" + intToString((int)(i + 1)) + "] " + profiles[i] + "\n");
        say("    Clave: " + pass + "\n");
        say("    ---\n");

        std::string plain = "[" + intToString((int)(i + 1)) + "] " + profiles[i] + "\n";
        plain += "    Clave: " + pass + "\n    ---\n";
        content += plain;

        std::string escapedPass = pass;
        size_t p = 0;
        while ((p = escapedPass.find('"', p)) != std::string::npos)
        {
            escapedPass.replace(p, 1, "\"\"");
            p += 2;
        }
        csvLines.push_back("\"" + profiles[i] + "\",\"" + escapedPass + "\",\"" + getCurrentDateTime() + "\"");
    }

    say(std::string("\n") + C_BOLD + "Exportar resultados?\n" + C_RESET);
    say("  1. TXT (texto plano)\n");
    say("  2. CSV (para Excel)\n");
    say("  0. No guardar\n");
    say("Opcion: ");

    int fmt = 0;
    std::cin >> fmt;
    std::cin.ignore();

    if (fmt == 1 || fmt == 2)
    {
        say("Nombre base (default: tikinet_passwords): ");
        std::string filename;
        std::getline(std::cin, filename);
        if (filename.empty())
            filename = "tikinet_passwords";
        size_t dot = filename.find('.');
        if (dot != std::string::npos)
            filename = filename.substr(0, dot);

        if (fmt == 1)
        {
            std::string f = filename + ".txt";
            if (saveToFile(f, content))
                say(std::string(C_GREEN) + "[OK] Guardado: " + f + C_RESET + "\n");
        }
        else
        {
            std::string f = filename + ".csv";
            if (saveToCSV(f, csvLines))
                say(std::string(C_GREEN) + "[OK] Guardado: " + f + C_RESET + "\n");
        }
    }

    say("\nCopiar alguna contrasena al portapapeles? (numero o 0): ");
    int idx = 0;
    std::cin >> idx;
    std::cin.ignore();
    if (idx > 0 && idx <= (int)profiles.size())
    {
        std::string pass = getPassword(profiles[idx - 1]);
        if (copyToClipboard(pass))
            say(std::string(C_GREEN) + "[OK] Contrasena copiada.\n" + C_RESET);
        else
            say(std::string(C_RED) + "[!] No se pudo copiar.\n" + C_RESET);
    }

    waitForEnter();
}

// MENÚ 2: ESCANEAR REDES CERCANAS
void scanNearbyNetworks()
{
    clearScreen();
    say(std::string(C_BOLD) + C_CYAN + "=== ESCANEO DE REDES WIFI ===\n\n" + C_RESET);

    std::vector<std::string> networks = scanNetworks();

    std::string content = "=== TIKINET WIFI - ESCANEO ===\n";
    content += "Fecha: " + getCurrentDateTime() + "\n\n";

    if (networks.empty())
    {
        say(std::string(C_RED) + "No se encontraron redes WiFi.\n" + C_RESET);
        say(std::string(C_YELLOW) + "\nCausas posibles: WiFi desactivado, ubicacion apagada o sin redes.\n" + C_RESET);
    }
    else
    {
        std::sort(networks.begin(), networks.end());
        networks.erase(std::unique(networks.begin(), networks.end()), networks.end());

        say(std::string(C_GREEN) + "Redes encontradas: " + intToString((int)networks.size()) + C_RESET + "\n\n");
        for (size_t i = 0; i < networks.size(); i++)
        {
            say("[" + intToString((int)(i + 1)) + "] " + networks[i] + "\n");
            content += "[" + intToString((int)(i + 1)) + "] " + networks[i] + "\n";
        }
        content += "\nTotal: " + intToString((int)networks.size()) + "\n";
    }

    say("\nGuardar resultados en .txt? (s/n): ");
    char option;
    std::cin >> option;
    std::cin.ignore();

    if (option == 's' || option == 'S')
    {
        say("Nombre (default: tikinet_scan.txt): ");
        std::string filename;
        std::getline(std::cin, filename);
        if (filename.empty())
            filename = "tikinet_scan.txt";
        if (filename.find(".txt") == std::string::npos)
            filename += ".txt";
        if (saveToFile(filename, content))
            say(std::string(C_GREEN) + "[OK] Guardado: " + filename + C_RESET + "\n");
    }

    say("\nHacer escaneo continuo (monitoreo)? (s/n): ");
    char cont;
    std::cin >> cont;
    std::cin.ignore();

    if (cont == 's' || cont == 'S')
    {
        say("Intervalo entre escaneos en segundos (default 15): ");
        std::string intervalStr;
        std::getline(std::cin, intervalStr);
        int interval = 15;
        if (!intervalStr.empty())
            interval = atoi(intervalStr.c_str());
        if (interval < 5)
            interval = 5;

        std::map<std::string, int> previousSignals;
        int iteration = 0;

        // AVISO IMPORTANTE
        clearScreen();
        say(std::string(C_BOLD) + "=== MODO MONITOREO ACTIVADO ===\n\n" + C_RESET);
        say("Presiona 'Q' o 'ESC' en cualquier momento para volver al menu.\n\n");
        say("Iniciando en 3 segundos...");
#ifdef _WIN32
        Sleep(3000);
#else
        sleep(3);
#endif

        while (true)
        {
            iteration++;
            clearScreen();
            say(std::string(C_BOLD) + C_CYAN + "=== MONITOREO EN VIVO - Iteracion #" + intToString(iteration) + " ===\n" + C_RESET);
            say("Hora: " + getCurrentDateTime() + "\n");
            say(std::string(C_GRAY) + "(Presiona Q o ESC para volver)\n\n" + C_RESET);

            std::vector<std::string> nets = scanNetworks();
            if (nets.empty())
            {
                say(std::string(C_RED) + "No se detectaron redes.\n" + C_RESET);
            }
            else
            {
                std::sort(nets.begin(), nets.end());
                for (size_t i = 0; i < nets.size(); i++)
                {
                    std::string net = nets[i];
                    int sig = 0;
                    size_t pct = net.find('%');
                    if (pct != std::string::npos)
                    {
                        size_t start = net.rfind(':', pct);
                        if (start != std::string::npos)
                        {
                            std::string s = trimString(net.substr(start + 1, pct - start - 1));
                            sig = atoi(s.c_str());
                        }
                    }
                    std::string marker = "  ";
                    std::map<std::string, int>::iterator it = previousSignals.find(net);
                    if (it == previousSignals.end())
                    {
                        marker = std::string(C_GREEN) + "[NUEVA]" + C_RESET;
                    }
                    else if (abs(it->second - sig) >= 10)
                    {
                        marker = std::string(C_YELLOW) + "[CAMBIO]" + C_RESET;
                    }
                    previousSignals[net] = sig;
                    say(marker + " " + net + "\n");
                }
            }
            say("\n" + std::string(C_GRAY) + "Proximo escaneo en " + intToString(interval) + "s...\n" + C_RESET);

            // Espera con detección de tecla
            bool salir = false;
            for (int i = 0; i < interval * 10; i++)
            {
#ifdef _WIN32
                if (_kbhit())
                {
                    int ch = _getch();
                    if (ch == 27 || ch == 'q' || ch == 'Q')
                    {
                        salir = true;
                        break;
                    }
                }
                Sleep(100);
#else
                usleep(100000);
#endif
            }
            if (salir)
            {
                clearScreen();
                say(std::string(C_GREEN) + "Monitoreo detenido.\n" + C_RESET);
                waitForEnter();
                return;
            }
        }
    }

    waitForEnter();
}

// MENÚ 3: DISPOSITIVOS CONECTADOS
void showConnectedDevices()
{
    clearScreen();
    say("=== DISPOSITIVOS CONECTADOS DE TU RED ===\n\n");
    say("Analizando tabla ARP...\n\n");

    std::vector<std::string> lines = getConnectedDevices();

    if (lines.empty())
    {
        say("No se detectaron dispositivos.\n");
        say("Verifica que estes conectado a una red WiFi.\n");
    }
    else
    {
        for (size_t i = 0; i < lines.size(); i++)
        {
            say(lines[i] + "\n");
        }
    }
    waitForEnter();
}

// MENÚ 4: GENERAR QR
void generateQRMenu()
{
    clearScreen();
    say(std::string(C_BOLD) + C_CYAN + "=== GENERAR QR PARA COMPARTIR RED ===\n\n" + C_RESET);

    std::vector<std::string> profiles = getSavedProfiles();
    if (profiles.empty())
    {
        say(std::string(C_YELLOW) + "No hay redes guardadas.\n" + C_RESET);
        waitForEnter();
        return;
    }

    say("Selecciona la red guardada:\n\n");
    for (size_t i = 0; i < profiles.size(); i++)
    {
        say("  [" + intToString((int)(i + 1)) + "] " + profiles[i] + "\n");
    }

    say("\nNumero de red (0 para cancelar): ");
    int idx = 0;
    std::cin >> idx;
    std::cin.ignore();

    if (idx <= 0 || idx > (int)profiles.size())
        return;

    std::string ssid = profiles[idx - 1];
    std::string pass = getPassword(ssid);

    std::string qrString;
    if (pass == "(Sin contraseña)" || pass == "(Sin contraseña)" ||
        pass.find("Sin contrase") != std::string::npos ||
        pass.find("abierta") != std::string::npos)
    {
        qrString = "WIFI:T:nopass;S:" + escapeWifiField(ssid) + ";;";
    }
    else
    {
        qrString = "WIFI:T:WPA;S:" + escapeWifiField(ssid) + ";P:" + escapeWifiField(pass) + ";;";
    }

    say("\nGenerando QR...\n\n");

    try
    {
        const QrCode qr = QrCode::encodeText(qrString.c_str(), QrCode::Ecc::MEDIUM);

        say(std::string(C_BOLD) + "Escanea este QR con tu celular:\n\n" + C_RESET);
        printQrToConsole(qr, 2);

        std::string filename = "QR_" + sanitizeFilename(ssid) + ".svg";
        std::string svg = qr.toSvgString(4);

        if (saveToFile(filename, svg))
        {
            say(std::string(C_GREEN) + "[OK] Guardado: " + filename + C_RESET + "\n");
            say(std::string(C_GRAY) + "Abrelo con Chrome/Firefox/Edge o subelo a Google Drive.\n" + C_RESET);
        }
        else
        {
            say(std::string(C_RED) + "[!] No se pudo guardar el archivo.\n" + C_RESET);
        }
    }
    catch (const std::exception &e)
    {
        say(std::string(C_RED) + "[!] Error generando QR: " + e.what() + "\n" + C_RESET);
    }

    waitForEnter();
}

// MENÚ 5: AYUDA
void showHelp()
{
    clearScreen();
    say(std::string(C_BOLD) + C_CYAN + "=== AYUDA - TIKINET WIFI C++ V.4.0 ===\n\n" + C_RESET);

    say(std::string(C_BOLD) + "Requisitos:\n" + C_RESET);
    say("  - Windows: Ejecutar como Administrador y tener activo\n");
    say("    el servicio de Ubicacion (Configuracion > Privacidad)\n");
    say("  - Linux: Ejecutar con sudo y tener NetworkManager (nmcli)\n\n");

    say(std::string(C_BOLD) + "Funciones:\n" + C_RESET);
    say("  1. Redes guardadas: muestra SSID + contrasena. Exporta a TXT/CSV.\n");
    say("  2. Escanear redes: redes cercanas con senal y MAC. Modo monitoreo.\n");
    say("  3. Dispositivos: IPs y MACs de tu red local (tabla ARP).\n");
    say("  4. Generar QR: QR para compartir red. Se guarda como .svg.\n\n");

    say(std::string(C_BOLD) + "Uso responsable:\n" + C_RESET);
    say("  Esta herramienta es para recuperar TUS PROPIAS contrasenas.\n");
    say("  No la uses en equipos ajenos sin autorizacion.\n\n");

    waitForEnter();
}

// FUNCIÓN PRINCIPAL
int main()
{
    enableColors();

#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif
    setlocale(LC_ALL, "");

    requestAdminIfNeeded();

    int option = -1;
    while (option != 0)
    {
        clearScreen();
        say(std::string(C_BOLD) + C_CYAN + "====================================\n" + "  " + TIKINET_NAME + " V." + TIKINET_VERSION + "\n" + "====================================\n" + C_RESET);
        say(std::string("\n") + C_BOLD + "Que quieres hacer?\n" + C_RESET);
        say("  1. Ver redes guardadas y contrasenas\n");
        say("  2. Escanear redes cercanas\n");
        say("  3. Ver dispositivos conectados de tu red\n");
        say("  4. Generar QR para compartir red\n");
        say("  5. Ayuda\n");
        say("  0. Salir\n");
        say(std::string("\n") + C_BOLD + "Opcion: " + C_RESET);

        std::cin >> option;
        std::cin.ignore();

        switch (option)
        {
        case 1:
            viewAllNetworks();
            break;
        case 2:
            scanNearbyNetworks();
            break;
        case 3:
            showConnectedDevices();
            break;
        case 4:
            generateQRMenu();
            break;
        case 5:
            showHelp();
            break;
        case 0:
            say(std::string("\n") + C_GREEN + "Hasta luego! Gracias por usar " + TIKINET_NAME + C_RESET + "\n");
            break;
        default:
            say(std::string("\n") + C_RED + "Opcion no valida.\n" + C_RESET);
            waitForEnter();
            break;
        }
    }
    return 0;
}