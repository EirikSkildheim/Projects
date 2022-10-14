ScoreState = Class{__includes = BaseState}

love.graphics.setDefaultFilter('nearest', 'nearest')
local images = {
    love.graphics.newImage('graphics/medals/noMedal.png'),
    love.graphics.newImage('graphics/medals/bronzeMedal.png'),
    love.graphics.newImage('graphics/medals/silverMedal.png'),
    love.graphics.newImage('graphics/medals/goldMedal.png'),
    love.graphics.newImage('graphics/medals/platinumMedal.png')
}

function ScoreState:enter(params)
    self.distance = math.floor(params.distance * -1 / 8)
    if self.distance < 250 then
        self.medal = images[1]
    elseif self.distance < 500 then
        self.medal = images[2]
    elseif self.distance < 750 then
        self.medal = images[3]
    elseif self.distance < 1000 then
        self.medal = images[4]
    else
        self.medal = images[5]
    end
end

function ScoreState:init()
    self.y = 130
    self.dy = -10
    self.dy2 = 20
    love.graphics.setDefaultFilter('nearest', 'nearest')
end

function ScoreState:update(dt)
    if love.keyboard.wasPressed('enter') or love.keyboard.wasPressed('return') then
        sounds['music']:setLooping(true)
        sounds['music']:play()
        gStateMachine:change('title')
    end

    self.y = self.y + self.dy * dt
    self.dy = self.dy + self.dy2 * dt
    if self.dy2 > 0 then
        if self.y > 130 then
            self.dy2 = -20
            self.dy = 10
        end
    else
        if self.y < 130 then
            self.dy2 = 20
            self.dy = -10
        end
    end
end

function ScoreState:render()
    love.graphics.setFont(gFonts['medium'])

    love.graphics.setColor(0,0,0,255)
    love.graphics.printf(tostring(self.distance) .. ' FEET', -2, 40, VIRTUAL_WIDTH, 'center')
    love.graphics.printf(tostring(self.distance) .. ' FEET', 2, 40, VIRTUAL_WIDTH, 'center')
    love.graphics.printf(tostring(self.distance) .. ' FEET', 0, 38, VIRTUAL_WIDTH, 'center')
    love.graphics.printf(tostring(self.distance) .. ' FEET', 0, 42, VIRTUAL_WIDTH, 'center')

    love.graphics.setFont(gFonts['small'])
    love.graphics.printf('Press Enter or Return', -1, self.y, VIRTUAL_WIDTH, 'center')
    love.graphics.printf('Press Enter or Return', 1, self.y, VIRTUAL_WIDTH, 'center')
    love.graphics.printf('Press Enter or Return', 0, self.y - 1, VIRTUAL_WIDTH, 'center')
    love.graphics.printf('Press Enter or Return', 0, self.y + 1, VIRTUAL_WIDTH, 'center')

    love.graphics.setColor(255,255,255,255)

    love.graphics.setFont(gFonts['medium'])
    love.graphics.printf(tostring(self.distance) .. ' FEET', 0, 40, VIRTUAL_WIDTH, 'center')

    love.graphics.setFont(gFonts['small'])
    love.graphics.printf('Press Enter or Return', 0, self.y, VIRTUAL_WIDTH, 'center')

    love.graphics.draw(self.medal,VIRTUAL_WIDTH/2 - 25, 68)
end