#pragma once

#include <winsock2.h>
#include <ws2tcpip.h>
#include <string>

#pragma comment(lib, "ws2_32.lib")

enum class Departamento {
    N1 = 1,
    N2 = 2,
    DISTRIBUICAO = 3,
    TRIAGEM = 4
};

enum class TipoMensagem {
    REGISTRO = 1,
    NOVA_TAREFA = 2
};

// Struct (Plain Old Data - POD) com buffers de tamanho fixo para envio seguro
struct MensagemRede {
    TipoMensagem tipoMensagem;
    int idServico;
    Departamento deptoAlvo;
    char nomeFuncionario[50];
    char descricaoTarefa[256];
};
