push = require 'lib/push'
Class = require 'lib/class'

require 'src/constants'
require 'src/StateMachine'
require 'src/Skiier'
require 'src/Obstacle'
require 'src/Trail'

require 'src/states/BaseState'
require 'src/states/TitleScreenState'
require 'src/states/AnimationState'
require 'src/states/PlayState'
require 'src/states/ScoreState'

gStateMachine = StateMachine {
    ['title'] = function() return TitleScreenState() end,
    ['animation'] = function() return AnimationState() end,
    ['play'] = function() return PlayState() end,
    ['score'] = function() return ScoreState() end
}

sounds = {
    ['turn'] = love.audio.newSource('audio/turn.wav', 'static'),
    ['hit'] = love.audio.newSource('audio/hit.wav', 'static'),
    ['ready'] = love.audio.newSource('audio/ready.wav', 'static'),
    ['go'] = love.audio.newSource('audio/go.wav', 'static'),

    ['music'] = love.audio.newSource('audio/music.wav', 'static')
}

gFonts = {
    ['small'] = love.graphics.newFont('fonts/RetroGaming.ttf', 8),
    ['medium'] = love.graphics.newFont('fonts/RetroGaming.ttf', 16),
    ['large'] = love.graphics.newFont('fonts/RetroGaming.ttf', 32)
}

love.keyboard.keysPressed = {}
love.mouse.buttonsPressed = {}