#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <mutex>
#include "Protocolo.h"

using namespace std;

struct ClienteConectado {
    SOCKET socket;
    Departamento depto;
    string nome;
};

vector<ClienteConectado> clientes;
mutex clientesMutex;

int main() {
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        cout << "Erro WSAStartup\n";
        return 1;
    }

    SOCKET s = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(8080);
    inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);

    if (bind(s, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
        cout << "Erro bind. A porta ja ta em uso.\n";
        return 1;
    }

    listen(s, SOMAXCONN);

    cout << "=== PAINEL DA CENTRAL ===\n";
    cout << "Aguardando funcionarios...\n\n";

    // thread para aceitar os funcionarios
    thread acceptThread([&]() {
        while (true) {
            SOCKET c = accept(s, NULL, NULL);
            if (c != INVALID_SOCKET) {
                // thread pra cada funcionario conectado
                thread([c]() {
                    MensagemRede msg;
                    int res = recv(c, (char*)&msg, sizeof(msg), 0);
                    if (res > 0 && msg.tipoMensagem == TipoMensagem::REGISTRO) {
                        {
                            lock_guard<mutex> lock(clientesMutex);
                            clientes.push_back({c, msg.deptoAlvo, msg.nomeFuncionario});
                        }
                        cout << "\n[CENTRAL] " << msg.nomeFuncionario << " logou (Depto " << (int)msg.deptoAlvo << ").\n> ";
                        
                        // loop aguardando cair a conexao
                        while (true) {
                            int r = recv(c, (char*)&msg, sizeof(msg), 0);
                            if (r <= 0) {
                                cout << "\n[CENTRAL] " << msg.nomeFuncionario << " desconectou.\n> ";
                                lock_guard<mutex> lock(clientesMutex);
                                for (int i = 0; i < clientes.size(); i++) {
                                    if (clientes[i].socket == c) {
                                        clientes.erase(clientes.begin() + i);
                                        break;
                                    }
                                }
                                closesocket(c);
                                break;
                            }
                        }
                    } else {
                        closesocket(c);
                    }
                }).detach();
            }
        }
    });
    acceptThread.detach();

    // menu do supervisor
    while (true) {
        MensagemRede tarefa;
        tarefa.tipoMensagem = TipoMensagem::NOVA_TAREFA;
        memset(tarefa.nomeFuncionario, 0, 50);

        cout << "> ID da tarefa: ";
        cin >> tarefa.idServico;
        cin.ignore();

        cout << "> O que tem que fazer: ";
        string desc;
        getline(cin, desc);
        strncpy_s(tarefa.descricaoTarefa, 256, desc.c_str(), _TRUNCATE);

        int depto = 0;
        while (depto < 1 || depto > 4) {
            cout << "\n1-N1 | 2-N2 | 3-Distribuicao | 4-Triagem\n";
            cout << "> Pra qual departamento enviar? ";
            cin >> depto;
            if (cin.fail()) {
                cin.clear();
                cin.ignore(1000, '\n');
            }
        }
        tarefa.deptoAlvo = (Departamento)depto;

        int cont = 0;
        lock_guard<mutex> lock(clientesMutex);
        for (auto& cliente : clientes) {
            if (cliente.depto == tarefa.deptoAlvo) {
                send(cliente.socket, (char*)&tarefa, sizeof(tarefa), 0);
                cont++;
            }
        }

        cout << "[CENTRAL] Enviado pra " << cont << " pessoa(s).\n\n";
    }

    return 0;
}
