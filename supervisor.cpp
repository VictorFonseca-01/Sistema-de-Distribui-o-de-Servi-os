#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <mutex>
#include <fstream>
#include <algorithm>
#include <cstring>
#include "Protocolo.h"

using namespace std;

struct ClienteConectado {
    SOCKET socket;
    Departamento depto;
    string nome;
};

vector<ClienteConectado> clientes;
vector<MensagemRede> tarefasPendentes;
vector<int> idsUsados;
mutex estadoMutex;

int main() {
    // Configura o console do Windows para UTF-8
    SetConsoleOutputCP(65001);

    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        cout << "[ERRO] Erro no WSAStartup.\n";
        return 1;
    }

    SOCKET s = socket(AF_INET, SOCK_STREAM, 0);
    if (s == INVALID_SOCKET) {
        cout << "[ERRO] Erro ao criar socket do servidor.\n";
        WSACleanup();
        return 1;
    }

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(8080);
    addr.sin_addr.s_addr = INADDR_ANY;

    int opt = 1;
    setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt));

    if (bind(s, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
        cout << "[ERRO] Nao foi possivel iniciar a Central na porta 8080. A porta ja esta em uso.\n";
        closesocket(s);
        WSACleanup();
        system("pause");
        return 1;
    }

    if (listen(s, SOMAXCONN) == SOCKET_ERROR) {
        cout << "[ERRO] Erro ao escutar conexoes na porta 8080.\n";
        closesocket(s);
        WSACleanup();
        return 1;
    }

    system("color 0A");
    cout << "====================================================\n";
    cout << "  CENTRAL DE DISTRIBUICAO DE SERVICOS (SUPERVISOR) \n";
    cout << "  Servidor ativo na porta 8080                      \n";
    cout << "====================================================\n\n";

    // Thread background para aceitar novas conexões
    thread acceptThread([&]() {
        while (true) {
            SOCKET c = accept(s, NULL, NULL);
            if (c != INVALID_SOCKET) {
                thread([c]() {
                    MensagemRede msg{};
                    int res = recv(c, (char*)&msg, sizeof(msg), 0);
                    if (res > 0 && msg.tipoMensagem == TipoMensagem::REGISTRO) {
                        {
                            lock_guard<mutex> lock(estadoMutex);
                            clientes.push_back({c, msg.deptoAlvo, msg.nomeFuncionario});
                            
                            cout << "\n[CENTRAL] Funcionário \"" << msg.nomeFuncionario << "\" conectou no setor ";
                            imprimirDepartamentoColorido(msg.deptoAlvo);
                            cout << ".\n";

                            // Entrega tarefas pendentes da fila para o setor
                            int enviadas = 0;
                            for (auto it = tarefasPendentes.begin(); it != tarefasPendentes.end(); ) {
                                if (it->deptoAlvo == msg.deptoAlvo) {
                                    send(c, (char*)&(*it), sizeof(MensagemRede), 0);
                                    it = tarefasPendentes.erase(it);
                                    enviadas++;
                                } else {
                                    ++it;
                                }
                            }

                            if (enviadas > 0) {
                                cout << "[CENTRAL] " << enviadas << " tarefa(s) pendente(s) entregue(s) a " << msg.nomeFuncionario << "!\n";
                            }
                        }
                        
                        // Loop escutando confirmações ou desconexão do funcionário
                        while (true) {
                            int r = recv(c, (char*)&msg, sizeof(msg), 0);
                            if (r > 0) {
                                if (msg.tipoMensagem == TipoMensagem::CONCLUSAO) {
                                    lock_guard<mutex> lock(estadoMutex);
                                    cout << "\n[CENTRAL] Funcionário " << msg.nomeFuncionario << " concluiu a tarefa ID " << msg.idServico << "!\n";
                                }
                            } else {
                                lock_guard<mutex> lock(estadoMutex);
                                cout << "\n[CENTRAL] Um funcionário desconectou.\n";
                                for (size_t i = 0; i < clientes.size(); i++) {
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

    // Menu do supervisor para cadastro de tarefas
    while (true) {
        cout << "\n----------------------------------------------------\n";
        cout << " [NOVA TAREFA PARA DESPACHAR]\n";
        cout << "----------------------------------------------------\n";

        MensagemRede tarefa{};
        tarefa.tipoMensagem = TipoMensagem::NOVA_TAREFA;

        int idValido = 0;
        while (!idValido) {
            cout << "> ID da tarefa (numero inteiro): ";
            cin >> tarefa.idServico;
            if (cin.fail()) {
                cin.clear();
                cin.ignore(1000, '\n');
                cout << "[ERRO] Digite um numero inteiro valido.\n";
                continue;
            }
            
            lock_guard<mutex> lock(estadoMutex);
            if (find(idsUsados.begin(), idsUsados.end(), tarefa.idServico) != idsUsados.end()) {
                cout << "[ERRO] O ID " << tarefa.idServico << " ja foi utilizado! Escolha outro ID.\n";
            } else {
                idsUsados.push_back(tarefa.idServico);
                idValido = 1;
            }
        }
        cin.ignore(); // limpa buffer

        cout << "> Descricao da tarefa: ";
        string desc;
        getline(cin, desc);
        copiarTextoSeguro(tarefa.descricaoTarefa, sizeof(tarefa.descricaoTarefa), desc.c_str());

        int depto = 0;
        while (depto < 1 || depto > 4) {
            cout << " Setores disponiveis:\n";
            cout << "  1 - "; imprimirDepartamentoColorido(Departamento::TI); cout << "\n";
            cout << "  2 - "; imprimirDepartamentoColorido(Departamento::DP); cout << "\n";
            cout << "  3 - "; imprimirDepartamentoColorido(Departamento::ALMOX); cout << "\n";
            cout << "  4 - "; imprimirDepartamentoColorido(Departamento::VENDAS); cout << "\n";
            cout << "> Escolha o setor de destino (1 a 4): ";
            cin >> depto;
            if (cin.fail()) {
                cin.clear();
                cin.ignore(1000, '\n');
            }
        }
        cin.ignore();
        tarefa.deptoAlvo = (Departamento)depto;

        int cont = 0;
        {
            lock_guard<mutex> lock(estadoMutex);
            for (auto& cliente : clientes) {
                if (cliente.depto == tarefa.deptoAlvo) {
                    send(cliente.socket, (char*)&tarefa, sizeof(tarefa), 0);
                    cont++;
                }
            }
            
            if (cont == 0) {
                tarefasPendentes.push_back(tarefa);
            }
        }

        // Salva histórico de expedição em arquivo de log historico.txt
        ofstream logFile("historico.txt", ios::app);
        if (logFile.is_open()) {
            logFile << "[TAREFA " << tarefa.idServico << "] Depto: " << depto << " | Descricao: " << tarefa.descricaoTarefa << "\n";
            logFile.close();
        }

        if (cont == 0) {
            cout << "\n[CENTRAL] NENHUM funcionário do setor ";
            imprimirDepartamentoColorido(tarefa.deptoAlvo);
            cout << " online. Tarefa guardada na FILA DE ESPERA!\n";
        } else {
            cout << "\n[CENTRAL] Tarefa enviada com sucesso para " << cont << " funcionário(s) do setor ";
            imprimirDepartamentoColorido(tarefa.deptoAlvo);
            cout << ".\n";
        }
    }

    return 0;
}
