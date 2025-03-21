# Toolchain file para compilación cruzada a Windows desde Linux

# Sistema objetivo
set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_PROCESSOR x86_64)

# Especificar los compiladores
set(CMAKE_C_COMPILER x86_64-w64-mingw32-gcc)
set(CMAKE_CXX_COMPILER x86_64-w64-mingw32-g++)
set(CMAKE_RC_COMPILER x86_64-w64-mingw32-windres)

# Donde buscar las cabeceras y librerías en el target
set(CMAKE_FIND_ROOT_PATH /usr/x86_64-w64-mingw32)

# Ajustar el comportamiento de búsqueda para cabeceras y librerías
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

# Ajustes para la compilación estática (para mejorar la portabilidad)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libgcc -static-libstdc++")
set(CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS "${CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS} -static-libgcc -static-libstdc++ -Wl,-Bstatic -lstdc++ -lpthread -Wl,-Bdynamic")

# Configurar el sufijo para los archivos ejecutables de Windows
set(CMAKE_EXECUTABLE_SUFFIX ".exe")
