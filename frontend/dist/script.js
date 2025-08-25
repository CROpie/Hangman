function parseResponse(event) {
    if (typeof event != "string")
        throw new Error(`Event was not a string: ${event}`);
    let json;
    try {
        json = JSON.parse(event);
    }
    catch (error) {
        throw new Error(`Input event could not be parsed: ${error.message}`);
    }
    return json;
}
async function loadConfig() {
    const response = await fetch("dist/config.json");
    console.log(response);
    return response.json();
}
async function startSocket() {
    const config = await loadConfig();
    const ws = new WebSocket(config.WS_HOST);
    ws.onmessage = (event) => {
        const response = parseResponse(event.data);
        console.log(response);
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
