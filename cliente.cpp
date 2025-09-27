// cliente.cpp
#include <iostream>
#include <memory>
#include <string>

#include <grpc++/grpc++.h>

// Inclui os cabeçalhos gerados pelo protoc
#include "urls.grpc.pb.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;

using encurtadorurl::EncurtadorURL;
using encurtadorurl::RequisicaoEncurtar;
using encurtadorurl::RespostaEncurtar;
using encurtadorurl::RequisicaoObterURLLonga;
using encurtadorurl::RespostaObterURLLonga;

// Classe que interage com o serviço EncurtadorURL
class EncurtadorURLClient {
 public:
  EncurtadorURLClient(std::shared_ptr<Channel> channel)
      : stub_(EncurtadorURL::NewStub(channel)) {}

  // Chama o RPC EncurtarURL
  std::string EncurtarURL(const std::string& url_longa) {
    RequisicaoEncurtar request;
    request.set_url_longa(url_longa);

    RespostaEncurtar reply;
    ClientContext context;

    Status status = stub_->EncurtarURL(&context, request, &reply);

    if (status.ok()) {
      return reply.url_curta();
    } else {
      std::cout << "Erro RPC EncurtarURL: " << status.error_code() << ": " << status.error_message()
                << std::endl;
      return "";
    }
  }

  // Chama o RPC ObterURLLonga
  std::string ObterURLLonga(const std::string& codigo_curto) {
    RequisicaoObterURLLonga request;
    request.set_codigo_curto(codigo_curto);

    RespostaObterURLLonga reply;
    ClientContext context;

    Status status = stub_->ObterURLLonga(&context, request, &reply);

    if (status.ok()) {
      return reply.url_longa();
    } else {
      std::cout << "Erro RPC ObterURLLonga: " << status.error_code() << ": " << status.error_message()
                << std::endl;
      return "";
    }
  }

 private:
  std::unique_ptr<EncurtadorURL::Stub> stub_;
};

// Função principal de testes
int main(int argc, char** argv) {
  // Cria o canal de comunicação com o servidor gRPC
  std::string target_str = "localhost:50051";
  
  EncurtadorURLClient client(
      grpc::CreateChannel(target_str, grpc::InsecureChannelCredentials()));

  std::string original_url = "https://www.google.com/search?q=gRPC+microservice+example";
  std::cout << "1. Tentando encurtar a URL: " << original_url << std::endl;
  
  // 1. Chamar EncurtarURL
  std::string short_url = client.EncurtarURL(original_url);
  std::cout << "   -> URL Curta recebida: " << short_url << std::endl;

  if (short_url.empty()) {
    std::cerr << "Falha ao obter URL curta. Encerrando." << std::endl;
    return 1;
  }
  
  // Extrai o código curto da URL completa
  std::size_t last_slash = short_url.find_last_of('/');
  std::string short_code = short_url.substr(last_slash + 1);

  std::cout << "\n2. Usando o código curto: " << short_code << " para obter a URL longa." << std::endl;

  // 2. Chamar ObterURLLonga
  std::string retrieved_url = client.ObterURLLonga(short_code);
  
  std::cout << "   -> URL Longa Recuperada: " << retrieved_url << std::endl;

  if (retrieved_url == original_url) {
    std::cout << "\nTeste de interoperabilidade CONCLUÍDO com SUCESSO!" << std::endl;
  } else {
    std::cout << "\nTeste de interoperabilidade FALHOU. URLs não coincidem." << std::endl;
  }

  return 0;
}
