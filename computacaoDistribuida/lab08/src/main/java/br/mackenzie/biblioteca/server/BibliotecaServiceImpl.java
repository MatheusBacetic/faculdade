package br.mackenzie.biblioteca.server;

import br.mackenzie.biblioteca.grpc.*;
import io.grpc.Status;
import io.grpc.stub.StreamObserver;

import java.util.*;
import java.util.concurrent.ConcurrentHashMap;
import java.util.stream.Collectors;

public class BibliotecaServiceImpl extends BibliotecaServiceGrpc.BibliotecaServiceImplBase {

    // Persistência em memória
    private final Map<String, Livro> acervo = new ConcurrentHashMap<>();
    private final Set<String> isbnsCadastrados = ConcurrentHashMap.newKeySet();

    @Override
    public void cadastrarLivro(LivroRequest request, StreamObserver<LivroResponse> responseObserver) {
        if (isbnsCadastrados.contains(request.getIsbn())) {
            responseObserver.onError(Status.ALREADY_EXISTS
                    .withDescription("Erro: O ISBN " + request.getIsbn() + " já está cadastrado.")
                    .asRuntimeException());
            return;
        }

        String novoId = UUID.randomUUID().toString();
        Livro novoLivro = Livro.newBuilder()
                .setId(novoId)
                .setTitulo(request.getTitulo())
                .setAutor(request.getAutor())
                .setAno(request.getAno())
                .setIsbn(request.getIsbn())
                .build();

        acervo.put(novoId, novoLivro);
        isbnsCadastrados.add(request.getIsbn());

        LivroResponse response = LivroResponse.newBuilder()
                .setIdGerado(novoId)
                .setStatus("SUCESSO: Livro cadastrado com êxito!")
                .build();

        responseObserver.onNext(response);
        responseObserver.onCompleted();
    }

    @Override
    public void listarLivrosPorAutor(AutorRequest request, StreamObserver<Livro> responseObserver) {
        List<Livro> livrosDoAutor = acervo.values().stream()
                .filter(livro -> livro.getAutor().equalsIgnoreCase(request.getNomeAutor()))
                .collect(Collectors.toList());

        if (livrosDoAutor.isEmpty()) {
            responseObserver.onError(Status.NOT_FOUND
                    .withDescription("Nenhum livro encontrado para o autor: " + request.getNomeAutor())
                    .asRuntimeException());
            return;
        }

        for (Livro livro : livrosDoAutor) {
            responseObserver.onNext(livro);
        }
        responseObserver.onCompleted();
    }

    @Override
    public StreamObserver<EmprestimoRequest> registrarEmprestimos(StreamObserver<ResumoEmprestimoResponse> responseObserver) {
        return new StreamObserver<EmprestimoRequest>() {
            int count = 0;
            long startTime = System.currentTimeMillis();

            @Override
            public void onNext(EmprestimoRequest emprestimoRequest) {
                // Simula o registro no banco
                count++;
            }

            @Override
            public void onError(Throwable t) {
                System.err.println("Erro no stream de empréstimos: " + t.getMessage());
            }

            @Override
            public void onCompleted() {
                long tempoTotal = System.currentTimeMillis() - startTime;
                ResumoEmprestimoResponse response = ResumoEmprestimoResponse.newBuilder()
                        .setTotalEmprestimos(count)
                        .setTempoProcessamentoMs(tempoTotal)
                        .build();

                responseObserver.onNext(response);
                responseObserver.onCompleted();
            }
        };
    }

    @Override
    public StreamObserver<ChatMessage> chatBibliotecario(StreamObserver<ChatMessage> responseObserver) {
        return new StreamObserver<ChatMessage>() {
            @Override
            public void onNext(ChatMessage chatMessage) {
                String mensagemCliente = chatMessage.getMensagem().toLowerCase();
                String sugestao = "Interessante! Posso te ajudar a encontrar algo específico se você mencionar um tema (ex: java, fantasia, banco de dados).";

                if (mensagemCliente.contains("java")) {
                    sugestao = "Recomendo a leitura de 'Effective Java' de Joshua Bloch.";
                } else if (mensagemCliente.contains("fantasia")) {
                    sugestao = "Temos a coleção completa de 'O Senhor dos Anéis' e 'Harry Potter'!";
                } else if (mensagemCliente.contains("banco de dados") || mensagemCliente.contains("sql")) {
                    sugestao = "Você deveria conferir 'Sistemas de Bancos de Dados' de Elmasri & Navathe.";
                }

                ChatMessage resposta = ChatMessage.newBuilder()
                        .setUsuario("Bibliotecário Virtual")
                        .setMensagem(sugestao)
                        .build();
                
                responseObserver.onNext(resposta);
            }

            @Override
            public void onError(Throwable t) {
                System.err.println("Erro no chat: " + t.getMessage());
            }

            @Override
            public void onCompleted() {
                responseObserver.onCompleted();
            }
        };
    }
}