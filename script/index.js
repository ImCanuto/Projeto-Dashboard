// converter uptime em formato hh:mm:ss
function formatUptime(seconds) {
    const hrs = Math.floor(seconds / 3600);
    const mins = Math.floor((seconds % 3600) / 60);
    const secs = seconds % 60;
    return `${hrs.toString().padStart(2, '0')}:${mins.toString().padStart(2, '0')}:${secs.toString().padStart(2, '0')}`;
}

// fetch das informações globais do sistema e criação da estrutura HTML
function fetchGlobalInfo() {
    fetch('global_info.json')
        .then(response => response.json())
        .then(data => {
            const globalInfoDiv = document.getElementById('globalInfo');
            globalInfoDiv.innerHTML = `
                <p><strong>Tempo de Atividade:</strong> ${formatUptime(data.uptime)}</p>
                <p><strong>Memória Total:</strong> ${data.totalMemoryMB} MB</p>
                <p><strong>Memória Livre:</strong> ${data.freeMemory.toFixed(2)}%</p>
                <p><strong>Uso da CPU:</strong> ${data.cpuUsage.toFixed(2)}%</p>
                <p><strong>Processos Totais:</strong> ${data.totalProcesses}</p>
            `;
        });
}

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

// visualizar detalhes de uma tarefa em específico
function viewDetails(pid) {
    window.location.href = `details.html?pid=${pid}`;
}

// atualização dos dados a cada 5s
setInterval(() => {
    fetchData();
    fetchGlobalInfo();
}, 5000);
window.onload = () => {
    fetchData();
    fetchGlobalInfo();
};
