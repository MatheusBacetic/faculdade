package br.mackenzie.biblioteca.server;

import io.grpc.Server;
import io.grpc.ServerBuilder;
import java.io.IOException;

public class ServidorBiblioteca {
    public static void main(String[] args) throws IOException, InterruptedException {
        int porta = 50051;
        
        Server server = ServerBuilder.forPort(porta)
                .addService(new BibliotecaServiceImpl())
                .intercept(new AuthInterceptor()) // Adicionando os bônus de Log e Auth
                .build()
                .start();

        System.out.println("Servidor da Biblioteca rodando na porta " + porta);
        server.awaitTermination();
    }
}