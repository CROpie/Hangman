// function setCookie(name: string, value: any, days=1) {
//     const expires = new Date(Date.now() + days * 24 * 60 * 60 * 1000).toUTCString()
//     document.cookie = `${name}=${encodeURIComponent(value)}; expires=${expires}; path=/`;
// }
function CanvasRenderService() {
    const FONT = "30px Arial";
    const COLOUR = "blue";
    let canvas;
    let ctx;
    const image = new Image();
    image.src = "https://hangman.cropie.online/avatars/rp.jpg";
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
    function render(guessState, username) {
        ctx.clearRect(0, 0, canvas.width, canvas.height);
        ctx.fillText(username, 30, 30);
        ctx.fillText(guessState, 50, 100);
        ctx.drawImage(image, 0, 30, 30, 30);
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
        if (sender === "system") {
            li.textContent = `--- ${message} ---`;
        }
        else {
            li.textContent = `${sender}: ${message}`;
        }
        ul.appendChild(li);
    }
    function renderChatHistory(chatHistory) {
        ul.innerHTML = '';
        for (const chat of chatHistory) {
            console.log(chat);
            appendMessage(chat.sender, chat.message);
        }
    }
    return { init, appendMessage, renderChatHistory };
}
function GameController(canvasRenderService, messageRenderService) {
    let ws;
    let chatMode = false;
    let isWin = false;
    let username = '';
    let userInput;
    let resetButton;
    function init(userData, hostUrl) {
        // const userdata = getCookie("user")
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
            if (!chatMode && !isWin) {
                console.log({ isWin, chatMode });
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
        canvasRenderService.render(gameState.guessState, meta.username);
        if (gameState.isWin) {
            isWin = true;
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
        isWin = false;
        ws.send(JSON.stringify({ action: "reset" }));
    }
    return { init };
}
function AuthService() {
    let token = '';
    let returnUrl = '';
    let authenticationUrl = '';
    let userData = '';
    async function validateTokenExtractUserdata() {
        try {
            const response = await fetch(authenticationUrl, {
                method: "GET",
                headers: {
                    Authorization: `Bearer ${token}`
                }
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
            // setCookie("user", JSON.stringify(json.data))
            return true;
        }
        catch (error) {
            console.error("Network error: request failed");
            return false;
        }
    }
    function init({ retUrl, authUrl }) {
        returnUrl = retUrl;
        authenticationUrl = authUrl;
        token = getCookie('token');
        if (!token) {
            console.error("cookie key 'token' has no value");
            sendToReturnUrl();
            return false;
        }
        return true;
    }
    function getCookie(name) {
        const matches = document.cookie.match(new RegExp(`(?:^|; )${name.replace(/([.$?*|{}()[\]\\/+^])/g, '\\$1')}=([^;]*)`));
        return matches ? decodeURIComponent(matches[1]) : "";
    }
    function sendToReturnUrl() {
        window.location.href = returnUrl;
    }
    function getUserData() {
        return userData;
    }
    return { init, sendToReturnUrl, validateTokenExtractUserdata, getUserData };
}
async function loadConfig() {
    const response = await fetch("dist/config.json");
    return response.json();
}
async function init() {
    const config = await loadConfig();
    const authService = AuthService();
    if (!authService.init({ retUrl: config.LOGIN_URL, authUrl: config.AUTH_URL })) {
        authService.sendToReturnUrl;
        return;
    }
    if (!await authService.validateTokenExtractUserdata()) {
        authService.sendToReturnUrl;
        return;
    }
    const canvasRenderService = CanvasRenderService();
    canvasRenderService.init();
    const messageRenderService = MessageRenderService();
    messageRenderService.init();
    const gameController = GameController(canvasRenderService, messageRenderService);
    gameController.init(authService.getUserData(), config.WS_HOST);
}
onload = init;
export {};
