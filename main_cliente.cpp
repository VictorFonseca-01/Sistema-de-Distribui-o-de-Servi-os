#include <iostream>
#include <string>
#include <WS2tcpip.h>
#include "Protocolo.h"

void MostrarMenuDepartamentos() {
    std::cout << "1 - N1\n";
    std::cout << "2 - N2\n";
    std::cout << "3 - DISTRIBUICAO\n";
    std::cout << "4 - TRIAGEM\n";
}

int main() {
    // Inicialização Winsock
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "Falha no WSAStartup" << std::endl;
        return 1;
    }

    std::cout << "=== TERMINAL DO FUNCIONARIO ===\n";
    std::string nome;
    std::cout << "Qual o seu nome? ";
    std::getline(std::cin, nome);

    std::cout << "Qual o seu departamento?\n";
    MostrarMenuDepartamentos();
    std::cout << "Opcao: ";
    int opDepto;
    std::cin >> opDepto;

    SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == INVALID_SOCKET) {
        std::cerr << "Erro ao criar socket." << std::endl;
        WSACleanup();
        return 1;
    }

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(8080);
    inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr);

    if (connect(clientSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Falha ao conectar no servidor (127.0.0.1:8080). Certifique-se de que o servidor esta rodando." << std::endl;
        closesocket(clientSocket);
        WSACleanup();
        system("pause");
        return 1;
    }

    std::cout << "[SISTEMA] Conectado ao servidor!\n";

    // Enviar mensagem de REGISTRO logo apos a conexao
    MensagemRede msgRegistro;
    msgRegistro.tipoMensagem = TipoMensagem::REGISTRO;
    msgRegistro.idServico = 0; // irrelevante para registro
    msgRegistro.deptoAlvo = static_cast<Departamento>(opDepto);
    memset(msgRegistro.descricaoTarefa, 0, sizeof(msgRegistro.descricaoTarefa));
    strncpy_s(msgRegistro.nomeFuncionario, sizeof(msgRegistro.nomeFuncionario), nome.c_str(), _TRUNCATE);

    if (send(clientSocket, (char*)&msgRegistro, sizeof(msgRegistro), 0) == SOCKET_ERROR) {
        std::cerr << "Erro ao enviar registro." << std::endl;
        closesocket(clientSocket);
        WSACleanup();
        return 1;
    }

    std::cout << "[SISTEMA] Aguardando novas tarefas do supervisor...\n\n";

    // Loop bloqueante para aguardar tarefas (recv)
    while (true) {
        MensagemRede msgRecebida;
        int bytesReceived = recv(clientSocket, (char*)&msgRecebida, sizeof(msgRecebida), 0);

        if (bytesReceived > 0) {
            if (msgRecebida.tipoMensagem == TipoMensagem::NOVA_TAREFA) {
                // Imprime a tarefa recebida no terminal de forma clara
                std::cout << "========================================" << std::endl;
                std::cout << " [!] NOVA TAREFA RECEBIDA [!]" << std::endl;
                std::cout << " ID do Servico: " << msgRecebida.idServico << std::endl;
                std::cout << " Descricao    : " << msgRecebida.descricaoTarefa << std::endl;
                std::cout << "========================================\n" << std::endl;
            }
        } else if (bytesReceived == 0) {
            std::cout << "\n[SISTEMA] Conexao encerrada pelo servidor." << std::endl;
            break;
        } else {
            std::cout << "\n[SISTEMA] Erro de conexao com o servidor (SOCKET_ERROR)." << std::endl;
            break;
        }
    }

    closesocket(clientSocket);
    WSACleanup();
    system("pause");
    return 0;
}
