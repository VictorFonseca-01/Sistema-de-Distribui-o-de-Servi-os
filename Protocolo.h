#pragma once

#include <winsock2.h>
#include <ws2tcpip.h>
#include <string>
#include <iostream>

#pragma comment(lib, "ws2_32.lib")

enum class Departamento {
    TI = 1,
    DP = 2,
    ALMOX = 3,
    VENDAS = 4
};

enum class TipoMensagem {
    REGISTRO = 1,
    NOVA_TAREFA = 2,
    CONCLUSAO = 3
};

// Struct (Plain Old Data - POD) com buffers de tamanho fixo para envio seguro
struct MensagemRede {
    TipoMensagem tipoMensagem;
    int idServico;
    Departamento deptoAlvo;
    char nomeFuncionario[50];
    char descricaoTarefa[256];
};

// Função auxiliar para exibir o nome do departamento com a cor correspondente no Console do Windows
inline void imprimirDepartamentoColorido(Departamento depto) {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

    switch (depto) {
        case Departamento::TI:
            SetConsoleTextAttribute(hConsole, FOREGROUND_BLUE | FOREGROUND_INTENSITY); // Azul
            std::cout << "TI";
            break;
        case Departamento::DP:
            SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_INTENSITY); // Rosa / Magenta
            std::cout << "DP";
            break;
        case Departamento::ALMOX:
            SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY); // Amarelo
            std::cout << "ALMOX";
            break;
        case Departamento::VENDAS:
            SetConsoleTextAttribute(hConsole, FOREGROUND_GREEN | FOREGROUND_INTENSITY); // Verde
            std::cout << "VENDAS";
            break;
    }
    
    // Reseta para a cor padrão do console (cinza claro/branco)
    SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
}
