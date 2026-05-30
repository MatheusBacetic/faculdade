package br.mackenzie.biblioteca.client;

import br.mackenzie.biblioteca.grpc.*;
import io.grpc.ManagedChannel;
import io.grpc.ManagedChannelBuilder;
import io.grpc.StatusRuntimeException;
import io.grpc.stub.MetadataUtils;
import io.grpc.Metadata;
import io.grpc.stub.StreamObserver;

import java.util.Iterator;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.TimeUnit;

public class ClienteBiblioteca {

    public static void main(String[] args) throws InterruptedException {
        ManagedChannel channel = ManagedChannelBuilder.forAddress("localhost", 50051)
                .usePlaintext()
                .build();

        // Configurando Token de Autenticação (Bônus)
        Metadata metadata = new Metadata();
        metadata.put(Metadata.Key.of("Authorization", Metadata.ASCII_STRING_MARSHALLER), "Bearer token-mackenzie-123");
        
        // Criando Stubs (Síncrono e Assíncrono)
        BibliotecaServiceGrpc.BibliotecaServiceBlockingStub blockingStub =
                BibliotecaServiceGrpc.newBlockingStub(channel)
                        .withInterceptors(MetadataUtils.newAttachHeadersInterceptor(metadata));

        BibliotecaServiceGrpc.BibliotecaServiceStub asyncStub =
                BibliotecaServiceGrpc.newStub(channel)
                        .withInterceptors(MetadataUtils.newAttachHeadersInterceptor(metadata));

        System.out.println("--- INICIANDO ROTEIRO DE TESTES ---");

        // 1. Cadastrar 3 livros (Unary) + Bônus Deadline
        System.out.println("\n1. Cadastrando livros...");
        cadastrarLivro(blockingStub, "Clean Code", "Robert C. Martin", 2008, "978-0132350884");
        cadastrarLivro(blockingStub, "Effective Java", "Joshua Bloch", 2017, "978-0134685991");
        cadastrarLivro(blockingStub, "Arquitetura Limpa", "Robert C. Martin", 2017, "978-8550804606");

        // 2. Listar livros de um autor cadastrado (Server Streaming)
        System.out.println("\n2. Listando livros do autor 'Robert C. Martin'...");
        listarLivros(blockingStub, "Robert C. Martin");

        // 3. Listar livros de um autor inexistente
        System.out.println("\n3. Listando livros de autor inexistente ('J.R.R. Tolkien')...");
        listarLivros(blockingStub, "J.R.R. Tolkien");

        // 6. Testar ISBN duplicado (Unary)
        System.out.println("\n6. Tentando cadastrar livro com ISBN duplicado...");
        cadastrarLivro(blockingStub, "Clean Code 2", "Robert C. Martin", 2009, "978-0132350884");

        // 4. Registrar 5 empréstimos (Client Streaming)
        System.out.println("\n4. Registrando empréstimos em lote...");
        registrarEmprestimos(asyncStub);

        // 5. Chat Bidirecional
        System.out.println("\n5. Iniciando chat com bibliotecário...");
        iniciarChat(asyncStub);

        channel.shutdown().awaitTermination(5, TimeUnit.SECONDS);
    }

    private static void cadastrarLivro(BibliotecaServiceGrpc.BibliotecaServiceBlockingStub stub, 
                                       String titulo, String autor, int ano, String isbn) {
        try {
            LivroRequest request = LivroRequest.newBuilder()
                    .setTitulo(titulo).setAutor(autor).setAno(ano).setIsbn(isbn).build();
            
            // Adicionado Deadline/Timeout de 5 segundos (Bônus)
            LivroResponse response = stub.withDeadlineAfter(5, TimeUnit.SECONDS).cadastrarLivro(request);
            System.out.println("[SUCESSO] " + response.getStatus() + " ID: " + response.getIdGerado());
        } catch (StatusRuntimeException e) {
            System.err.println("[ERRO gRPC] Status: " + e.getStatus().getCode() + " | Descrição: " + e.getStatus().getDescription());
        }
    }

    private static void listarLivros(BibliotecaServiceGrpc.BibliotecaServiceBlockingStub stub, String autor) {
        try {
            AutorRequest request = AutorRequest.newBuilder().setNomeAutor(autor).build();
            Iterator<Livro> livros = stub.withDeadlineAfter(5, TimeUnit.SECONDS).listarLivrosPorAutor(request);
            while (livros.hasNext()) {
                Livro l = livros.next();
                System.out.println("  -> Encontrado: " + l.getTitulo() + " (" + l.getAno() + ")");
            }
        } catch (StatusRuntimeException e) {
            System.err.println("  [ERRO gRPC] " + e.getStatus().getCode() + ": " + e.getStatus().getDescription());
        }
    }

    private static void registrarEmprestimos(BibliotecaServiceGrpc.BibliotecaServiceStub asyncStub) throws InterruptedException {
        CountDownLatch latch = new CountDownLatch(1);
        
        StreamObserver<ResumoEmprestimoResponse> responseObserver = new StreamObserver<ResumoEmprestimoResponse>() {
            @Override
            public void onNext(ResumoEmprestimoResponse resumo) {
                System.out.println("  [RESUMO EMPRÉSTIMOS] Total: " + resumo.getTotalEmprestimos() + 
                                   " livros. Processado em: " + resumo.getTempoProcessamentoMs() + "ms.");
            }
            @Override
            public void onError(Throwable t) {
                System.err.println("Erro: " + t.getMessage());
                latch.countDown();
            }
            @Override
            public void onCompleted() {
                latch.countDown();
            }
        };

        StreamObserver<EmprestimoRequest> requestObserver = asyncStub.registrarEmprestimos(responseObserver);

        // Enviando 5 empréstimos em stream
        for (int i = 1; i <= 5; i++) {
            requestObserver.onNext(EmprestimoRequest.newBuilder().setUsuario("UsuarioX").setLivroId("LIVRO-" + i).build());
        }
        requestObserver.onCompleted();
        
        latch.await(3, TimeUnit.SECONDS);
    }

    private static void iniciarChat(BibliotecaServiceGrpc.BibliotecaServiceStub asyncStub) throws InterruptedException {
        CountDownLatch latch = new CountDownLatch(1);

        StreamObserver<ChatMessage> responseObserver = new StreamObserver<ChatMessage>() {
            @Override
            public void onNext(ChatMessage chatMessage) {
                System.out.println("  [Bibliotecário]: " + chatMessage.getMensagem());
            }
            @Override
            public void onError(Throwable t) {
                latch.countDown();
            }
            @Override
            public void onCompleted() {
                latch.countDown();
            }
        };

        StreamObserver<ChatMessage> requestObserver = asyncStub.chatBibliotecario(responseObserver);

        String[] mensagens = {
                "Olá, bom dia!",
                "Estou procurando um livro sobre java, pode me ajudar?",
                "Também queria algo de fantasia para o final de semana."
        };

        for (String msg : mensagens) {
            System.out.println("  [Você]: " + msg);
            requestObserver.onNext(ChatMessage.newBuilder().setUsuario("Matheus").setMensagem(msg).build());
            Thread.sleep(500); // Simulando delay de digitação
        }

        requestObserver.onCompleted();
        latch.await(3, TimeUnit.SECONDS);
    }
}