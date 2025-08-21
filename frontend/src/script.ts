function init(): void {

    const ws = new WebSocket("ws://localhost:9002")

    ws.onmessage = (event) => {
        console.log(event)
    }
    
    document.addEventListener("keypress", (e) => {
        ws.send(e.key);
    })
}

init()