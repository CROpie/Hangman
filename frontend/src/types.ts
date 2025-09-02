export type Config = {
    WS_HOST: string
}

type Meta = {
    username: string
}

type GameState = {
    isWin: boolean
    guessState: string
}

export type HangmanResponse = {
    meta: Meta
    gameState: GameState
}

export interface RenderService {
    init: () => void
    render: (guessState: string, username: string) => void
}