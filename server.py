from flask import Flask, request, jsonify
from flask_cors import CORS
from http.server import SimpleHTTPRequestHandler, HTTPServer
import threading
import os

# Cria a aplicação Flask
app = Flask(__name__)
CORS(app)

# Rota para atualizar o caminho
@app.route('/update_path', methods=['POST'])
def update_path():
    data = request.get_json()
    path = data.get('path', '/')
    print(f"Received path update request: {path}")  # Log para depuração
    with open('current_path.txt', 'w') as f:
        f.write(path)
    return jsonify({"status": "success"}), 200

# Função para executar o servidor HTTP
def run_http_server():
    os.chdir(os.path.dirname(os.path.abspath(__file__)))
    server_address = ('', 8000)
    httpd = HTTPServer(server_address, SimpleHTTPRequestHandler)
    print("Servidor HTTP rodando na porta 8000...")
    httpd.serve_forever()

# Inicia o servidor HTTP em uma thread separada
server_thread = threading.Thread(target=run_http_server)
server_thread.start()

# Executa a aplicação Flask no thread principal
if __name__ == '__main__':
    print("Servidor Flask rodando na porta 5000...")
    app.run(debug=True, port=5000)
