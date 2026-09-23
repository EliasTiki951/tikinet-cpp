/*
 * ============================================================
 *   Tikinet Wifi C++ - Cabecera Principal
 * ============================================================
 * Autor: EliasTiki
 * Licencia: MIT
 * Versión: 4.0
 * Fecha: 2026
 * Repositorio: https://github.com/EliasTiki951/tikinet-cpp
 * ============================================================
 */

#ifndef TIKINET_WIFI_H
#define TIKINET_WIFI_H

#include <string>
#include <vector>

#define TIKINET_NAME "Tikinet Wifi C++"
#define TIKINET_VERSION "4.0"

// COLORES ANSI
#define C_RESET "\033[0m"
#define C_RED "\033[31m"
#define C_GREEN "\033[32m"
#define C_YELLOW "\033[33m"
#define C_CYAN "\033[36m"
#define C_BOLD "\033[1m"
#define C_GRAY "\033[90m"

// UTILIDADES (main.cpp)
std::string intToString(int num);
void clearScreen();
std::string getCurrentDateTime();
bool saveToFile(const std::string &filename, const std::string &content);
bool saveToCSV(const std::string &filename, const std::vector<std::string> &lines);
void waitForEnter();
std::string execCommand(const std::string &cmd);
std::string trimString(std::string str);
void enableColors();
void showProgressBar(int segundos, const std::string &mensaje);
void say(const std::string &s); // <-- NUEVO: imprime UTF-8 correctamente

// INTERFAZ POR SO
bool isAdmin();
void requestAdminIfNeeded();
std::vector<std::string> getSavedProfiles();
std::string getPassword(const std::string &profileName);
std::vector<std::string> scanNetworks();
std::vector<std::string> getConnectedDevices();
bool copyToClipboard(const std::string &text);

// FUNCIONES DE MENÚ
void viewAllNetworks();
void scanNearbyNetworks();
void showConnectedDevices();
void generateQRMenu();
void showHelp();

#endif