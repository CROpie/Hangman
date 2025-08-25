function renderer() {
    const FONT = "30px Arial";
    const COLOUR = "blue";
    let canvas;
    let ctx;
    function init() {
        const canvasEl = document.getElementById("renderCanvas");
        if (!(canvasEl instanceof HTMLCanvasElement))
            throw new Error("no canvas");
        canvas = canvasEl;
        const ctxEl = canvas.getContext("2d");
        if (!ctxEl)
            throw new Error("2D context not supported");
        ctx = ctxEl;
        ctx.font = FONT;
        ctx.fillStyle = COLOUR;
    }
    function render(guessState) {
        ctx.clearRect(0, 0, canvas.width, canvas.height);
        ctx.fillText(guessState, 50, 100);
    }
    return { init, render };
}
function parseResponse(event) {
    if (typeof event != "string")
        throw new Error(`Event was not a string: ${event}`);
    try {
        return JSON.parse(event);
    }
    catch (error) {
        throw new Error(`Input event could not be parsed: ${error.message}`);
    }
}
async function loadConfig() {
    const response = await fetch("dist/config.json");
    console.log(response);
    return response.json();
}
async function startSocket(renderService) {
    const config = await loadConfig();
    const ws = new WebSocket(config.WS_HOST);
    ws.onmessage = (event) => {
        const response = parseResponse(event.data);
        renderService.render(response.guessState);
        if (response.isWin) {
            alert("You win!!");
            ws.send(JSON.stringify({ isReset: true }));
        }
    };
    document.addEventListener("keypress", (e) => {
        ws.send(JSON.stringify({ letter: e.key }));
    });
}
async function init() {
    const renderService = renderer();
    renderService.init();
    await startSocket(renderService);
}
await init();
export {};
