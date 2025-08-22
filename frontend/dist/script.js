async function loadConfig() {
    const response = await fetch("dist/config.json");
    console.log(response);
    return response.json();
}
async function startSocket() {
    const config = await loadConfig();
    console.log(config);
    //"wss://hangman.cropie.online/ws"
    const ws = new WebSocket(config.WS_HOST);
    ws.onmessage = (event) => {
        console.log(event);
    };
    document.addEventListener("keypress", (e) => {
        ws.send(e.key);
    });
}
async function init() {
    await startSocket();
}
await init();
export {};
