// formata o tamanho do arquivo para algo mais legível 
function formatSize(size) {
    if (size === 0) return '0 B';
    const i = Math.floor(Math.log(size) / Math.log(1024));
    return (size / Math.pow(1024, i)).toFixed(2) + ' ' + ['B', 'KB', 'MB', 'GB', 'TB'][i];
}

// atualiza o path no servidor que atualiza o current_path.txt
function updatePath(path) {
    fetch('http://localhost:5000/update_path', {
        method: 'POST',
        headers: {
            'Content-Type': 'application/json'
        },
        body: JSON.stringify({ path })
    }).then(response => {
        if (response.ok) {
            fetchDirectoryTree(path);
        } else {
            console.error('Failed to update path:', response.statusText);
        }
    }).catch(error => {
        console.error('Error updating path:', error);
    });
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

// fetch da árvore de diretórios e criação do HTML
function fetchDirectoryTree(path = '/') {
    fetch('directory_tree.json')
        .then(response => response.json())
        .then(data => {
            const directoryTreeDiv = document.getElementById('directoryTree');
            directoryTreeDiv.innerHTML = `
                <h2>Conteúdo de: ${data.path}</h2>
                <button class="btn" onclick="updateRootPath()">Voltar para a raiz</button>
                <ul class="detailsUl">
                    ${data.contents.map(item => `
                        <li class="${item.isDirectory ? 'directory' : 'file'}" onclick="viewFileDetails('${item.name}', '${item.path}', ${item.isDirectory}, '${item.size}', '${item.mode}', '${item.creationTime}', '${item.modificationTime}', '${path}')">
                            ${item.name} ${!item.isDirectory ? `(${formatSize(item.size)})` : ''}
                            <ul>
                                <li>Path: ${item.path}</li>
                                ${!item.isDirectory ? `<li>Tamanho: ${formatSize(item.size)}</li>` : ''}
                                <li>Modo: ${item.mode}</li>
                                <li>Criação: ${item.creationTime}</li>
                                <li>Modificação: ${item.modificationTime}</li>
                            </ul>
                        </li>
                    `).join('')}
                </ul>
            `;
        });
}

// atualiza o path se for um diretório e redireciona para file_details.html se for um arquivo
function viewFileDetails(name, path, isDirectory, size, mode, creationTime, modificationTime, parentPath) {
    if (isDirectory) {
        updatePath(path);
    } else {
        const url = new URL('file_details.html', window.location.origin);
        url.searchParams.append('name', name);
        url.searchParams.append('path', path);
        url.searchParams.append('isDirectory', isDirectory);
        url.searchParams.append('size', size);
        url.searchParams.append('mode', mode);
        url.searchParams.append('creationTime', creationTime);
        url.searchParams.append('modificationTime', modificationTime);
        url.searchParams.append('parentPath', parentPath);
        window.location.href = url;
    }
}

// Função para atualizar o caminho para "/"
function updateRootPath() {
    updatePath('/');
}

// atualização dos dados a cada 2s
setInterval(() => {
    fetchFileSystemInfo();
    fetchDirectoryTree();
}, 2000);

// executa tudo ao carregar a tela
window.onload = () => {
    fetchFileSystemInfo();
    fetchDirectoryTree();
};
