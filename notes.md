# Está é uma pasta padrao para projetos em C++, Imgui, OpenGL3 e consequentemente o GLFW(frame work do open gl)

Autor: TH

Este é um projeto base (boilerplate) configurado para desenvolvimento ágil de aplicações de interface gráfica (GUI) em C++, utilizando **Premake5** para gerenciamento de build multiplataforma.

---

## 🛠️ Tecnologias Utilizadas

* **Linguagem:** C++17 (Garante recursos modernos e excelente performance).
* **Premake5 & Lua:** Automação de build. Em vez de escrever Makefiles gigantes ou se prender a uma IDE (como Visual Studio), usamos scripts em **Lua** para gerar os arquivos de compilação nativos de cada sistema operacional automaticamente.
* **GLFW:** Biblioteca multiplataforma para abertura de janelas, gerenciamento de contextos OpenGL e leitura de inputs (teclado/mouse).
* **OpenGL 3.3 (Core Profile):** API gráfica moderna utilizada para renderizar a interface acelerada por hardware (GPU).
* **Dear ImGui (Branch: Docking):** Biblioteca de GUI de modo imediato (*Immediate Mode GUI*), ideal para ferramentas de engenharia, dashboards e painéis de controle.

---

## 📁 Estrutura de Pastas

```text
├── externals/               # Bibliotecas de terceiros (ImGui via Sparse-Checkout)
├── include/                 # Arquivos de cabeçalho (.hpp) globais
├── src/                     # Código-fonte (.cpp) do projeto
│   └── main.cpp             # Ponto de entrada (Inicialização GLFW/OpenGL/ImGui)
├── compile_flags.txt        # Configuração de caminhos para o LSP (clangd/Neovim)
├── premake5.lua             # Script de configuração do projeto
└── NOTES.md                 # Este guia de anotações

⚙️ Configuração Inicial do Ambiente
1. Sistema de Checkout do ImGui

Para manter o projeto leve e rastreável, o ImGui é clonado diretamente na pasta externals/imgui usando o modo seletivo do Git (Sparse-Checkout):
Bash

mkdir -p externals/imgui && cd externals/imgui
git init
git remote add origin [https://github.com/ocornut/imgui.git](https://github.com/ocornut/imgui.git)
git config core.sparseCheckout true
echo "imgui*.cpp" >> .git/info/sparse-checkout
echo "imgui*.h" >> .git/info/sparse-checkout
echo "imconfig.h" >> .git/info/sparse-checkout
echo "imstb_*.h" >> .git/info/sparse-checkout
echo "backends/imgui_impl_glfw.*" >> .git/info/sparse-checkout
echo "backends/imgui_impl_opengl3.*" >> .git/info/sparse-checkout
echo "backends/imgui_impl_opengl3_loader.h" >> .git/info/sparse-checkout
git pull origin docking --depth 1
git sparse-checkout reapply
cd ../..

💻 Comandos de Compilação

Sempre que o arquivo premake5.lua for alterado, limpe o cache anterior e gere os novos arquivos de build.
🐧 No Linux (GCC / Make)
Bash

# 1. Limpeza de builds e Makefiles antigos
rm -rf Makefile *.make bin bin-int

# 2. Gerar o Makefile atualizado via Premake
./premake5 gmake2

# 3. Compilar o projeto
make

# 4. Executar
./bin/Debug/[Nome_Do_Projeto]

🪟 No Windows (Visual Studio 2022)
PowerShell

# Gerar a Solution (.sln) para o Visual Studio
.\premake5.exe vs2022

📝 Notas de Desenvolvimento & Dicas do Editor (Neovim)

    LSP (Clangd): O arquivo compile_flags.txt na raiz avisa o editor onde encontrar as definições do ImGui. Se adicionar novas bibliotecas em externals, lembre-se de adicionar o caminho com -I nele para sumir com os erros de sintaxe em tempo real.

    Ordem dos Includes: No main.cpp, o cabeçalho do OpenGL (<GL/gl.h>) deve sempre ser importado antes do GLFW (<GLFW/glfw3.h>), caso contrário, o compilador gerará erros de redefinição de escopo de macro no Linux.

    Tela Cheia ImGui: O loop principal deste template está configurado para capturar dinamicamente o tamanho do framebuffer do GLFW e esticar a janela do ImGui em 100% da tela, desativando bordas, títulos e movimentações para simular um app nativo.
