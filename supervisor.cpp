#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <mutex>
#include <fstream>
#include <algorithm>
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
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        cout << "Erro WSAStartup\n";
        return 1;
    }

    SOCKET s = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(8080);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(s, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
        cout << "Erro bind. A porta ja ta em uso.\n";
        return 1;
    }

    listen(s, SOMAXCONN);

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
                            lock_guard<mutex> lock(estadoMutex);
                            clientes.push_back({c, msg.deptoAlvo, msg.nomeFuncionario});
                            
                            // DISPARA TAREFAS ATRASADAS DA FILA
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

                            cout << "\n[CENTRAL] " << msg.nomeFuncionario << " logou (Depto " << (int)msg.deptoAlvo << ").";
                            if (enviadas > 0) {
                                cout << " " << enviadas << " tarefa(s) atrasada(s) da fila enviada(s)!";
                            }
                            cout << "\n> ";
                        }
                        
                        // loop aguardando confirmacoes ou queda de conexao
                        while (true) {
                            int r = recv(c, (char*)&msg, sizeof(msg), 0);
                            if (r > 0) {
                                if (msg.tipoMensagem == TipoMensagem::CONCLUSAO) {
                                    cout << "\n[CENTRAL] " << msg.nomeFuncionario << " concluiu a tarefa ID " << msg.idServico << "!\n> ";
                                }
                            } else {
                                cout << "\n[CENTRAL] " << msg.nomeFuncionario << " desconectou.\n> ";
                                lock_guard<mutex> lock(estadoMutex);
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
        system("cls"); // limpa a tela a cada loop pra ficar limpo!
        cout << "=== PAINEL DA CENTRAL ===\n";
        cout << "Aguardando inserir tarefas...\n\n";

        MensagemRede tarefa;
        tarefa.tipoMensagem = TipoMensagem::NOVA_TAREFA;
        memset(tarefa.nomeFuncionario, 0, 50);

        int idValido = 0;
        while (!idValido) {
            cout << "> ID da tarefa: ";
            cin >> tarefa.idServico;
            if (cin.fail()) {
                cin.clear();
                cin.ignore(1000, '\n');
                continue;
            }
            
            // VERIFICADOR DE ID DUPLICADO
            lock_guard<mutex> lock(estadoMutex);
            if (find(idsUsados.begin(), idsUsados.end(), tarefa.idServico) != idsUsados.end()) {
                cout << "[ERRO] Esse ID ja existe! Tente outro numero.\n";
            } else {
                idsUsados.push_back(tarefa.idServico);
                idValido = 1;
            }
        }
        cin.ignore();

        cout << "> O que tem que fazer: ";
        string desc;
        getline(cin, desc);
        strncpy_s(tarefa.descricaoTarefa, 256, desc.c_str(), _TRUNCATE);

        int depto = 0;
        while (depto < 1 || depto > 4) {
            cout << "\n1-TI | 2-DP | 3-Almoxarifado | 4-Vendas\n";
            cout << "> Pra qual departamento enviar? ";
            cin >> depto;
            if (cin.fail()) {
                cin.clear();
                cin.ignore(1000, '\n');
            }
        }
        cin.ignore(); // Limpa o buffer do Enter para o pause abaixo
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
            
            // TAREFA NA FILA
            if (cont == 0) {
                tarefasPendentes.push_back(tarefa);
            }
        }

        // Salva no arquivo historico.txt
        ofstream log("historico.txt", ios::app);
        if (log.is_open()) {
            log << "Tarefa " << tarefa.idServico << " enviada para o depto " << depto << ". Descricao: " << tarefa.descricaoTarefa << "\n";
            log.close();
        }

        if (cont == 0) {
            cout << "\n[CENTRAL] Ninguem de " << depto << " online. Tarefa guardada na FILA DE ESPERA!\n";
        } else {
            cout << "\n[CENTRAL] Enviado pra " << cont << " pessoa(s).\n";
        }
        
        cout << "Aperte ENTER para despachar outra tarefa...";
        string lixo;
        getline(cin, lixo);
    }

    return 0;
}
