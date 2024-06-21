// formata o tamanho do arquivo
function formatSize(size) {
    const i = Math.floor(Math.log(size) / Math.log(1024));
    return (size / Math.pow(1024, i)).toFixed(2) * 1 + ' ' + ['B', 'KB', 'MB', 'GB', 'TB'][i];
}

// fetch do sistema de arquivos e criação do HTML
function fetchFileSystemInfo() {
    fetch('file_system_info.json')
        .then(response => response.json())
        .then(data => {
            const fileSystemInfoDiv = document.getElementById('fileSystemInfo');
            fileSystemInfoDiv.innerHTML = `
                <h2>Informações do Sistema de Arquivos</h2>
                <table>
                    <tr>
                        <th>Partição</th>
                        <th>Tamanho Total</th>
                        <th>Usado</th>
                        <th>Percentual de Uso</th>
                    </tr>
                    ${data.map(partition => `
                        <tr>
                            <td>${partition.partition}</td>
                            <td>${formatSize(partition.totalSize)}</td>
                            <td>${formatSize(partition.usedSize)}</td>
                            <td>${partition.usagePercent.toFixed(2)}%</td>
                        </tr>
                    `).join('')}
                </table>
            `;
        });
}

// fetch da árvore de diretórios
function fetchDirectoryTree(path = '/') {
    fetch(`directory_tree.json?path=${encodeURIComponent(path)}`)
        .then(response => response.json())
        .then(data => {
            const directoryTreeDiv = document.getElementById('directoryTree');
            directoryTreeDiv.innerHTML = `
                <h2>Conteúdo de: ${data.path}</h2>
                <ul>
                    ${data.contents.map(item => `
                        <li ${item.isDirectory ? 'class="directory"' : ''} onclick="fetchDirectoryTree('${item.path}')">
                            ${item.name} ${!item.isDirectory ? `(${formatSize(item.size)})` : ''}
                        </li>
                    `).join('')}
                </ul>
            `;
        });
}

window.onload = () => {
    fetchFileSystemInfo();
    fetchDirectoryTree();
};
