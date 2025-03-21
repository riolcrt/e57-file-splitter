# E57 File Splitter

Esta aplicación divide archivos E57 de gran tamaño en archivos más pequeños, cada uno por debajo de 4.5GB de tamaño. Preserva los grupos de escaneo y renombra los archivos de salida para incluir información de los escaneos.

## Requisitos

- CMake (3.10 o superior)
- Compilador compatible con C++17
- libE57Format (v3.2.0)

## Instalación de Dependencias

### libE57Format

Esta aplicación depende de la biblioteca libE57Format para leer y escribir archivos en formato E57. Debes descargarla e instalarla manualmente:

```bash
# Clonar el repositorio de libE57Format (versión 3.2.0)
git clone https://github.com/asmaloney/libE57Format.git
cd libE57Format
git checkout v3.2.0

# Crear directorio de compilación
mkdir -p build && cd build

# Configurar y compilar
cmake ..
make
sudo make install  # Opcional, para instalación global
```

Alternativamente, puedes colocar la biblioteca libE57Format en el directorio del proyecto:

```bash
# Desde el directorio raíz del proyecto
git clone https://github.com/asmaloney/libE57Format.git
git -C libE57Format checkout v3.2.0
```

## Configuración de Desarrollo

Este proyecto utiliza un DevContainer para desarrollo. Para comenzar:

1. Instalar [Visual Studio Code](https://code.visualstudio.com/)
2. Instalar la extensión [Remote - Containers](https://marketplace.visualstudio.com/items?itemName=ms-vscode-remote.remote-containers)
3. Abrir la carpeta del proyecto en VS Code
4. Cuando se le solicite, hacer clic en "Reabrir en Contenedor" o ejecutar el comando "Remote-Containers: Reopen in Container" desde la Paleta de Comandos (F1)

## Compilación del Proyecto

### Linux/macOS

```bash
mkdir -p build && cd build
cmake ..
make
```

El ejecutable se creará en el directorio `build`.

### Windows

Hay dos formas de compilar para Windows:

#### 1. Usando Visual Studio (Windows nativo)

- Instalar [Visual Studio](https://visualstudio.microsoft.com/) con soporte para C++
- Instalar [CMake](https://cmake.org/download/)
- Clonar este repositorio y libE57Format
- Generar los archivos de proyecto:
  ```bash
  mkdir build
  cd build
  cmake .. -G "Visual Studio 16 2019" -A x64
  ```
- Abrir el archivo `.sln` en Visual Studio y compilar

#### 2. Compilación cruzada desde Linux usando MinGW

Puedes compilar para Windows desde Linux utilizando el archivo de toolchain incluido:

```bash
# Instalar MinGW (necesario para compilación cruzada)
sudo apt-get install mingw-w64

# Configurar la compilación cruzada
mkdir build-windows && cd build-windows
cmake .. -DCMAKE_TOOLCHAIN_FILE=../windows-toolchain.cmake
make
```

Alternativamente, puedes usar Docker:

```bash
docker build -t e57splitter-windows -f Dockerfile.windows .
docker create --name temp e57splitter-windows
docker cp temp:/app/build-windows/e57splitter.exe .
docker rm temp
```

## Uso

```bash
./e57splitter /ruta/a/entrada.e57
```

La aplicación:
1. Analizará el archivo E57 de entrada
2. Agrupará los escaneos para crear archivos por debajo de 4.5GB
3. Creará archivos de salida con nombres basados en los escaneos contenidos

## Solución de Problemas

Si encuentras errores relacionados con libE57Format:

1. Asegúrate de que la versión de libE57Format sea la 3.2.0:
   ```bash
   cd libE57Format
   git checkout v3.2.0
   ```

2. La aplicación utiliza la API simplificada de libE57Format (E57SimpleReader, E57SimpleWriter) que es parte de la versión 3.2.0.

3. Si el recuento de puntos en los escaneos muestra valores idénticos (como 42359888), esto podría indicar un problema con los metadatos del archivo E57. La aplicación intentará validar esto leyendo una muestra de puntos.

## Notas Técnicas

- Los archivos de salida están limitados a 4.5GB para garantizar la compatibilidad con sistemas de archivos FAT32.
- El tamaño de los escaneos se estima en base al número de puntos y puede no ser exacto debido a la compresión.
- Los nombres de los archivos de salida se basan en los nombres de los escaneos incluidos.
