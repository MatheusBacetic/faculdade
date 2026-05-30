# Sistema de Gerenciamento de Biblioteca Digital (gRPC)

Projeto de Computação Distribuída demonstrando a comunicação entre microsserviços em Java 17 utilizando **gRPC e Protocol Buffers**.

## Aluno
- **Nome:** Matheus Veiga Bacetic Joaquim
- **RA:** 10425638
- **Nome:** Beatriz Aparecida de Mello Barbosa
- **RA:** 10354067
- **Instituição:** Universidade Presbiteriana Mackenzie

## Descrição do Projeto
Este projeto implementa o backend de um Sistema de Gerenciamento de Biblioteca Digital distribuído. O sistema é composto por um servidor gRPC central que oferece serviços para gerenciar livros, empréstimos e relatórios em tempo real, além de um cliente para testar as operações. 

Foram implementados os 4 tipos de comunicação gRPC:
1. **Unary RPC:** `cadastrarLivro`
2. **Server Streaming RPC:** `listarLivrosPorAutor`
3. **Client Streaming RPC:** `registrarEmprestimos`
4. **Bidirectional Streaming RPC:** `chatBibliotecario`

### Bônus Implementados (+10 pontos)
⭐ **Bônus 1 (Interceptor gRPC):** Implementado `AuthInterceptor` no servidor para gerar log automático de todas as chamadas RPC.
⭐ **Bônus 2 (Autenticação):** O interceptor também valida um *Bearer Token* enviado via `Metadata` no header das requisições.
⭐ **Bônus 3 (Timeout/Deadline):** Uso de `withDeadlineAfter` para definir um tempo máximo de espera nas chamadas síncronas do cliente.

---

## Como compilar e executar

### Pré-requisitos
- Java 17 ou superior
- Maven

### 1. Compilar o projeto
No terminal, na raiz do projeto (onde está o `pom.xml`), execute o comando abaixo para compilar o código e gerar as classes do Protocol Buffers:
```bash
mvn clean compile