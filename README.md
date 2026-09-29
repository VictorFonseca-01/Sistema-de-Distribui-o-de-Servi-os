# Sistema de Distribuição de Serviços (C++)

Sistema cliente-servidor em C++ utilizando a API Windows Sockets (`Winsock2`) para comunicação em tempo real via TCP entre a Central (Supervisor) e os Terminais de Funcionários divididos por departamentos.

## 📁 Estrutura do Projeto

- **`Protocolo.h`**: Definições das estruturas de pacotes da rede (`MensagemRede`), enumeradores de departamentos (`TI`, `DP`, `ALMOXARIFADO`, `VENDAS`), tipos de mensagens e funções auxiliares de cores no console.
- **`supervisor.cpp`**: Servidor Central multithread. Aceita conexões de múltiplos funcionários, permite despachar tarefas por departamento, gerencia fila de tarefas offline e registra histórico em `historico.txt`.
- **`funcionario.cpp`**: Cliente para o terminal do funcionário. Conecta-se à Central por IP/Porta (porta padrão: `8080`), registra o setor e aguarda/responde tarefas enviadas.
- **`compilar.bat`**: Script de automação para compilar os executáveis diretamente no CMD do Windows.

---

## 🛠️ Como Compilar no CMD do Windows

### Opção 1: Usando `compilar.bat`
Abra o **CMD** na pasta do projeto e execute:
```cmd
compilar.bat
```

### Opção 2: Compilação Manual com MinGW (g++)
```cmd
g++ -std=c++11 supervisor.cpp -o supervisor.exe -lws2_32
g++ -std=c++11 funcionario.cpp -o funcionario.exe -lws2_32
```

### Opção 3: Compilação Manual com Visual Studio (cl)
Abra o **Developer Command Prompt for VS** e rode:
```cmd
cl /EHsc supervisor.cpp /Fe:supervisor.exe ws2_32.lib
cl /EHsc funcionario.cpp /Fe:funcionario.exe ws2_32.lib
```

---

## 🚀 Como Executar no CMD

1. **Abra o 1º CMD** e inicie o Servidor Central:
   ```cmd
   supervisor.exe
   ```

2. **Abra o 2º CMD** (ou mais janelas para diferentes funcionários) e inicie o Terminal do Funcionário:
   ```cmd
   funcionario.exe
   ```
   - Insira o nome do funcionário.
   - Escolha o setor (1 - TI, 2 - DP, 3 - Almoxarifado, 4 - Vendas).
   - Insira o IP da Central (pressione **Enter** para usar `127.0.0.1` localmente).

---

## ✨ Recursos Implementados

- **Suporte Multithread**: Múltiplos funcionários podem se conectar concorrentemente.
- **Fila de Espera Offline**: Se uma tarefa for enviada para um setor sem funcionários online, ela é armazenada e entregue automaticamente assim que um funcionário daquele setor se conectar.
- **Cores Dinâmicas no CMD**: Identificação visual por cor para cada setor (TI = Azul, DP = Magenta, Almoxarifado = Amarelo, Vendas = Verde).
- **Validação de ID Duplicado**: Garante que o supervisor não reutilize o mesmo ID de tarefa.
- **Log de Expedição**: Registro de histórico em `historico.txt`.