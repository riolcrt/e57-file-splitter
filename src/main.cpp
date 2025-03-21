#include <iostream>
#include <string>
#include <vector>
#include <filesystem>
#include <algorithm>
#include <numeric>

// Usando la API simplificada de E57Format
#include <E57SimpleReader.h>
#include <E57SimpleWriter.h>
#include <E57SimpleData.h>

namespace fs = std::filesystem;

// Estructura para almacenar información sobre un escaneo
struct ScanInfo {
    std::string name;
    int64_t pointCount;
    size_t estimatedSize;
    int scanIndex;
};

// Estimar el tamaño de una nube de puntos en bytes
size_t estimatePointCloudSize(const e57::Data3D& header) {
    // Estimación basada en el recuento de puntos y el tamaño típico del punto
    // Esta es una aproximación - el tamaño real depende de la compresión
    int64_t pointCount = header.pointCount;
    
    // Estimación aproximada: 30 bytes por punto
    // Incluye posición (12), color (6), intensidad (2), otros atributos y overhead
    return pointCount * 30;
}

// Función para analizar un archivo E57 y devolver información de escaneo
std::vector<ScanInfo> analyzeE57File(const std::string& filePath) {
    std::vector<ScanInfo> scans;
    
    try {
        // Abrir el archivo E57 para lectura
        e57::Reader reader(filePath, {});
        
        // Obtener el número de escaneos (Data3D)
        int scanCount = reader.GetData3DCount();
        
        if (scanCount == 0) {
            std::cerr << "Error: El archivo E57 no contiene secciones Data3D" << std::endl;
            return scans;
        }
        
        std::cout << "Archivo E57 contiene " << scanCount << " escaneos." << std::endl;
        
        // Obtener la información de la raíz del archivo E57
        e57::E57Root header;
        if (reader.GetE57Root(header)) {
            std::cout << "Información del archivo:" << std::endl;
            std::cout << "  Formato: " << header.formatName << std::endl;
            std::cout << "  Versión: " << header.versionMajor << "." << header.versionMinor << std::endl;
            std::cout << "  GUID: " << header.guid << std::endl;
        }
        
        // Iterar a través de todos los escaneos en el archivo
        for (int i = 0; i < scanCount; i++) {
            e57::Data3D header;
            
            // Leer los metadatos del escaneo
            if (!reader.ReadData3D(i, header)) {
                std::cerr << "Warning: No se puede leer el escaneo en el índice " << i << std::endl;
                continue;
            }
            
            // Obtener el nombre del escaneo
            std::string scanName = "Scan" + std::to_string(i);
            if (!header.name.empty()) {
                scanName = header.name;
            }
            
            // Obtener el recuento de puntos
            int64_t pointCount = header.pointCount;
            
            // Imprimir información detallada para depuración
            std::cout << "Escaneo[" << i << "] - " << scanName << ":" << std::endl;
            std::cout << "  GUID: " << (header.guid.empty() ? "No GUID" : header.guid) << std::endl;
            std::cout << "  Descripción: " << (header.description.empty() ? "No descripción" : header.description) << std::endl;
            std::cout << "  Fabricante del sensor: " << (header.sensorVendor.empty() ? "Desconocido" : header.sensorVendor) << std::endl;
            std::cout << "  Modelo del sensor: " << (header.sensorModel.empty() ? "Desconocido" : header.sensorModel) << std::endl;
            
            // Información de los puntos disponibles
            std::cout << "  Campos de puntos disponibles: ";
            if (header.pointFields.cartesianXField) std::cout << "X ";
            if (header.pointFields.cartesianYField) std::cout << "Y ";
            if (header.pointFields.cartesianZField) std::cout << "Z ";
            if (header.pointFields.intensityField) std::cout << "Intensidad ";
            if (header.pointFields.colorRedField) std::cout << "Rojo ";
            if (header.pointFields.colorGreenField) std::cout << "Verde ";
            if (header.pointFields.colorBlueField) std::cout << "Azul ";
            std::cout << std::endl;
            
            // Comprobar si pointCount parece válido
            if (pointCount == 0) {
                std::cout << "  ADVERTENCIA: El recuento de puntos es 0, esto podría indicar un problema." << std::endl;
            } else if (pointCount == 42359888) {
                std::cout << "  ADVERTENCIA: El recuento de puntos es exactamente 42359888, posible valor predeterminado o error." << std::endl;
            }
            
            // Intentar leer algunos puntos para validar el recuento
            std::cout << "  Intentando leer algunos puntos para validar..." << std::endl;
            
            e57::Data3DPointsFloat pointsData(header);
            const int64_t SAMPLE_SIZE = std::min(pointCount, static_cast<int64_t>(10));
            
            // Sólo leer unos pocos puntos para validar
            try {
                e57::CompressedVectorReader vectorReader = reader.SetUpData3DPointsData(i, SAMPLE_SIZE, pointsData);
                uint64_t numRead = vectorReader.read();
                vectorReader.close();
                
                std::cout << "  Leídos " << numRead << " puntos de muestra." << std::endl;
                
                // Si pudimos leer algunos puntos, comprobemos los valores
                if (numRead > 0 && pointsData.cartesianX != nullptr) {
                    std::cout << "  Primer punto: X=" << pointsData.cartesianX[0] 
                              << ", Y=" << pointsData.cartesianY[0] 
                              << ", Z=" << pointsData.cartesianZ[0] << std::endl;
                }
                
                if (numRead != SAMPLE_SIZE) {
                    std::cout << "  ADVERTENCIA: Se solicitaron " << SAMPLE_SIZE << " puntos pero sólo se leyeron " << numRead << std::endl;
                }
            } catch (const e57::E57Exception& e) {
                std::cout << "  ERROR al leer puntos: " << e.what() << std::endl;
                std::cout << "  Es posible que los metadatos de recuento de puntos sean incorrectos." << std::endl;
            }
            
            // Estimar el tamaño
            size_t estimatedSize = estimatePointCloudSize(header);
            
            // Añadir a la lista de escaneos
            scans.push_back({scanName, pointCount, estimatedSize, i});
            
            std::cout << "Encontrado escaneo: " << scanName << " con aproximadamente " 
                      << pointCount << " puntos (" 
                      << (estimatedSize / (1024 * 1024)) << " MB estimados)" << std::endl;
        }
    }
    catch (const e57::E57Exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
    
    return scans;
}

// Función para crear un nuevo archivo E57 con escaneos específicos
void createSplitE57File(const std::string& sourceFilePath, 
                        const std::string& outputFilePath,
                        const std::vector<int>& scanIndices) {
    try {
        // Abrir el archivo fuente
        e57::Reader reader(sourceFilePath, {});
        
        // Crear un nuevo archivo E57
        e57::WriterOptions options;
        options.guid = "E57Splitter_" + std::to_string(std::time(nullptr));
        e57::Writer writer(outputFilePath, options);
        
        // Copiar los escaneos especificados al nuevo archivo
        for (int scanIndex : scanIndices) {
            if (scanIndex < 0 || scanIndex >= reader.GetData3DCount()) {
                std::cerr << "Warning: Índice de escaneo inválido " << scanIndex << std::endl;
                continue;
            }
            
            // Leer los metadatos del escaneo fuente
            e57::Data3D header;
            if (!reader.ReadData3D(scanIndex, header)) {
                std::cerr << "Warning: No se puede leer el escaneo en el índice " << scanIndex << std::endl;
                continue;
            }
            
            // Obtener el nombre del escaneo
            std::string scanName = "Scan" + std::to_string(scanIndex);
            if (!header.name.empty()) {
                scanName = header.name;
            }
            
            std::cout << "Copiando escaneo: " << scanName << " al archivo de salida" << std::endl;
            
            // Crear un buffer para los datos de puntos (podemos usar float o double)
            e57::Data3DPointsFloat pointsData(header);
            
            // Configurar un lector de vectores para leer los datos
            e57::CompressedVectorReader vectorReader = reader.SetUpData3DPointsData(scanIndex, header.pointCount, pointsData);
            
            // Leer todos los puntos
            uint64_t numRead = vectorReader.read();
            vectorReader.close();
            
            std::cout << "Leídos " << numRead << " puntos del escaneo origen" << std::endl;
            
            // Si se leyeron puntos, escribir los datos al nuevo archivo
            if (numRead > 0) {
                // Escribir los datos en el nuevo archivo
                writer.WriteData3DData(header, pointsData);
                std::cout << "Escrito escaneo al archivo de salida" << std::endl;
            }
        }
        
        std::cout << "Creado archivo de salida: " << outputFilePath << std::endl;
    }
    catch (const e57::E57Exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}

int main(int argc, char* argv[]) {
    const int64_t MAX_FILE_SIZE = 4.5 * 1024 * 1024 * 1024; // 4.5 GB en bytes
    
    // Verificar argumentos de línea de comandos
    if (argc != 2) {
        std::cerr << "Uso: " << argv[0] << " <ruta_archivo_e57>" << std::endl;
        return 1;
    }
    
    std::string inputFilePath = argv[1];
    
    // Comprobar si existe el archivo de entrada
    if (!fs::exists(inputFilePath)) {
        std::cerr << "Error: El archivo de entrada no existe: " << inputFilePath << std::endl;
        return 1;
    }
    
    std::cout << "Analizando archivo E57: " << inputFilePath << std::endl;
    
    // Analizar el archivo para obtener información sobre los escaneos
    std::vector<ScanInfo> scans = analyzeE57File(inputFilePath);
    
    if (scans.empty()) {
        std::cerr << "Error: No se encontraron escaneos válidos en el archivo" << std::endl;
        return 1;
    }
    
    // Obtener el nombre del archivo de entrada sin extensión
    fs::path inputPath(inputFilePath);
    std::string baseFileName = inputPath.stem().string();
    std::string outputDir = inputPath.parent_path().string();
    
    // Agrupar escaneos en archivos de tamaño < MAX_FILE_SIZE
    std::vector<std::vector<int>> fileGroups;
    std::vector<int> currentGroup;
    size_t currentSize = 0;
    
    for (const auto& scan : scans) {
        // Si agregar este escaneo excedería el tamaño máximo, comenzar un nuevo grupo
        if (currentSize + scan.estimatedSize > MAX_FILE_SIZE && !currentGroup.empty()) {
            fileGroups.push_back(currentGroup);
            currentGroup.clear();
            currentSize = 0;
        }
        
        // Agregar el escaneo al grupo actual
        currentGroup.push_back(scan.scanIndex);
        currentSize += scan.estimatedSize;
    }
    
    // Agregar el último grupo si no está vacío
    if (!currentGroup.empty()) {
        fileGroups.push_back(currentGroup);
    }
    
    std::cout << "Se crearán " << fileGroups.size() << " archivos de salida" << std::endl;
    
    // Crear archivos de salida
    for (size_t groupIndex = 0; groupIndex < fileGroups.size(); groupIndex++) {
        const auto& group = fileGroups[groupIndex];
        
        // Crear un nombre para el archivo de salida basado en los nombres de los escaneos
        std::string outputNamePrefix = baseFileName + "_";
        if (group.size() == 1) {
            outputNamePrefix += scans[group[0]].name;
        } else {
            outputNamePrefix += scans[group.front()].name + "-" + scans[group.back()].name;
        }
        
        std::string outputFilePath = (fs::path(outputDir) / (outputNamePrefix + ".e57")).string();
        
        std::cout << "Creando archivo de salida " << (groupIndex + 1) << " de " << fileGroups.size() 
                  << ": " << outputFilePath << std::endl;
        
        // Crear el archivo de salida con los escaneos seleccionados
        createSplitE57File(inputFilePath, outputFilePath, group);
    }
    
    std::cout << "¡División del archivo E57 completa!" << std::endl;
    return 0;
}
