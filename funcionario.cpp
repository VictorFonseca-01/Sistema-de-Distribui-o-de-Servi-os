#include <iostream>
#include <string>
#include <WS2tcpip.h>
#include "Protocolo.h"

using namespace std;

int main() {
    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);

    cout << "--- TERMINAL DO FUNCIONARIO ---\n";
    string nome;
    cout << "Qual seu nome? ";
    getline(cin, nome);

    int depto = 0;
    while (depto < 1 || depto > 4) {
        cout << "\n1-TI | 2-DP | 3-Almoxarifado | 4-Vendas\n";
        cout << "Escolha o setor (1 a 4): ";
        cin >> depto;
        if (cin.fail()) {
            cin.clear();
            cin.ignore(1000, '\n');
        }
    }
    cin.ignore(); // limpa o enter pro proximo getline

    SOCKET s = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(8080);
    inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);

    if (connect(s, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
        cout << "Falha ao conectar. A central ta aberta?\n";
        system("pause");
        return 1;
    }

    system("cls"); // Limpa a tela pra iniciar o trabalho
    cout << "Conectado na Central!\n";

    // manda pra central quem acabou de logar
    MensagemRede msg;
    msg.tipoMensagem = TipoMensagem::REGISTRO;
    msg.deptoAlvo = (Departamento)depto;
    strncpy_s(msg.nomeFuncionario, 50, nome.c_str(), _TRUNCATE);

    send(s, (char*)&msg, sizeof(msg), 0);

    cout << "Aguardando tarefas...\n\n";

    while (true) {
        MensagemRede recebida;
        int bytes = recv(s, (char*)&recebida, sizeof(recebida), 0);

        if (bytes > 0) {
            if (recebida.tipoMensagem == TipoMensagem::NOVA_TAREFA) {
                cout << "\n===============================\n";
                cout << "     *** TAREFA RECEBIDA ***\n";
                cout << " ID: " << recebida.idServico << "\n";
                cout << " Fazer: " << recebida.descricaoTarefa << "\n";
                cout << "===============================\n\n";
                
                // Trava o sistema do funcionario ate ele confirmar que fez
                cout << "Aperte ENTER quando terminar o servico...";
                string lixo;
                getline(cin, lixo);

                // Envia a mensagem de volta confirmando a conclusao
                MensagemRede conf;
                conf.tipoMensagem = TipoMensagem::CONCLUSAO;
                conf.idServico = recebida.idServico;
                strncpy_s(conf.nomeFuncionario, 50, nome.c_str(), _TRUNCATE);
                send(s, (char*)&conf, sizeof(conf), 0);

                system("cls");
                cout << "Servico " << recebida.idServico << " concluido! Avisamos a central.\n";
                cout << "Aguardando novas tarefas...\n\n";
            }
        } else {
            cout << "\nA central caiu ou fechou o expediente.\n";
            break;
        }
    }

    closesocket(s);
    WSACleanup();
    system("pause");
    return 0;
}
