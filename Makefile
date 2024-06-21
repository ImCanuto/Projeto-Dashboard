# Nome dos executáveis
TARGET_A = extract_data
TARGET_B = extract_file_data

# Arquivos de código-fonte
SRC_A = extract_data.c
SRC_B = extract_file_data.c

# Compilador e flags
CC = gcc
CFLAGS = -pthread

# Comando Python e servidor HTTP
PYTHON = python3
SERVER_SCRIPT_A = server.py
SERVER_SCRIPT_B = -m http.server 8000

# Regras padrão
all: run

# Regras para compilar os programas C
$(TARGET_A): $(SRC_A)
	$(CC) $(CFLAGS) -o $(TARGET_A) $(SRC_A)

$(TARGET_B): $(SRC_B)
	$(CC) $(CFLAGS) -o $(TARGET_B) $(SRC_B)

# Regras para executar os programas C em segundo plano
run: $(TARGET_A) $(TARGET_B)
	./$(TARGET_A) & echo $$! > $(TARGET_A).pid
	./$(TARGET_B) & echo $$! > $(TARGET_B).pid

# Regras para iniciar os servidores Python
server:
	$(PYTHON) $(SERVER_SCRIPT_A) & echo $$! > server_a.pid
	$(PYTHON) $(SERVER_SCRIPT_B) & echo $$! > server_b.pid

# Regra para parar qualquer execução anterior e iniciar os programas e servidores
start: clean run server

# Regra para limpar os arquivos compilados e parar processos
clean:
	-killall -q $(TARGET_A) $(TARGET_B) || true
	rm -f $(TARGET_A) $(TARGET_A).pid $(TARGET_B) $(TARGET_B).pid server_a.pid server_b.pid

# Regra para parar os programas C e os servidores Python
stop:
	@if [ -f $(TARGET_A).pid ]; then \
		kill `cat $(TARGET_A).pid`; \
		rm -f $(TARGET_A).pid; \
	fi
	@if [ -f $(TARGET_B).pid ]; then \
		kill `cat $(TARGET_B).pid`; \
		rm -f $(TARGET_B).pid; \
	fi
	@if [ -f server_a.pid ]; then \
		kill `cat server_a.pid`; \
		rm -f server_a.pid; \
	fi
	@if [ -f server_b.pid ]; then \
		kill `cat server_b.pid`; \
		rm -f server_b.pid; \
	fi
	-pkill -f $(SERVER_SCRIPT_A)
	-pkill -f $(SERVER_SCRIPT_B)

# Regras padrão para iniciar o servidor apenas
.PHONY: all clean start server stop
