function init() {
    const ws = new WebSocket("wss://hangman.cropie.online");
    ws.onmessage = (event) => {
        console.log(event);
    };
    document.addEventListener("keypress", (e) => {
        ws.send(e.key);
    });
}
init();
export {};
