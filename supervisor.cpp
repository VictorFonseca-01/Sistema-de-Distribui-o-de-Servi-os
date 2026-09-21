#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <mutex>
#include "Protocolo.h"

// Estrutura para rastrear os clientes logados no servidor
struct ClienteConectado {
    SOCKET socket;
    Departamento depto;
    std::string nome;
};

std::vector<ClienteConectado> clientes;
std::mutex clientesMutex;

// Thread dedicada para cada cliente, monitorando desconexões e o registro inicial
void ThreadCliente(SOCKET clientSocket) {
    MensagemRede msg;
    
    // 1. Aguarda mensagem de REGISTRO inicial (bloqueante)
    int bytesReceived = recv(clientSocket, (char*)&msg, sizeof(msg), 0);
    if (bytesReceived <= 0) {
        closesocket(clientSocket);
        return;
    }

    if (msg.tipoMensagem == TipoMensagem::REGISTRO) {
        std::string nome = msg.nomeFuncionario;
        Departamento depto = msg.deptoAlvo;

        // Adiciona cliente à lista global de forma thread-safe
        {
            std::lock_guard<std::mutex> lock(clientesMutex);
            clientes.push_back({clientSocket, depto, nome});
        }

        std::cout << "\n[CENTRAL] Novo funcionario conectado: " << nome << " (Depto: " << (int)depto << ")\n> ";

        // 2. Loop de monitoramento de vida do socket (bloqueante aguardando erro/fechamento)
        while (true) {
            int res = recv(clientSocket, (char*)&msg, sizeof(msg), 0);
            if (res <= 0) {
                // Alerta no console conforme o requisito
                std::cout << "\n[ALERTA] Conexao perdida com " << nome << " (Depto: " << (int)depto << ")\n> ";
                
                // Remove da lista ativa
                {
                    std::lock_guard<std::mutex> lock(clientesMutex);
                    for (auto it = clientes.begin(); it != clientes.end(); ++it) {
                        if (it->socket == clientSocket) {
                            clientes.erase(it);
                            break;
                        }
                    }
                }
                closesocket(clientSocket);
                break;
            }
        }
    }
}

// Thread principal de Accept para aceitar novas conexões
void ThreadAccept(SOCKET serverSocket) {
    while (true) {
        SOCKET clientSocket = accept(serverSocket, NULL, NULL);
        if (clientSocket != INVALID_SOCKET) {
            std::thread(ThreadCliente, clientSocket).detach();
        }
    }
}

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

    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET) {
        std::cerr << "Erro ao criar painel da central" << std::endl;
        WSACleanup();
        return 1;
    }

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(8080);
    inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr);

    if (bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Erro no bind. A porta 8080 pode estar ocupada." << std::endl;
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR) {
        std::cerr << "Erro no listen." << std::endl;
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    std::cout << "=== PAINEL DO SUPERVISOR (SERVIDOR) ===" << std::endl;
    std::cout << "Aguardando login de funcionarios na rede da empresa (Porta 8080)...\n" << std::endl;

    // Dispara a thread de Aceite
    std::thread acceptThread(ThreadAccept, serverSocket);
    acceptThread.detach();

    // Loop do Supervisor para envio de Tarefas
    while (true) {
        MensagemRede novaTarefa;
        novaTarefa.tipoMensagem = TipoMensagem::NOVA_TAREFA;
        memset(novaTarefa.nomeFuncionario, 0, sizeof(novaTarefa.nomeFuncionario));

        std::cout << "> Digite o ID do servico: ";
        if (!(std::cin >> novaTarefa.idServico)) {
            std::cin.clear();
            std::cin.ignore(10000, '\n');
            continue;
        }
        std::cin.ignore();

        std::cout << "> Digite a descricao da tarefa: ";
        std::string desc;
        std::getline(std::cin, desc);
        strncpy_s(novaTarefa.descricaoTarefa, sizeof(novaTarefa.descricaoTarefa), desc.c_str(), _TRUNCATE);

        std::cout << "> Escolha o Departamento Alvo:\n";
        MostrarMenuDepartamentos();
        int opDepto;
        while (true) {
            std::cout << "Opcao: ";
            if (std::cin >> opDepto && opDepto >= 1 && opDepto <= 4) {
                break;
            }
            std::cout << "[ERRO] Opcao invalida. Digite um numero de 1 a 4.\n";
            std::cin.clear();
            std::cin.ignore(10000, '\n');
        }
        novaTarefa.deptoAlvo = static_cast<Departamento>(opDepto);

        int enviados = 0;
        {
            std::lock_guard<std::mutex> lock(clientesMutex);
            // Broadcast para todos os sockets daquele departamento
            for (const auto& c : clientes) {
                if (c.depto == novaTarefa.deptoAlvo) {
                    send(c.socket, (char*)&novaTarefa, sizeof(novaTarefa), 0);
                    enviados++;
                }
            }
        }

        std::cout << "[CENTRAL] Tarefa " << novaTarefa.idServico << " enviada para " << enviados << " funcionario(s) do departamento " << opDepto << ".\n\n";
    }

    closesocket(serverSocket);
    WSACleanup();
    return 0;
}
