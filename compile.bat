@echo off
chcp 65001 > nul
echo Компиляция программы сравнения алгоритмов сжатия...
echo.

echo Очистка старых файлов...
del *.o 2>nul
del compressor.exe 2>nul

echo Компиляция отдельных файлов...
g++ -c compression.cpp -o compression.o -std=c++11 -O2
if %errorlevel% neq 0 goto error

g++ -c Huffman_Elya/huffman.cpp -o huffman.o -std=c++11 -O2
if %errorlevel% neq 0 goto error

g++ -c LZSS_Masha/lzss.cpp -o lzss.o -std=c++11 -O2
if %errorlevel% neq 0 goto error

g++ -c LZ77_Diana/lz77.cpp -o lz77.o -std=c++11 -O2
if %errorlevel% neq 0 goto error

g++ -c LZ78_Miri/lz78.cpp -o lz78.o -std=c++11 -O2
if %errorlevel% neq 0 goto error

g++ -c LZW_Malika/lzw.cpp -o lzw.o -std=c++11 -O2
if %errorlevel% neq 0 goto error

g++ -c main.cpp -o main.o -std=c++11 -O2
if %errorlevel% neq 0 goto error

echo.
echo Линковка...
g++ main.o compression.o huffman.o lzss.o lz77.o lz78.o lzw.o -o compressor.exe -std=c++11 -O2
if %errorlevel% neq 0 goto error

echo.
echo ==========================================
echo ✓ КОМПИЛЯЦИЯ УСПЕШНА!
echo ==========================================
echo.

del *.o 2>nul

echo Запуск программы...
echo.
compressor.exe

goto end

:error
echo.
echo ==========================================
echo ✗ ОШИБКА КОМПИЛЯЦИИ!
echo ==========================================
echo.
pause
exit /b 1

:end
pause