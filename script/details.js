// get dos parâmetros de uma página específica 
function getQueryParameter(name) {
    const urlParams = new URLSearchParams(window.location.search);
    return urlParams.get(name);
}

// fetch dos dados do JSON e criação da estrutura HTML
function fetchProcessDetails(pid) {
    fetch('process_data.json')
        .then(response => response.json())
        .then(data => {
            const process = data.find(p => p.pid == pid);
            if (process) {
                document.getElementById('details').innerHTML = `
                    <h2>Detalhes do Processo: ${process.pid}</h2>
                    <p><strong>Usuário:</strong> ${process.user}</p>
                    <p><strong>Nome:</strong> ${process.name}</p>
                    <p><strong>CPU:</strong> ${process.cpu.toFixed(2)}%</p>
                    <p><strong>Memória:</strong> ${process.memory.toFixed(2)} KB</p>
                    <p><strong>Threads:</strong> ${process.threads}</p>
                `;
            } else {
                document.getElementById('details').innerHTML = '<p>Processo não encontrado.</p>';
            }
        });
}

// método que chama os outros métodos com os dados de acordo com o PID escolhido
window.onload = () => {
    const pid = getQueryParameter('pid');
    if (pid) {
        fetchProcessDetails(pid);
    } else {
        document.getElementById('details').innerHTML = '<p>PID não fornecido.</p>';
    }
}