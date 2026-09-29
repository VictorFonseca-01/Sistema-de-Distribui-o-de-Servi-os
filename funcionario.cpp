#include <iostream>
#include <string>
#include <WS2tcpip.h>
#include "Protocolo.h"

using namespace std;

int main() {
    // Garante suporte a acentuacao UTF-8 no CMD do Windows
    SetConsoleOutputCP(65001);

    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        cout << "[ERRO] Falha ao inicializar o Winsock.\n";
        return 1;
    }

    system("color 0F");
    cout << "=========================================\n";
    cout << "       TERMINAL DO FUNCIONARIO           \n";
    cout << "=========================================\n\n";

    string nome;
    cout << "Qual seu nome? ";
    getline(cin, nome);
    while (nome.empty()) {
        cout << "O nome nao pode ser vazio. Digite seu nome: ";
        getline(cin, nome);
    }

    int depto = 0;
    while (depto < 1 || depto > 4) {
        cout << "\nSelecione o seu setor:\n";
        cout << " 1 - "; imprimirDepartamentoColorido(Departamento::TI); cout << "\n";
        cout << " 2 - "; imprimirDepartamentoColorido(Departamento::DP); cout << "\n";
        cout << " 3 - "; imprimirDepartamentoColorido(Departamento::ALMOX); cout << "\n";
        cout << " 4 - "; imprimirDepartamentoColorido(Departamento::VENDAS); cout << "\n";
        cout << "Escolha o setor (1 a 4): ";
        cin >> depto;
        if (cin.fail()) {
            cin.clear();
            cin.ignore(1000, '\n');
        }
    }
    cin.ignore(); // limpa buffer do enter

    cout << "\nDigite o IP da Central (pressione ENTER para 127.0.0.1): ";
    string ipCentral;
    getline(cin, ipCentral);
    if (ipCentral.empty()) {
        ipCentral = "127.0.0.1";
    }

    SOCKET s = socket(AF_INET, SOCK_STREAM, 0);
    if (s == INVALID_SOCKET) {
        cout << "[ERRO] Nao foi possivel criar o Socket.\n";
        WSACleanup();
        system("pause");
        return 1;
    }

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(8080);
    if (inet_pton(AF_INET, ipCentral.c_str(), &addr.sin_addr) <= 0) {
        cout << "[ERRO] Formato de IP invalido!\n";
        closesocket(s);
        WSACleanup();
        system("pause");
        return 1;
    }

    cout << "Conectando a Central (" << ipCentral << ":8080)...\n";
    if (connect(s, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
        cout << "[ERRO] Falha ao conectar. Certifique-se de que a Central (supervisor.exe) esta rodando.\n";
        closesocket(s);
        WSACleanup();
        system("pause");
        return 1;
    }

    system("cls");
    cout << "=========================================\n";
    cout << "           CONECTADO NA CENTRAL!         \n";
    cout << " Funcionario: " << nome << "\n Setor: ";
    imprimirDepartamentoColorido((Departamento)depto);
    cout << "\n=========================================\n\n";

    // Envia mensagem de registro do funcionário
    MensagemRede msg{};
    msg.tipoMensagem = TipoMensagem::REGISTRO;
    msg.deptoAlvo = (Departamento)depto;
    copiarTextoSeguro(msg.nomeFuncionario, sizeof(msg.nomeFuncionario), nome.c_str());

    send(s, (char*)&msg, sizeof(msg), 0);

    cout << "Aguardando tarefas atribuídas ao seu setor...\n\n";

    while (true) {
        MensagemRede recebida{};
        int bytes = recv(s, (char*)&recebida, sizeof(recebida), 0);

        if (bytes > 0) {
            if (recebida.tipoMensagem == TipoMensagem::NOVA_TAREFA) {
                cout << "\n=========================================\n";
                cout << "         *** TAREFA RECEBIDA ***         \n";
                cout << " ID da Tarefa: " << recebida.idServico << "\n";
                cout << " Descricao   : " << recebida.descricaoTarefa << "\n";
                cout << "=========================================\n\n";
                
                cout << "Pressione ENTER quando concluir este servico...";
                string lixo;
                getline(cin, lixo);

                // Envia confirmação de conclusão
                MensagemRede conf{};
                conf.tipoMensagem = TipoMensagem::CONCLUSAO;
                conf.idServico = recebida.idServico;
                copiarTextoSeguro(conf.nomeFuncionario, sizeof(conf.nomeFuncionario), nome.c_str());
                send(s, (char*)&conf, sizeof(conf), 0);

                system("cls");
                cout << "Servico ID " << recebida.idServico << " concluido com sucesso! Central notificada.\n";
                cout << "Aguardando novas tarefas...\n\n";
            }
        } else {
            cout << "\nConexao finalizada com a Central.\n";
            break;
        }
    }

    closesocket(s);
    WSACleanup();
    system("pause");
    return 0;
}
