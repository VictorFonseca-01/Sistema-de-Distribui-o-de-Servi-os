#pragma once

#include <winsock2.h>
#include <ws2tcpip.h>
#include <string>
#include <iostream>
#include <cstring>

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

// Struct (Plain Old Data - POD) com buffers de tamanho fixo para envio seguro via TCP
struct MensagemRede {
    TipoMensagem tipoMensagem;
    int idServico;
    Departamento deptoAlvo;
    char nomeFuncionario[50];
    char descricaoTarefa[256];
};

// Copia strings garantindo terminador nulo ('\0'), compatível com MSVC e GCC/MinGW
inline void copiarTextoSeguro(char* destino, size_t tamanhoDestino, const char* origem) {
    if (!destino || tamanhoDestino == 0) return;
    std::strncpy(destino, origem ? origem : "", tamanhoDestino - 1);
    destino[tamanhoDestino - 1] = '\0';
}

// Exibe o nome do departamento com destaque de cor no terminal Windows (CMD)
inline void imprimirDepartamentoColorido(Departamento depto) {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

    switch (depto) {
        case Departamento::TI:
            SetConsoleTextAttribute(hConsole, FOREGROUND_BLUE | FOREGROUND_INTENSITY); // Azul brilhante
            std::cout << "TI";
            break;
        case Departamento::DP:
            SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_INTENSITY); // Magenta
            std::cout << "DP";
            break;
        case Departamento::ALMOX:
            SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY); // Amarelo
            std::cout << "ALMOXARIFADO";
            break;
        case Departamento::VENDAS:
            SetConsoleTextAttribute(hConsole, FOREGROUND_GREEN | FOREGROUND_INTENSITY); // Verde
            std::cout << "VENDAS";
            break;
        default:
            std::cout << "DESCONHECIDO";
            break;
    }
    
    // Restaura a cor padrão do console (Branco/Cinza claro)
    SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
}
