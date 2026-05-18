# -*- coding: utf-8 -*-

# ===============================================================================
# Título: Similaridade de generos para Recomendação de Jogos Digitais

# Integrantes: Beatriz Aparecida de Mello Barbosa - RA: 10354067
#              Gabriel Pereira Faravola - RA: 10427189
#              Matheus Veiga Bacetic Joaquim - RA: 10425638
#
# Síntese: Aplicação para modelagem e manipulação de um grafo de
#          recomendação de jogos (Não-Direcionado, Ponderado nas Arestas)
# ===============================================================================

import kagglehub
import pandas as pd
import glob
import os
from itertools import combinations

path = kagglehub.dataset_download("jypenpen54534/steam-games-dataset-2021-2025-65k")

# 1. Localizar o arquivo CSV baixado especificamente do caminho do Kaggle
csv_files = glob.glob(os.path.join(path, "*.csv"))

if not csv_files:
    print("Nenhum arquivo CSV encontrado no caminho do dataset do Kaggle.")
else:
    # Carrega o primeiro CSV encontrado na pasta do kagglehub
    df = pd.read_csv(csv_files[0])
    
    # Removemos nulos e duplicatas para garantir integridade
    df_clean = df[['name', 'genres', 'recommendations']].dropna().drop_duplicates(subset=['name'])

    # Pegamos os 500 jogos com mais recomendações. Filtragem usando coluna 'recommendations' garantindo popularidade
    topQuinhentos = df_clean.sort_values(by='recommendations', ascending=False).head(500)

    # Selecionamos 70 aleatórios dentre os 500
    random70 = topQuinhentos.sample(n=min(70, len(topQuinhentos)))
    jogos = random70['name'].tolist()
    print("[+] Critério: Top 500 mais recomendados -> Amostra aleatória de 70")

    # Separação de gêneros (suportando ponto e vírgula ou vírgula)
    generos = [set(str(g).replace(',', ';').split(';')) for g in random70['genres']]

    v_count = len(jogos)
    arestas = []

    # 3. Calcular pesos (similaridade por gêneros)
    for i, j in combinations(range(v_count), 2):
        comum = generos[i].intersection(generos[j])
        peso = len(comum)
        if peso > 0:
            arestas.append((i, j, peso))
    print(f"[+] Vértices (Jogos): {v_count}")
    print(f"[+] Arestas (Conexões): {len(arestas)}")

    # 4. Salvar grafo.txt
    with open("grafo.txt", "w", encoding="utf-8") as f:
        f.write("2\n")
        f.write(f"{v_count}\n")
        for idx, nome in enumerate(jogos):
            f.write(f'{idx} "{nome}"\n')
        f.write(f"{len(arestas)}\n")
        for u, v, w in arestas:
            f.write(f"{u} {v} {w}\n")
    print("[+] Arquivo 'grafo.txt' gerado com sucesso usando o CSV do Kaggle!")

class GrafoRecomendacao:
    #Classe que gerencia o grafo de jogos usando Lista de Adjacência

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
            return True

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

    def buscarRecomendacao(self):
        ID = int(input("Digite o ID do jogo que deseja ver recomendações: "))
        jogos_similares = sorted(self.adj[ID].items(), key=lambda item: item[1], reverse=True)
        
        print("\n==================== RECOMENDAÇÕES ====================")
        print(f"Jogos semelhantes a \"{self.rotulos[ID]}\" (grau: {len(jogos_similares)})\n")

        for i in range(len(jogos_similares)):
            if i >= 10: # Apenas 10 recomendações
                break
            print(f"{self.rotulos[jogos_similares[i][0]]} ({jogos_similares[i][1]} generos em comum)")
    
    def analisar_propriedades(self):
        #Analisa coloração, graus, Euler e Hamilton do grafo
        if self.V == 0:
            return print("\n[-] Grafo vazio.")

        graus = [len(self.adj[i]) for i in range(self.V)]

        print("\nGRAUS DOS VÉRTICES")
        print(f"   Mínimo: {min(graus)}  |  Máximo: {max(graus)}  |  Médio: {sum(graus)/self.V:.2f}")
        top5 = sorted(range(self.V), key=lambda i: graus[i], reverse=True)[:5]
        print("   Top-5:")
        for i in top5:
            print(f"      [{i:02d}] {self.rotulos[i]:<30} grau={graus[i]}")

        visitados = [False] * self.V
        fila = [next((i for i in range(self.V) if graus[i] > 0), 0)]
        visitados[fila[0]] = True
        while fila:
            u = fila.pop(0)
            for v in self.adj[u]:
                if not visitados[v]:
                    visitados[v] = True
                    fila.append(v)
        conexo = all(visitados[i] or graus[i] == 0 for i in range(self.V))

        impares = sum(1 for g in graus if g % 2 != 0)
        print("\nEULER")
        if conexo and impares == 0:
            print("   ✅ Admite circuito euleriano")
        elif conexo and impares == 2:
            print("   ✅ Admite percurso euleriano (não-fechado)")
        else:
            motivo = "grafo desconexo" if not conexo else f"{impares} vértices de grau ímpar"
            print(f"   ❌ Não euleriano ({motivo})")

        cor = [-1] * self.V
        for u in sorted(range(self.V), key=lambda i: graus[i], reverse=True):
            usadas = {cor[v] for v in self.adj[u] if cor[v] != -1}
            c = 0
            while c in usadas:
                c += 1
            cor[u] = c
        num_cores = max(cor) + 1
        print(f"\nCOLORAÇÃO")
        print(f"   χ ≈ {num_cores} cores — {num_cores} partições independentes")

        dirac = all(g >= self.V / 2 for g in graus)
        ore = all(
            graus[u] + graus[v] >= self.V
            for u in range(self.V) for v in range(u + 1, self.V)
            if v not in self.adj[u]
        )
        print("\nHAMILTON")
        if self.V < 3:
            print("   ⚠️  Vértices insuficientes (mínimo 3)")
        elif dirac or ore:
            print("   ✅ Admite ciclo hamiltoniano (Dirac ou Ore satisfeito)")
            print("      → Também admite percurso hamiltoniano")
        elif conexo:
            print("   ⚠️  Condições suficientes não satisfeitas — inconclusivo")
            print("      (verificação exata é NP-completo)")
        else:
            print("   ❌ Não hamiltoniano (grafo desconexo)")

def menu():
    sistema = GrafoRecomendacao()
    fileLoaded = False
    while True:
        print("\n==================== MENU ====================")
        print("a) Ler arquivo\nb) Gravar\nc) +Vértice\nd) +Aresta\ne) -Vértice\nf) -Aresta\n" \
        "g) Ver Arq\nh) Ver Grafo\ni) Conexidade\nj) Recomendações\nk) Analisar Propriedades\nl) Sair")

        op = input("\n> ").lower().strip()

        if op == 'a': 
            fileLoaded = sistema.ler_arquivo()

        elif op == 'b': 
            if fileLoaded == False:
                print("Nenhum arquivo foi carregado. Utilize primeiro a opção 'a) Ler arquivo'.")
            else:
                sistema.gravar_arquivo()

        elif op == 'c': 
            if fileLoaded == False:
                print("Nenhum arquivo foi carregado. Utilize primeiro a opção 'a) Ler arquivo'.")
            else:
                sistema.inserir_vertice(input("Nome: "))

        elif op == 'd':
            if fileLoaded == False:
                print("Nenhum arquivo foi carregado. Utilize primeiro a opção 'a) Ler arquivo'.")
            else:
                try: sistema.inserir_aresta(int(input("ID1: ")), int(input("ID2: ")), int(input("Peso: ")))
                except: print("Erro: use números.")

        elif op == 'e':
            if fileLoaded == False:
                print("Nenhum arquivo foi carregado. Utilize primeiro a opção 'a) Ler arquivo'.")
            else:
                try: sistema.remover_vertice(int(input("ID: ")))
                except: print("Erro: use número.")

        elif op == 'f':
            if fileLoaded == False:
                print("Nenhum arquivo foi carregado. Utilize primeiro a opção 'a) Ler arquivo'.")
            else:
                try: sistema.remover_aresta(int(input("ID1: ")), int(input("ID2: ")))
                except: print("Erro: use números.")

        elif op == 'g': 
            if fileLoaded == False:
                print("Nenhum arquivo foi carregado. Utilize primeiro a opção 'a) Ler arquivo'.")
            else:    
                sistema.mostrar_arquivo()

        elif op == 'h': 
            if fileLoaded == False:
                print("Nenhum arquivo foi carregado. Utilize primeiro a opção 'a) Ler arquivo'.")
            else:
                sistema.mostrar_grafo()

        elif op == 'i': 
            if fileLoaded == False:
                print("Nenhum arquivo foi carregado. Utilize primeiro a opção 'a) Ler arquivo'.")
            else:
                sistema.verificar_conexidade()
        
        elif op == 'j': 
            if fileLoaded == False:
                print("Nenhum arquivo foi carregado. Utilize primeiro a opção 'a) Ler arquivo'.")
            else:
                sistema.buscarRecomendacao()

        elif op == 'k':
            if fileLoaded == False:
                print("Nenhum arquivo foi carregado. Utilize primeiro a opção 'a) Ler arquivo'.")
            else:
                sistema.analisar_propriedades()

        elif op == 'l': 
            print("\n")
            break
    
        else:
            print("Opção não reconhecida.")
        input("\nPressione ENTER para continuar...")

if __name__ == "__main__":
    menu()

# Script rápido para extrair a matriz limpa para o Graph Online
def gerar_matriz_limpa(nome_arquivo="grafo.txt"):
    with open(nome_arquivo, 'r', encoding='utf-8') as f:
        linhas = [l.strip() for l in f.readlines() if l.strip()]

    qtd_v = int(linhas[1])
    matriz = [[0] * qtd_v for _ in range(qtd_v)]

    linha_atual = 2 + qtd_v + 1 # Pula cabeçalho, vértices e a linha do total de arestas

    while linha_atual < len(linhas):
        partes = linhas[linha_atual].split()
        if len(partes) >= 3:
            u, v, peso = map(int, partes[:3])
            matriz[u][v] = peso
            matriz[v][u] = peso # Não-direcionado
        linha_atual += 1

    # Salva no formato exato que o Graph Online pede
    with open("matriz_para_copiar.txt", "w", encoding='utf-8') as f:
        for linha in matriz:
            f.write(", ".join(map(str, linha)) + "\n")

    print("[+] Arquivo 'matriz_para_copiar.txt' gerado com sucesso!")
    print("    Abra este arquivo, copie tudo e cole no Graph Online.")

if __name__ == "__main__":
    gerar_matriz_limpa()
