# servidor.py
import grpc
import time
import hashlib
import random
from concurrent import futures

# Importa as classes geradas pelo protoc
import urls_pb2
import urls_pb2_grpc

# Constante para o endereço base
BASE_URL = "http://localhost:50051/"
# Dicionário em memória para armazenar as associações: código -> URL longa
url_map = {}

class EncurtadorURLServicer(urls_pb2_grpc.EncurtadorURLServicer):
    """Implementa os métodos RPC do serviço EncurtadorURL."""

    def EncurtarURL(self, request, context):
        url_longa = request.url_longa
        print(f"Recebida requisição para encurtar: {url_longa}")

        # --- Lógica de geração do código curto (Simples) ---
        # Combina a URL com o tempo atual e faz um hash SHA256, pega os primeiros 7 caracteres
        unique_string = url_longa + str(time.time()) + str(random.randint(1, 1000))
        codigo_curto = hashlib.sha256(unique_string.encode()).hexdigest()[:7]

        # Armazena a associação
        url_map[codigo_curto] = url_longa
        
        url_curta = BASE_URL + codigo_curto
        print(f"URL Curta gerada: {url_curta}")

        # Retorna a resposta
        return urls_pb2.RespostaEncurtar(url_curta=url_curta)

    def ObterURLLonga(self, request, context):
        codigo_curto = request.codigo_curto
        print(f"Recebida requisição para obter URL longa: {codigo_curto}")

        # Busca no mapa
        url_longa = url_map.get(codigo_curto)

        if url_longa:
            print(f"Encontrada URL: {url_longa}")
            # Retorna a URL longa
            return urls_pb2.RespostaObterURLLonga(url_longa=url_longa)
        else:
            # Caso não encontre, pode-se retornar um erro gRPC ou uma string vazia (como no requisito)
            print("Código não encontrado.")
            # Para retornar erro, use: context.set_code(grpc.StatusCode.NOT_FOUND)
            # context.set_details('Código curto não encontrado')
            # return urls_pb2.RespostaObterURLLonga() 
            return urls_pb2.RespostaObterURLLonga(url_longa="")


def serve():
    # Cria o pool de threads para o servidor
    server = grpc.server(futures.ThreadPoolExecutor(max_workers=10))
    # Adiciona a implementação do servicer ao servidor
    urls_pb2_grpc.add_EncurtadorURLServicer_to_server(
        EncurtadorURLServicer(), server)
    
    # Define a porta de escuta
    porta = '50051'
    server.add_insecure_port(f'[::]:{porta}')
    
    print(f"Servidor gRPC iniciado e escutando na porta {porta}...")
    server.start()
    
    try:
        # Mantém o servidor rodando até que seja interrompido
        server.wait_for_termination()
    except KeyboardInterrupt:
        print("Servidor desligado.")
        server.stop(0)

if __name__ == '__main__':
    serve()
