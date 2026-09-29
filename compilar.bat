@echo off
chcp 65001 > nul
title Compilador do Sistema de Distribuição de Serviços

echo ====================================================
echo   COMPILANDO SISTEMA DE DISTRIBUIÇÃO DE SERVIÇOS
echo ====================================================
echo.

where g++ >nul 2>nul
if %errorlevel% == 0 (
    echo [FOUND] Compilador MinGW GCC (g++) encontrado!
    echo Compilando supervisor.exe...
    g++ -std=c++11 supervisor.cpp -o supervisor.exe -lws2_32
    if %errorlevel% neq 0 (
        echo [ERRO] Falha ao compilar supervisor.cpp
        pause
        exit /b 1
    )

    echo Compilando funcionario.exe...
    g++ -std=c++11 funcionario.cpp -o funcionario.exe -lws2_32
    if %errorlevel% neq 0 (
        echo [ERRO] Falha ao compilar funcionario.cpp
        pause
        exit /b 1
    )

    echo.
    echo [SUCESSO] supervisor.exe e funcionario.exe compilados com sucesso!
    echo.
    echo Para executar no CMD:
    echo  1. Em um CMD, rode: supervisor.exe
    echo  2. Em outro CMD, rode: funcionario.exe
    echo.
    pause
    exit /b 0
)

where cl >nul 2>nul
if %errorlevel% == 0 (
    echo [FOUND] Compilador MSVC (cl.exe) encontrado!
    echo Compilando supervisor.exe...
    cl /EHsc supervisor.cpp /Fe:supervisor.exe ws2_32.lib
    if %errorlevel% neq 0 (
        echo [ERRO] Falha ao compilar supervisor.cpp
        pause
        exit /b 1
    )

    echo Compilando funcionario.exe...
    cl /EHsc funcionario.cpp /Fe:funcionario.exe ws2_32.lib
    if %errorlevel% neq 0 (
        echo [ERRO] Falha ao compilar funcionario.cpp
        pause
        exit /b 1
    )

    echo.
    echo [SUCESSO] supervisor.exe e funcionario.exe compilados com sucesso!
    echo.
    pause
    exit /b 0
)

echo [AVISO] Nenhum compilador C++ (g++ ou cl) encontrado diretamente no PATH do CMD.
echo.
echo DICA DE INSTALAÇÃO:
echo  - Se você usa MinGW / MSYS2 / Code::Blocks: Adicione a pasta 'bin' ao PATH do Windows.
echo  - Se você usa Visual Studio: Abra o "Developer Command Prompt for VS" e rode este script.
echo.
echo Exemplo manual com g++:
echo   g++ -std=c++11 supervisor.cpp -o supervisor.exe -lws2_32
echo   g++ -std=c++11 funcionario.cpp -o funcionario.exe -lws2_32
echo.
pause
