# Nome do executável
TARGET = extract_data

# Arquivo de código-fonte
SRC = extract_data.c

# Compilador e flags
CC = gcc
CFLAGS = -pthread

# Comando Python
PYTHON = python3
SERVER_SCRIPT = server.py

# Regras padrão
all: run

# Regra para compilar o programa C
$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

# Regra para executar o programa C em segundo plano
run: $(TARGET)
	./$(TARGET) & echo $$! > extract_data.pid

# Regra para iniciar o servidor Python
server:
	$(PYTHON) $(SERVER_SCRIPT)

# Regra para parar qualquer execução anterior e iniciar o servidor
start: clean run server

# Regra para limpar os arquivos compilados e parar processos
clean:
	-killall -q $(TARGET) || true
	rm -f $(TARGET) extract_data.pid

# Regra para parar o programa C e o servidor Python
stop:
	@if [ -f extract_data.pid ]; then \
		kill `cat extract_data.pid`; \
		rm -f extract_data.pid; \
	fi
	-pkill -f $(SERVER_SCRIPT)

# Regra padrão para iniciar o servidor apenas
.PHONY: all clean start server stop
