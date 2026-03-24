
# =======================================================================
# Nomes: Beatriz Aparecida de Mello Barbosa (10354067)
#        Matheus Veiga Bacetic Joaquim (10425638)
#        Gabriel Pereira Faravola (10427189)
# Síntese: Aplicação para modelagem e manipulação de um grafo de
#          recomendação de jogos (Não-Direcionado, Ponderado nas Arestas)
# =======================================================================

import os

class GrafoRecomendacao:
    """Classe que gerencia o grafo de jogos usando Lista de Adjacência."""

    def __init__(self):
        self.tipo = 2  # 2 = Grafo não orientado com peso na aresta
        self.V = 0     # Contador de vértices
        self.rotulos = {} # Dicionário para mapear ID -> Nome do Jogo
        # Lista de Adjacência: lista onde cada índice (ID) contém um dicionário {id_vizinho: peso}
        self.adj = []

    def ler_arquivo(self, nome_arquivo="grafo.txt"):
        """Lê a estrutura do grafo a partir de um arquivo .txt formatado."""
        try:
            with open(nome_arquivo, 'r', encoding='utf-8') as f:
                # Filtra linhas vazias para evitar erros de leitura
                linhas = [l.strip() for l in f.readlines() if l.strip()]

            if len(linhas) < 2:
                print("\n[-] O arquivo está vazio ou incompleto.")
                return

            self.tipo = int(linhas[0])
            self.V = int(linhas[1])
            self.adj = [{} for _ in range(self.V)]
            self.rotulos = {}

            linha_atual = 2
            # 1. Leitura dos Vértices (ID e Nome entre aspas)
            for _ in range(self.V):
                if linha_atual < len(linhas):
                    partes = linhas[linha_atual].split('"')
                    id_v = int(partes[0].strip())
                    nome = partes[1].strip() if len(partes) > 1 else f"Desconhecido {id_v}"
                    self.rotulos[id_v] = nome
                    linha_atual += 1
                else:
                    break

            # 2. Leitura das Arestas
            if linha_atual < len(linhas):
                linha_atual += 1 # Pula a linha que indica a contagem de arestas
                num_arestas_lidas = 0
                while linha_atual < len(linhas):
                    try:
                        u, v, peso = map(int, linhas[linha_atual].split())
                        if u < self.V and v < self.V:
                            self.adj[u][v] = peso
                            self.adj[v][u] = peso # Simetria: u-v é o mesmo que v-u
                            num_arestas_lidas += 1
                    except ValueError:
                        pass # Ignora linhas mal formatadas
                    linha_atual += 1

            self.V = len(self.rotulos)
            print(f"\n[+] Sucesso! Grafo carregado com {self.V} jogos e {num_arestas_lidas} conexões.")

        except FileNotFoundError:
             print(f"\n[-] Erro: Arquivo '{nome_arquivo}' não encontrado.")
        except Exception as e:
            print(f"\n[-] Erro inesperado ao ler arquivo: {e}")

    def gravar_arquivo(self, nome_arquivo="grafo.txt"):
        """Exporta o estado atual do grafo de volta para o arquivo .txt."""
        try:
            arestas = []
            for u in range(self.V):
                for v, peso in self.adj[u].items():
                    if u <= v: # Evita duplicar a mesma aresta no arquivo texto
                        arestas.append((u, v, peso))

            with open(nome_arquivo, 'w', encoding='utf-8') as f:
                f.write(f"{self.tipo}\n")
                f.write(f"{self.V}\n")
                for i in range(self.V):
                    f.write(f'{i} "{self.rotulos.get(i, "Desconhecido")}"\n')
                f.write(f"{len(arestas)}\n")
                for u, v, peso in arestas:
                    f.write(f"{u} {v} {peso}\n")
            print(f"\n[+] Dados salvos com sucesso em '{nome_arquivo}'!")
        except Exception as e:
            print(f"\n[-] Erro ao gravar arquivo: {e}")

    def inserir_vertice(self, nome):
        """Adiciona um novo jogo (vértice) ao grafo."""
        novo_id = self.V
        self.adj.append({})
        self.rotulos[novo_id] = nome
        self.V += 1
        print(f"\n[+] Jogo '{nome}' inserido com ID {novo_id}.")

    def inserir_aresta(self, u, v, peso):
        """Cria uma conexão de similaridade (aresta) entre dois jogos."""
        if 0 <= u < self.V and 0 <= v < self.V:
            self.adj[u][v] = peso
            self.adj[v][u] = peso
            print(f"\n[+] Conexão criada entre '{self.rotulos[u]}' e '{self.rotulos[v]}' (Peso: {peso}).")
        else:
            print("\n[-] IDs de vértices inválidos.")

    def remover_vertice(self, id_v):
        """Remove um jogo e ajusta todos os IDs subsequentes para manter a integridade."""
        if 0 <= id_v < self.V:
            nome = self.rotulos.pop(id_v)
            self.adj.pop(id_v)
            self.V -= 1

            # Reindexação dos rótulos
            for i in range(id_v, self.V):
                self.rotulos[i] = self.rotulos.pop(i + 1)

            # Reindexação das conexões na lista de adjacência
            for u in range(self.V):
                nova_adj_u = {}
                for v, peso in self.adj[u].items():
                    if v == id_v:
                        continue
                    elif v > id_v:
                        nova_adj_u[v - 1] = peso # Diminui o ID do vizinho
                    else:
                        nova_adj_u[v] = peso
                self.adj[u] = nova_adj_u

            print(f"\n[+] Jogo '{nome}' removido e IDs reordenados.")
        else:
            print("\n[-] ID inválido.")

    def remover_aresta(self, u, v):
        """Remove a conexão entre dois jogos específicos."""
        if 0 <= u < self.V and 0 <= v < self.V:
            if v in self.adj[u]:
                del self.adj[u][v]
                del self.adj[v][u]
                print(f"\n[+] Conexão entre '{self.rotulos[u]}' e '{self.rotulos[v]}' removida.")
            else:
                print("\n[-] Conexão não encontrada.")

    def mostrar_arquivo(self, nome_arquivo="grafo.txt"):
        """Exibe o conteúdo bruto e formatado do arquivo de persistência."""
        print(f"\n{'='*50}\n📄 CONTEÚDO DO ARQUIVO: {nome_arquivo}\n{'='*50}")
        try:
            with open(nome_arquivo, 'r', encoding='utf-8') as f:
                linhas = [l.strip() for l in f.readlines() if l.strip()]

            if not linhas: return
            print(f"📌 TIPO: {linhas[0]} | 🎮 VÉRTICES: {linhas[1]}")

            linha_atual = 2
            for _ in range(int(linhas[1])):
                if linha_atual < len(linhas):
                    print(f"   {linhas[linha_atual]}")
                    linha_atual += 1

            if linha_atual < len(linhas):
                print(f"🔗 ARESTAS: {linhas[linha_atual]}")
                linha_atual += 1
                while linha_atual < len(linhas):
                    print(f"   {linhas[linha_atual]}")
                    linha_atual += 1
        except Exception as e:
            print(f"[-] Erro ao ler: {e}")
        print(f"{'='*50}")

    def mostrar_grafo(self):
        """Exibe a lista de adjacência de forma legível no console."""
        if self.V == 0: return print("\n[-] Grafo vazio.")
        print("\n--- LISTA DE ADJACÊNCIA ---")
        for i in range(self.V):
            conexoes = [f"[{v}: {self.rotulos[v]} (p:{p})]" for v, p in sorted(self.adj[i].items())]
            print(f"{i:02d} | {self.rotulos[i]:<20} -> {' -> '.join(conexoes) if conexoes else '(Isolado)'}")

    def verificar_conexidade(self):
        """Usa Busca em Largura (BFS) para identificar componentes conexas."""
        if self.V == 0: return
        visitados = [False] * self.V
        componentes = []

        for i in range(self.V):
            if not visitados[i]:
                comp = []
                fila = [i]
                visitados[i] = True
                while fila:
                    u = fila.pop(0)
                    comp.append(self.rotulos[u])
                    for v in self.adj[u]:
                        if not visitados[v]:
                            visitados[v] = True
                            fila.append(v)
                componentes.append(comp)

        print(f"\n📊 CONEXIDADE: {'CONEXO' if len(componentes) == 1 else 'DESCONEXO'}")
        print(f"   Total de componentes: {len(componentes)}")

def menu():
    """Interface de usuário via terminal."""
    sistema = GrafoRecomendacao()
    while True:
        print("\na) Ler arquivo | b) Gravar | c) +Vértice | d) +Aresta | e) -Vértice")
        print("f) -Aresta  | g) Ver Arq | h) Ver Grafo | i) Conexidade | j) Sair")

        op = input("\nEscolha: ").lower()
        if op == 'a': sistema.ler_arquivo()
        elif op == 'b': sistema.gravar_arquivo()
        elif op == 'c': sistema.inserir_vertice(input("Nome: "))
        elif op == 'd':
            try: sistema.inserir_aresta(int(input("ID1: ")), int(input("ID2: ")), int(input("Peso: ")))
            except: print("Erro: use números.")
        elif op == 'e':
            try: sistema.remover_vertice(int(input("ID: ")))
            except: print("Erro: use número.")
        elif op == 'f':
            try: sistema.remover_aresta(int(input("ID1: ")), int(input("ID2: ")))
            except: print("Erro: use números.")
        elif op == 'g': sistema.mostrar_arquivo()
        elif op == 'h': sistema.mostrar_grafo()
        elif op == 'i': sistema.verificar_conexidade()
        elif op == 'j': break

if __name__ == "__main__":
    menu()

