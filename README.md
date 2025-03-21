# E57 File Splitter

Esta aplicación divide archivos E57 de gran tamaño en archivos más pequeños, cada uno por debajo de 4.5GB de tamaño. Preserva los grupos de escaneo y renombra los archivos de salida para incluir información de los escaneos.

## Requisitos

- CMake (3.10 o superior)
- Compilador compatible con C++17
- libE57Format (v3.2.0, incluido como submódulo)

## Configuración de Desarrollo

Este proyecto utiliza un DevContainer para desarrollo. Para comenzar:

1. Instalar [Visual Studio Code](https://code.visualstudio.com/)
2. Instalar la extensión [Remote - Containers](https://marketplace.visualstudio.com/items?itemName=ms-vscode-remote.remote-containers)
3. Abrir la carpeta del proyecto en VS Code
4. Cuando se le solicite, hacer clic en "Reabrir en Contenedor" o ejecutar el comando "Remote-Containers: Reopen in Container" desde la Paleta de Comandos (F1)

## Compilación del Proyecto

Una vez dentro del DevContainer:

```bash
mkdir -p build && cd build
cmake ..
make
```

El ejecutable se creará en el directorio `build/bin`.

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

1. Asegúrate de que el submódulo libE57Format esté actualizado:
   ```bash
   git submodule update --init --recursive
   ```

2. La aplicación utiliza la API simplificada de libE57Format (E57SimpleReader, E57SimpleWriter) que es parte de la versión 3.2.0.

## Notas Técnicas

- Los archivos de salida están limitados a 4.5GB para garantizar la compatibilidad con sistemas de archivos FAT32.
- El tamaño de los escaneos se estima en base al número de puntos y puede no ser exacto debido a la compresión.
- Los nombres de los archivos de salida se basan en los nombres de los escaneos incluidos.
