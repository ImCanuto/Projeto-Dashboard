// formata o tamanho do arquivo para algo mais legível
function formatSize(size) {
            const i = Math.floor(Math.log(size) / Math.log(1024));
            return (size / Math.pow(1024, i)).toFixed(2) * 1 + ' ' + ['B', 'KB', 'MB', 'GB', 'TB'][i];
        }
        // get dos parâmetros, quase a mesma coisa do details.js
        window.onload = () => {
            const params = new URLSearchParams(window.location.search);
            const name = params.get('name');
            const path = params.get('path');
            const isDirectory = params.get('isDirectory') === 'true';
            const size = params.get('size');
            const mode = params.get('mode');
            const creationTime = params.get('creationTime');
            const modificationTime = params.get('modificationTime');
            // get dos detalhes e criação da estrutura HTML
            document.getElementById('details').innerHTML = `
                <h2>Detalhes de: ${name}</h2>
                <ul>
                    <li><strong>Nome:</strong> ${name}</li>
                    <li><strong>Caminho:</strong> ${path}</li>
                    <li><strong>Tipo:</strong> ${isDirectory ? 'Diretório' : 'Arquivo'}</li>
                    ${!isDirectory ? `<li><strong>Tamanho:</strong> ${formatSize(size)}</li>` : ''}
                    <li><strong>Modo:</strong> ${mode}</li>
                    <li><strong>Criação:</strong> ${creationTime}</li>
                    <li><strong>Modificação:</strong> ${modificationTime}</li>
                </ul>
                <button class="btn" onclick="window.history.back()">Voltar</button>
            `;
        };