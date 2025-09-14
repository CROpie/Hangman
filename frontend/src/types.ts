export type Config = {
    WS_HOST: string,
    LOBBY_URL: string,
    AUTH_BASE_URL: string
    IMAGE_PATH: string
}

type Meta = {
    username: string
}

type GameState = {
    isWin: boolean
    isLose: boolean
    guessState: string
    misses: number
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
    init: ({IMAGE_PATH}: {IMAGE_PATH: string}) => void
    render: (guessState: string, misses: number, username: string) => void
}

export interface MessageRenderService {
    init: () => void
    renderChatHistory: (chatHistory: Chat[]) => void
}