export type Config = {
    WS_HOST: string,
    LOBBY_URL: string,
    AUTH_BASE_URL: string
}

type Meta = {
    username: string
}

type GameState = {
    isWin: boolean
    guessState: string
}

export type GameStateResponse = {
    type: "gameState"
    meta: Meta
    gameState: GameState
}

export type Chat = {
    message: string
    sender: string
}

export type MessageResponse = {
    type: "message"
    chatHistory: Chat[]
}

export type HangmanResponse = GameStateResponse | MessageResponse

export interface CanvasRenderService {
    init: () => void
    render: (guessState: string, username: string) => void
}

export interface MessageRenderService {
    init: () => void
    renderChatHistory: (chatHistory: Chat[]) => void
}