# 🎮 Sistema de Recomendação de Jogos
<p align="center">
  <img src="https://img.shields.io/badge/Python-3776AB?style=for-the-badge&logo=python&logoColor=white" alt="Python" />
  <img src="https://img.shields.io/badge/Teoria_dos_Grafos-Mackenzie-red?style=for-the-badge" alt="Mackenzie" />
  <img src="https://img.shields.io/badge/Status-Parte_3_Concluída-green?style=for-the-badge" alt="Status" />
</p>

## 📌 Sobre o Projeto
Este projeto foi desenvolvido para a disciplina de **Teoria dos Grafos** na **Faculdade de Computação e Informática (FCI)** da Universidade Presbiteriana Mackenzie. 
A aplicação utiliza conceitos de grafos para resolver um problema de **Sistemas de Recomendação**. Através de um grafo não-orientado ponderado, o software analisa a similaridade entre títulos de videojogos, permitindo identificar conexões baseadas em categorias, mecânicas e tags em comum.

---

## 🔍 Modelagem do Problema
* **Vértices ($V$):** Representam os títulos dos jogos.
* **Arestas ($E$):** Representam a relação de recomendação ou similaridade entre dois títulos.
* **Pesos ($W$):** Quantificam a força da recomendação (ex: número de tags coincidentes).
* **Tipo de Grafo:** Não-orientado com peso nas arestas (Tipo 2).

---

## 🛠️ Funcionalidades do Menu
| Opção | Ação | Descrição |
| :---: | :--- | :--- |
| **a** | 📂 Ler Dados | Importa a estrutura do grafo a partir do ficheiro `grafo.txt`. |
| **b** | 💾 Gravar Dados | Salva o estado atual da memória de volta no ficheiro. |
| **c** | 🆕 Inserir Vértice | Adiciona um novo jogo à rede. |
| **d** | 🔗 Inserir Aresta | Cria um vínculo de similaridade entre dois jogos. |
| **e** | ❌ Remover Vértice | Elimina um jogo e limpa todas as suas conexões. |
| **f** | ✂️ Remover Aresta | Desfaz a relação de recomendação entre dois jogos. |
| **g** | 📄 Conteúdo TXT | Exibe a formatação bruta do ficheiro de dados. |
| **h** | 📊 Mostrar Grafo | Visualiza a lista de adjacência formatada. |
| **i** | 🌐 Conexidade | Analisa se a rede está integrada ou fragmentada. |
| **j** | 🎯 Recomendações | Exibe os jogos mais similares a um título informado pelo usuário. |
| **k** | 🔬 Analisar Propriedades | Verifica conectividade, euler, coloração e hamilton do grafo. |
| **l** | 🚪 Sair | Encerra a execução do programa. |

---

## 🌍 Objetivos de Desenvolvimento Sustentável (ODS)

ODS 9 - Indústria, Inovação e Infraestrutura: Desenvolvimento de algoritmos de inovação tecnológica para o setor de software e entretenimento.

---

## 📂 Estrutura do Ficheiro `grafo.txt`
```text
2           // Tipo de Grafo (Não-orientado ponderado)
n           // Quantidade total de vértices
id "Nome"   // Identificador e Rótulo do Jogo
...
m           // Quantidade total de arestas
u v peso    // Vértice 1, Vértice 2 e Peso da conexão
```
