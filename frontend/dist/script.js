// function setCookie(name: string, value: any, days=1) {
//     const expires = new Date(Date.now() + days * 24 * 60 * 60 * 1000).toUTCString()
//     document.cookie = `${name}=${encodeURIComponent(value)}; expires=${expires}; path=/`;
// }
function CanvasRenderService() {
    let FONT = "48px 'Excalifont'";
    const COLOUR = "white";
    let hangmanSprites = new Image();
    let canvas;
    let ctx;
    const image = new Image();
    image.src = "https://hangman.cropie.online/avatars/rp.jpg";
    async function init() {
        await document.fonts.load("48px 'Excalifont'");
        hangmanSprites.src = "/hangman/images/hangman-sprites.png";
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
    function render(guessState, misses, username) {
        ctx.clearRect(0, 0, canvas.width, canvas.height);
        ctx.fillText(guessState.toUpperCase(), 50, 100);
        // ctx.fillText(username, 30, 30)
        // ctx.drawImage(image, 0, 30, 30, 30)
        // sprite sheet has image at position 0, only render when misses is >= 1
        if (misses)
            drawHangman(misses - 1);
    }
    function drawHangman(misses) {
        const height = 200;
        const width = 233;
        ctx.drawImage(hangmanSprites, misses * width, 0, width, height, 0, 200, width, height);
    }
    return { init, render };
}
function MessageRenderService() {
    let ul;
    function init() {
        const ulEl = document.getElementById("messageList");
        if (!(ulEl instanceof HTMLUListElement))
            throw new Error("no message ul");
        ul = ulEl;
    }
    function appendMessage(sender, message) {
        const li = document.createElement("li");
        li.classList.add(sender === "system" ? "text-system" : "text-player");
        const span = document.createElement("span");
        span.textContent = "[" + sender + "]: ";
        li.appendChild(span);
        li.appendChild(document.createTextNode(message));
        ul.appendChild(li);
    }
    function renderChatHistory(chatHistory) {
        ul.innerHTML = '';
        for (const chat of chatHistory) {
            appendMessage(chat.sender, chat.message);
        }
    }
    return { init, appendMessage, renderChatHistory };
}
function GameController(canvasRenderService, messageRenderService) {
    let ws;
    let chatMode = false;
    let disableGame = false;
    let username = '';
    let userInput;
    let resetButton;
    function init(userData, hostUrl) {
        if (!userData)
            throw new Error("No user data");
        const inputEl = document.getElementById("userInput");
        if (!(inputEl instanceof HTMLInputElement))
            throw new Error("no input");
        userInput = inputEl;
        const buttonEl = document.getElementById("resetButton");
        if (!(buttonEl instanceof HTMLButtonElement))
            throw new Error("no button");
        resetButton = buttonEl;
        username = JSON.parse(userData).username ?? "unknown";
        ws = new WebSocket(hostUrl);
        ws.onopen = () => {
            ws.send(JSON.stringify({ action: "join", username }));
        };
        ws.onmessage = (event) => {
            const response = parseResponse(event.data);
            if (response.type === "gameState") {
                handleGameState(response);
                return;
            }
            if (response.type === "message") {
                handleMessage(response);
                return;
            }
        };
        /*
            Basic idea: press enter to toggle chatMode
                If in chatMode a key is pressed, and that key is enter: send message, chatInput.blur()
                Otherwise: return
    
                If not in chatmode
                    if key is Enter
                        enter chatmode, chatInput.focus(), return
                    if key is a letter
                        send letter
        */
        document.addEventListener("keypress", (e) => {
            if (e.key == "Enter" && !chatMode) {
                chatMode = true;
                userInput.disabled = false;
                userInput.focus();
                return;
            }
            if (e.key == "Enter" && chatMode) {
                chatMode = false;
                userInput.blur();
                userInput.disabled = true;
                if (!userInput.value)
                    return;
                ws.send(JSON.stringify({ action: "chat", chat: userInput.value, username }));
                userInput.value = '';
                return;
            }
            if (!chatMode && !disableGame) {
                ws.send(JSON.stringify({ action: "play", letter: e.key }));
                return;
            }
        });
        resetButton.addEventListener("click", () => handleReset());
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
    function handleGameState(response) {
        const { meta, gameState } = response;
        canvasRenderService.render(gameState.guessState, gameState.misses, meta.username);
        if (gameState.isWin) {
            disableGame = true;
            resetButton.disabled = false;
        }
        if (gameState.isLose) {
            disableGame = true;
            resetButton.disabled = false;
        }
    }
    function handleMessage(response) {
        const { chatHistory } = response;
        console.log(chatHistory);
        messageRenderService.renderChatHistory(chatHistory);
    }
    function handleReset() {
        resetButton.disabled = true;
        disableGame = false;
        ws.send(JSON.stringify({ action: "reset" }));
    }
    return { init };
}
function AuthService() {
    let lobbyUrl = '';
    let authBaseUrl = '';
    let userData = '';
    async function validateToken() {
        try {
            const response = await fetch(`${authBaseUrl}/api/authenticate`, {
                method: "GET",
                credentials: 'include'
            });
            let json;
            // just handing the situation where data might be empty or not json
            try {
                json = await response.json();
            }
            catch {
                json = null;
            }
            if (!response.ok) {
                console.error(json?.error ?? "something went wrong...");
                return false;
            }
            if (!json?.data) {
                console.error("Response did not contain data object");
                return false;
            }
            userData = JSON.stringify(json.data);
            return true;
        }
        catch (error) {
            console.error("Network error: request failed");
            return false;
        }
    }
    function init({ LOBBY_URL, AUTH_BASE_URL }) {
        lobbyUrl = LOBBY_URL;
        authBaseUrl = AUTH_BASE_URL;
    }
    function sendToReturnUrl() {
        window.location.href = lobbyUrl;
    }
    function getUserData() {
        return userData;
    }
    return { init, sendToReturnUrl, validateToken, getUserData };
}
async function loadConfig() {
    const response = await fetch("dist/config.json");
    return response.json();
}
async function init() {
    const config = await loadConfig();
    const authService = AuthService();
    authService.init({ LOBBY_URL: config.LOBBY_URL, AUTH_BASE_URL: config.AUTH_BASE_URL });
    if (!await authService.validateToken()) {
        authService.sendToReturnUrl;
        return;
    }
    const canvasRenderService = CanvasRenderService();
    await canvasRenderService.init();
    const messageRenderService = MessageRenderService();
    messageRenderService.init();
    const gameController = GameController(canvasRenderService, messageRenderService);
    gameController.init(authService.getUserData(), config.WS_HOST);
}
onload = init;
export {};
