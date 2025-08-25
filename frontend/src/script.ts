type Config = {
    WS_HOST: string
}

function parseResponse(event: any) {
    if (typeof event != "string") throw new Error(`Event was not a string: ${event}`)

    try {
        return JSON.parse(event)
    } catch (error: any) {
        throw new Error(`Input event could not be parsed: ${error.message}`)
    }
}

async function loadConfig(): Promise<Config> {
    const response = await fetch("dist/config.json");
    console.log(response)
    return response.json();
}

async function startSocket(): Promise<void> {

    const config = await loadConfig()

    const ws = new WebSocket(config.WS_HOST)

    ws.onmessage = (event) => {

        const response = parseResponse(event.data)
        console.log(response)

    }
    
    document.addEventListener("keypress", (e) => {
        ws.send(e.key);
    })
}

async function init() {
    await startSocket()
}

await init()