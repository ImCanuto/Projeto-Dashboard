from http.server import SimpleHTTPRequestHandler, HTTPServer
import threading
import os

# Função para executar o servidor HTTP
def run_server():
    os.chdir(os.path.dirname(os.path.abspath(__file__)))
    server_address = ('', 8000)
    httpd = HTTPServer(server_address, SimpleHTTPRequestHandler)
    print("Servidor rodando na porta 8000...")
    httpd.serve_forever()

# Inicia o servidor em uma thread separada
server_thread = threading.Thread(target=run_server)
server_thread.start()
