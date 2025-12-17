# AA_24_10_search_algorithms
Домашняя работа группы АА-24-10. Состав команды: Боликов Артемий, Воронина Мария, Байрамбаева Диана, Чугунова Элина, Этезова Малика, бабаев Мирюсиф. Алгоритмы сжатия файлов.

## СТРУКТУРА
search_project/<br>
├── main.cpp     # Основной файл с запуском и тестирование файлов <br>
├── compression.h    # Главный зоголовочный файл<br>
├── compression.cpp    # файл сохранения и чтения<br>
├── compressors.h          # Обявление функций сжатия<br>
├── method_Name_author/      # Папка с алгоитмом сжатия<br>
│   ├── method.cpp<br>
│   └── method.h<br>
├── tests/        # Тестовые файлы для тестов<br>
│   ├── idiot.txt<br>
│   └── ...<br>
│   # создаются автоматически<br>
├── compressed_methodname # сжатый файл<br>
└── decompressed_methodname # разжатый обратно файл<br>

1. Создание файла алгоритма
Создайте папку с названием алгоритма и вашем именем METHOD_Author, в нем создайте два файла method.cpp и method.h

2. Структура алгоритма
Каждый .ccp должен иметь следующую структуру:
    ```cpp
    #include "./.h"
    #include <map>
    #include <string>
    #include <iostream>


    CompressionResult название_алгоритма::compress(const char* inputFile, const char* outputFile) {
    
        CompressionResult result;
    
        // Чтение файла через readFile
        //...

        // Реализация вашего алгоритма поиска
        // ...

        // Запись через writeFile
        // ...
        return result;
    }

    CompressionResult LZSSCompressor::decompress(const char* inputFile, const char* outputFile) {

        CompressionResult result;
    
        // Реализация вашего алгоритма поиска
        // ...

        // Только запись через writeFile
        // ...
        return result;
    }
    ```


## Требования к алгоритмам
Функция должна возвращать CompressionResult

Функция должна принимать два параметра: inputFile и outputFile

Использовать compression::readFile() для чтения файлов и compression::writeFile()

Не использовать вывод в консоль (cout)