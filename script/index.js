// fetch dos dados do JSON e criação da estrutura HTML na tabela
function fetchData() {
    fetch('process_data.json')
        .then(response => response.json())
        .then(data => {
            const processTable = document.getElementById('processTable');
            processTable.innerHTML = `
                <tr>
                    <th>PID</th>
                    <th>Usuário</th>
                    <th>Nome</th>
                    <th>CPU (%)</th>
                    <th>Memória (KB)</th>
                    <th>Threads</th>
                </tr>
            `;
            data.forEach(process => {
                processTable.innerHTML += `
                    <tr onclick="viewDetails(${process.pid})">
                        <td>${process.pid}</td>
                        <td>${process.user}</td>
                        <td>${process.name}</td>
                        <td>${process.cpu.toFixed(2)}</td>
                        <td>${process.memory.toFixed(2)}</td>
                        <td>${process.threads}</td>
                    </tr>
                `;
            });
        });
}

// método para vizualizar detalhes de uma tarefa em específico
function viewDetails(pid) {
    window.location.href = `details.html?pid=${pid}`;
}

// atualização dos dados a cada 5s
setInterval(fetchData, 5000);
window.onload = fetchData;