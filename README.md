<div align="center">

# 📡 Tikinet Wifi C++

![Windows](https://img.shields.io/badge/Windows-0078D6?style=for-the-badge&logo=windows&logoColor=white)
![Linux](https://img.shields.io/badge/Linux-FCC624?style=for-the-badge&logo=linux&logoColor=black)
![C++](https://img.shields.io/badge/C++-00599C?style=for-the-badge&logo=c%2B%2B&logoColor=white)
![License](https://img.shields.io/badge/License-MIT-green?style=for-the-badge)
![Version](https://img.shields.io/badge/Version-4.0-blue?style=for-the-badge)
![Multi-OS](https://img.shields.io/badge/Multi--OS-Windows%20%7C%20Linux-purple?style=for-the-badge)

**Utilidad de consola multi-OS que permite recuperar las contraseñas de las redes WiFi guardadas, escanear redes cercanas, ver los dispositivos conectados a tu red y generar códigos QR para compartir tus credenciales fácilmente.**

</div>

---

## 📋 Descripción

¿Alguna vez olvidaste la contraseña de tu propia red WiFi? ¿Necesitás compartirla con una visita y no la recordás? ¿Querés saber quién está conectado a tu red?

**Tikinet Wifi C++** es una herramienta de consola, rápida y liviana, que te permite:

- Visualizar todas las contraseñas WiFi guardadas en tu equipo.
- Escanear redes cercanas con su intensidad de señal y BSSID.
- Monitorear en vivo los cambios en las redes disponibles.
- Listar los dispositivos conectados a tu red local (con clasificación de dispositivos reales, broadcast y multicast).
- Generar códigos QR para compartir una red sin tener que escribir la contraseña.

El programa utiliza los comandos nativos de cada sistema operativo, por lo que no requiere instalar dependencias externas (a diferencia de herramientas similares).

- **Windows:** `netsh wlan show profile key=clear` + API nativa para adaptadores de red.
- **Linux:** `nmcli` y archivos de NetworkManager.

---

## ✨ Características principales

|                                |                                                                                    |
| ------------------------------ | ---------------------------------------------------------------------------------- |
| 🔑 **Ver redes guardadas**     | Muestra todas las redes WiFi guardadas con sus contraseñas                         |
| 📡 **Escanear redes cercanas** | Detecta redes disponibles con intensidad de señal, BSSID y MAC del punto de acceso |
| 🔄 **Escaneo continuo**        | Modo de monitoreo en vivo que detecta cambios y nuevas redes cada X segundos       |
| 🌐 **Dispositivos conectados** | Lista IPs y MACs de tu red, diferenciando dispositivos, broadcast y multicast      |
| 📱 **Generar códigos QR**      | Crea un QR (SVG) para compartir una red con cualquier dispositivo móvil            |
| 📋 **Copiar al portapapeles**  | Copia cualquier contraseña directamente con un solo número                         |
| 💾 **Exportar a TXT y CSV**    | Guarda los resultados en formato TXT o CSV (para Excel)                            |
| 🎨 **Interfaz con colores**    | Colores ANSI para mejor legibilidad (solo lo necesario)                            |
| 📊 **Barra de progreso**       | Feedback visual durante los escaneos                                               |
| 🌍 **Soporte multi-idioma**    | Compatible con sistemas en español e inglés                                        |
| 🖥️ **Multi-OS**                | Compatible con Windows (7, 8, 10, 11) y Linux                                      |
| 🔒 **Auto-elevación**          | En Windows solicita permisos de Administrador automáticamente                      |

---

## 📜 Historial de versiones

### 🔐 WiFi Scanner C++ (Predecesor)

| Versión   | Fecha | Cambios                                                                    |
| --------- | ----- | -------------------------------------------------------------------------- |
| **V.0.5** | 2026  | (Beta) Solo mostraba las redes guardadas y sus contraseñas en Windows      |
| **V.1.1** | 2026  | Opción para guardar los resultados en un archivo `.txt` y arreglos menores |
| **V.2.0** | 2026  | Escaneo de redes cercanas en Windows. Pequeñas mejoras en la interfaz      |
| **V.2.1** | 2026  | Mejoras con el escaneo de redes cercanas y muestra de intensidad de señal  |
| **V.3.0** | 2026  | Se expandió la compatibilidad a Linux (NetworkManager)                     |

### 📡 Tikinet Wifi C++ (Actual)

| Versión   | Fecha          | Cambios                                                                                                                                                                                                                                                       |
| --------- | -------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| **V.4.0** | 2026           | Interfaz con colores, barra de progreso, exportación a CSV, copiado al portapapeles, escaneo continuo, detección de dispositivos conectados (con clasificación de broadcast y multicast) y generación de códigos QR (SVG). Renombrado a **Tikinet Wifi C++**. |
| **V.4.5** | _Próximamente_ |                                                                                                                                                                                                                                                               |

---

## 🚀 Requisitos

### Windows

- **Sistema operativo:** Windows 7, 8, 10, 11
- **Permisos:** Ejecutar como **Administrador** (el programa se auto-eleva)
- **Servicio de Ubicación:** Debe estar **activado** para poder escanear redes WiFi
  - Ir a: `Configuración → Privacidad y seguridad → Ubicación`
  - Activar: _"Servicios de ubicación"_ y _"Permitir que las aplicaciones de escritorio accedan a tu ubicación"_
- **Compilador:** g++ (C++11 o superior) — se recomienda MinGW o MSYS2
- **Librerías:** `shell32` e `iphlpapi` (incluidas con MinGW)

### Linux

- **Sistema operativo:** Cualquier distribución (Ubuntu, Debian, Fedora, Arch, etc.)
- **Permisos:** Ejecutar con **sudo** (root) para leer los perfiles de NetworkManager
- **Dependencias:**
  - NetworkManager con `nmcli` instalado
  - `xclip` o `xsel` (para la función de copiado al portapapeles)
- **Compilador:** g++ (C++11 o superior)

---

## 📥 Instalación

### Opción 1: Descargar ejecutable (recomendado)

1. Ir a la sección [Releases](https://github.com/EliasTiki951/tikinet-cpp/releases)
2. Descargar el archivo correspondiente a tu sistema:
   - `tikinet-wifi-v4.exe` → Windows
   - `tikinet-wifi-v4` → Linux
3. Ejecutar según tu sistema:
   - **Windows:** Clic derecho → _"Ejecutar como administrador"_
   - **Linux:** `sudo ./tikinet-wifi-v4`

### Opción 2: Compilar desde código fuente

```bash
# Clonar el repositorio
git clone https://github.com/EliasTiki951/tikinet-cpp.git
cd tikinet-cpp

# Compilar con Makefile (recomendado)
make

# El ejecutable se generará en:
#   - Windows: bin/tikinet-wifi.exe
#   - Linux:   bin/tikinet-wifi
```

---

## ▶️ Uso

1. Ejecutar el programa con permisos de administrador (Windows) o `sudo` (Linux).
2. Elegir una opción del menú:

```
====================================
  Tikinet Wifi C++ V.4.0
====================================

¿Qué quieres hacer?
  1. Ver redes guardadas y contraseñas
  2. Escanear redes cercanas
  3. Ver dispositivos conectados de tu red
  4. Generar QR para compartir red
  5. Ayuda
  0. Salir

Opción:
```

### 🔑 Opción 1 — Redes guardadas

- Lista todas las redes WiFi guardadas con sus contraseñas.
- Permite exportar a **TXT** o **CSV** (Excel).
- Permite copiar una contraseña al portapapeles.

### 📡 Opción 2 — Escanear redes cercanas

- Muestra todas las redes disponibles con su porcentaje de señal.
- Muestra el **BSSID** (MAC del punto de acceso) para identificar cada antena.
- Al terminar, ofrece iniciar el **modo de escaneo continuo** (monitoreo en vivo cada X segundos).
- Durante el monitoreo, se puede salir con `Q` o `Esc`.

### 🌐 Opción 3 — Dispositivos conectados

- Lista los dispositivos conectados a tu red local.
- Clasifica automáticamente:
  - **Dispositivos reales** (con etiquetas `[ROUTER]`, `[ESTE EQUIPO]`, `[Dispositivo]`)
  - **Broadcast** (paquetes enviados a todos los dispositivos)
  - **Multicast** (direcciones de grupo: mDNS, SSDP, LLMNR, IGMPv3, etc.)

### 📱 Opción 4 — Generar QR

- Selecciona una red guardada y genera su código QR.
- El QR se muestra en la consola (ASCII) y se guarda como archivo `.svg`.
- El archivo se puede abrir con Chrome, Firefox o Edge, o subir a Google Drive.

### ❓ Opción 5 — Ayuda

- Muestra información detallada sobre las funciones y requisitos del sistema.

---

## 📂 Estructura del proyecto

```
tikinet-cpp/
├── src/
│   ├── main.cpp             # Programa principal y menú
│   ├── tikinet_wifi.h       # Cabecera común
│   ├── wifi_windows.cpp     # Implementación específica de Windows
│   ├── wifi_linux.cpp       # Implementación específica de Linux
│   ├── QrCode.cpp           # Librería de generación de QR
│   └── QrCode.hpp           # Cabecera de la librería QR
├── bin/                     # Ejecutables (ignorado por Git)
├── .gitignore               # Archivos ignorados por Git
├── LICENSE                  # Licencia MIT
├── Makefile                 # Compilación automática
└── README.md                # Este archivo
```

---

## 🛠️ Compilación

**Usando Makefile (recomendado)**

```bash
make           # Compila el programa
make debug     # Compila con símbolos de depuración
make clean     # Elimina el ejecutable y la carpeta bin/
make run       # Compila y ejecuta
make help      # Muestra todas las opciones disponibles
```

**Compilación manual (Windows)**

```cmd
g++ -std=c++11 src/main.cpp src/wifi_windows.cpp src/QrCode.cpp ^
    -o bin/tikinet-wifi.exe -lshell32 -liphlpapi
```

**Compilación manual (Linux)**

```bash
g++ -std=c++11 src/main.cpp src/wifi_linux.cpp src/QrCode.cpp -o bin/tikinet-wifi
```

---

## ⚠️ Advertencia de seguridad

> Este programa **solo funciona en tu propia PC** y muestra las contraseñas que vos mismo guardaste. **No es un "hackeador de WiFi"** ni puede acceder a redes ajenas. Es una herramienta de recuperación de contraseñas olvidadas y de diagnóstico de red local.

**Recomendaciones:**

- No compartas los archivos `tikinet_passwords.txt`, `tikinet_passwords.csv` ni los `.svg` generados con nadie.
- Ejecutá siempre como Administrador (Windows) o con `sudo` (Linux) para que funcione correctamente.
- Usá esta herramienta de forma ética y solo en tus propios equipos.
- Eliminá los archivos generados después de usarlos si contienen información sensible.

**Responsabilidad:**
El autor no se hace responsable del uso malintencionado que se le pueda dar a esta herramienta. El programa fue diseñado con fines educativos, de recuperación de contraseñas personales y de diagnóstico de red. El usuario es el único responsable del uso que le dé y de cumplir con las leyes aplicables en su país o región.

**Aviso legal:**
El acceso no autorizado a redes WiFi ajenas está penado por la ley en muchos países. Esta herramienta solo debe utilizarse en redes propias o con autorización expresa del propietario. El autor no promueve ni apoya el uso ilegal de este software.

---

## 🤝 Contribuciones

Las contribuciones son bienvenidas. Si querés mejorar el proyecto:

1. Hacé un **Fork** del repositorio.
2. Creá una rama con tu feature (`git checkout -b feature/mejora`).
3. Hacé commit de tus cambios (`git commit -m 'Agrega nueva funcionalidad'`).
4. Hacé push a la rama (`git push origin feature/mejora`).
5. Abrí un **Pull Request**.

---

## 📄 Licencia

Este proyecto está bajo la licencia MIT. Ver el archivo [LICENSE](LICENSE) para más detalles.

---

## 🙏 Créditos

- **Librería QR:** [QR Code generator library](https://www.nayuki.io/page/qr-code-generator-library) por **Project Nayuki** (Licencia MIT).
- **Framework de escaneo:** Comandos nativos de Windows (`netsh`) y Linux (`nmcli`).

---

<div align="center">

## 👨‍💻 Autor

**Elias Tiquicala** (EliasTiki951)

[![GitHub](https://img.shields.io/badge/GitHub-100000?style=for-the-badge&logo=github&logoColor=white)](https://github.com/EliasTiki951)
[![LinkedIn](https://img.shields.io/badge/LinkedIn-0077B5?style=for-the-badge&logo=linkedin&logoColor=white)](https://www.linkedin.com/in/elias-tiquicala-7672701b6/)

</div>

---

## 📞 Soporte

Si encontrás algún problema o tenés sugerencias:

- Abrí un [Issue](https://github.com/EliasTiki951/tikinet-cpp/issues)
- Contactame directamente

---

<div align="center">

**¡No olvides dejar una ⭐ si te fue útil!**

</div>
