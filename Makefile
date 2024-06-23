# Nome dos executáveis
TARGET_A = extract_data
TARGET_B = extract_file_data

# Arquivos de código-fonte
SRC_A = extract_data.c
SRC_B = extract_file_data.c

# Compilador e flags
CC = gcc
CFLAGS = -pthread

# Comando Python e script do servidor
PYTHON = python3
SERVER_SCRIPT = server.py

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

# Regras para iniciar o servidor Python
server:
	$(PYTHON) $(SERVER_SCRIPT) & echo $$! > server.pid

# Regra para parar qualquer execução anterior e iniciar os programas e servidores
start: stop clean run server

# Regra para limpar os arquivos compilados e parar processos
clean:
	-killall -q $(TARGET_A) $(TARGET_B) || true
	rm -f $(TARGET_A) $(TARGET_A).pid $(TARGET_B) $(TARGET_B).pid server.pid

# Regra para parar os programas C e o servidor Python
stop:
	@if [ -f $(TARGET_A).pid ]; then \
		kill `cat $(TARGET_A).pid`; \
		rm -f $(TARGET_A).pid; \
	fi
	@if [ -f $(TARGET_B).pid ]; then \
		kill `cat $(TARGET_B).pid`; \
		rm -f $(TARGET_B).pid; \
	fi
	@if [ -f server.pid ]; then \
		kill `cat server.pid`; \
		rm -f server.pid; \
	fi
	-pkill -f $(SERVER_SCRIPT)

# Regras padrão para iniciar o servidor apenas
.PHONY: all clean start server stop
