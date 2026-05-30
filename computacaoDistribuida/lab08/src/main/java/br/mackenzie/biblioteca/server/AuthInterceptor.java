package br.mackenzie.biblioteca.server;

import io.grpc.*;

public class AuthInterceptor implements ServerInterceptor {
    private static final Metadata.Key<String> AUTH_KEY =
            Metadata.Key.of("Authorization", Metadata.ASCII_STRING_MARSHALLER);
    private static final String VALID_TOKEN = "Bearer token-mackenzie-123";

    @Override
    public <ReqT, RespT> ServerCall.Listener<ReqT> interceptCall(
            ServerCall<ReqT, RespT> call,
            Metadata headers,
            ServerCallHandler<ReqT, RespT> next) {

        System.out.println("[LOG] Chamada RPC Recebida: " + call.getMethodDescriptor().getFullMethodName());

        String authHeader = headers.get(AUTH_KEY);
        if (authHeader == null || !authHeader.equals(VALID_TOKEN)) {
            call.close(Status.UNAUTHENTICATED.withDescription("Token inválido ou ausente"), headers);
            return new ServerCall.Listener<ReqT>() {};
        }

        return next.startCall(call, headers);
    }
}