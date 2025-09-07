import type { Config, HangmanResponse, RenderService } from "./types";

function setCookie(name: string, value: any, days=1) {
    const expires = new Date(Date.now() + days * 24 * 60 * 60 * 1000).toUTCString()
    document.cookie = `${name}=${encodeURIComponent(value)}; expires=${expires}; path=/`;
}

function getCookie(name: string) {
    const matches = document.cookie.match(new RegExp(
        `(?:^|; )${name.replace(/([.$?*|{}()[\]\\/+^])/g, '\\$1')}=([^;]*)`
    ));
    return matches ? decodeURIComponent(matches[1]!) : undefined;
}

function renderer() {

    const FONT = "30px Arial"
    const COLOUR = "blue"

    let canvas: HTMLCanvasElement
    let ctx: CanvasRenderingContext2D
    const image = new Image()
    image.src = "https://hangman.cropie.online/avatars/rp.jpg"
    
    function init() {
        const canvasEl = document.getElementById("renderCanvas")
        if (!(canvasEl instanceof HTMLCanvasElement)) throw new Error("no canvas")
        canvas = canvasEl

        const ctxEl = canvas.getContext("2d")
        if (!ctxEl) throw new Error("2D context not supported")
        ctx = ctxEl

        ctx.font = FONT
        ctx.fillStyle = COLOUR
    }

    function render(guessState: string, username: string) {
        ctx.clearRect(0, 0, canvas.width, canvas.height)
        ctx.fillText(username, 30, 30)
        ctx.fillText(guessState, 50, 100)
        ctx.drawImage(image, 0, 30, 30, 30)
    }

    return { init, render }

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

async function startSocket(renderService: RenderService): Promise<void> {

    const config = await loadConfig()


    const userdata = getCookie("user")
    if (!userdata) throw new Error("No user data")

    const username = JSON.parse(userdata).username

    const ws = new WebSocket(config.WS_HOST)

    ws.onopen = () => {
        ws.send(JSON.stringify({action: "join", username }))
    }

    ws.onmessage = (event) => {

        const response: HangmanResponse = parseResponse(event.data)

        const { meta, gameState } = response

        renderService.render(gameState.guessState, meta.username)

        if (gameState.isWin) {
            alert("You win!!")
            ws.send(JSON.stringify({action: "reset"}))
        }
    }
    
    document.addEventListener("keypress", (e) => {
        ws.send(JSON.stringify({action: "play", letter: e.key}));
    })
}

async function isTokenValid(token: string) {

    try {
        const response = await fetch("http://localhost:8000/api/authenticate", {
                method: "GET",
                headers: {
                    Authorization: `Bearer ${token}`
                }
        })

        let json;
        try {
            json = await response.json()
        } catch {
            json = null
            // data might be empty or not json
        }

        if (!response.ok) {
            console.error(json?.error ?? "something went wrong...")
            return false
        }

        if (!json?.data) {
            console.error("Response did not contain data object")
            return false
        }

        setCookie("user", JSON.stringify(json.data))

        return true

    } catch (error) {
        console.error("Network error: request failed")
        return false
    }

}

async function init() {

    const token = getCookie('token')

    if (!token) {
        console.log("no token")
        window.location.href = "http://localhost:1234";
        return;
    }

    if (!await isTokenValid(token)) {
        console.log("not valid token")
        window.location.href = "http://localhost:1234";
        return;
    }


    const renderService = renderer()
    renderService.init()

    await startSocket(renderService)
}

onload = init