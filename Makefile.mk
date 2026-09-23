# Tikinet Wifi C++ - Makefile (Versión 4.0)

CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra -O2
SRC_DIR = src
BIN_DIR = bin

ifeq ($(OS),Windows_NT)
    TARGET = $(BIN_DIR)/tikinet-wifi.exe
    SOURCES = $(SRC_DIR)/main.cpp $(SRC_DIR)/wifi_windows.cpp $(SRC_DIR)/QrCode.cpp
    LDFLAGS = -lshell32 -liphlpapi
    RUN_CMD = $(TARGET)
else
    TARGET = $(BIN_DIR)/tikinet-wifi
    SOURCES = $(SRC_DIR)/main.cpp $(SRC_DIR)/wifi_linux.cpp $(SRC_DIR)/QrCode.cpp
    LDFLAGS =
    RUN_CMD = ./$(TARGET)
endif

HEADERS = $(SRC_DIR)/tikinet_wifi.h $(SRC_DIR)/QrCode.hpp

GREEN = \033[0;32m
YELLOW = \033[0;33m
NC = \033[0m

all: $(TARGET)

$(TARGET): $(SOURCES) $(HEADERS)
	@echo "$(GREEN)Compilando Tikinet Wifi C++...$(NC)"
	@mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $(TARGET) $(SOURCES)
	@echo "$(GREEN)[OK] Compilado: $(TARGET)$(NC)"

debug:
	@mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) -g $(LDFLAGS) -o $(TARGET) $(SOURCES)

clean:
	@rm -rf $(BIN_DIR)
	@echo "$(GREEN)[OK] Limpieza completada$(NC)"

run: $(TARGET)
	@$(RUN_CMD)

help:
	@echo "Tikinet Wifi C++ - Makefile"
	@echo "  make         - Compilar"
	@echo "  make debug   - Compilar con símbolos"
	@echo "  make clean   - Limpiar"
	@echo "  make run     - Compilar y ejecutar"

.PHONY: all debug clean run help