TitleScreenState = Class{__includes = BaseState}

love.graphics.setDefaultFilter('nearest', 'nearest')
local images = {
    love.graphics.newImage('graphics/skyBackground.png'),
    love.graphics.newImage('graphics/mountainBackground.png'),
    love.graphics.newImage('graphics/cloud1.png'),
    love.graphics.newImage('graphics/cloud2.png'),
    love.graphics.newImage('graphics/turns/0turn.png')
}

function TitleScreenState:init()
    self.y = 156
    self.dy = -10
    self.dy2 = 20
    self.skyBackground = images[1]
    self.mountainBackground = images[2]
    self.cloud1 = images[3]
    self.cloud2 = images[4]
    self.cloud1x = 100
    self.cloud2x = 330
    self.cloudScroll = 20
    self.skiier = images[5]
end

function TitleScreenState:update(dt)
    if love.keyboard.wasPressed('space') or love.mouse.wasPressed(1) then
        love.audio.stop()
        sounds['ready']:play()
        gStateMachine:change('animation', {
            ['x1'] = self.cloud1x,
            ['x2'] = self.cloud2x
        })
    end
    self.cloud1x = (self.cloud1x + self.cloudScroll * dt) % 420
    self.cloud2x = (self.cloud2x + self.cloudScroll * dt) % 420
    self.y = self.y + self.dy * dt
    self.dy = self.dy + self.dy2 * dt
    if self.dy2 > 0 then
        if self.y > 156 then
            self.dy2 = -20
            self.dy = 10
        end
    else
        if self.y < 156 then
            self.dy2 = 20
            self.dy = -10
        end
    end
end

function TitleScreenState:render()
    love.graphics.draw(self.skyBackground, 0, 0)
    love.graphics.draw(self.cloud1, self.cloud1x - 100, 15)
    love.graphics.draw(self.cloud2, self.cloud2x - 100, 25)
    love.graphics.draw(self.mountainBackground, 0, 0)
    love.graphics.draw(self.skiier, VIRTUAL_WIDTH/2 - 15, 50)

    love.graphics.setFont(gFonts['large'])

    love.graphics.setColor(0,0,0,255)
    love.graphics.printf('SILLY', -2, 0, VIRTUAL_WIDTH, 'center')
    love.graphics.printf('SILLY', 2, 0, VIRTUAL_WIDTH, 'center')
    love.graphics.printf('SILLY', 0, -2, VIRTUAL_WIDTH, 'center')
    love.graphics.printf('SILLY', 0, 2, VIRTUAL_WIDTH, 'center')
    love.graphics.printf('SKIIER', -2, 95, VIRTUAL_WIDTH, 'center')
    love.graphics.printf('SKIIER', 2, 95, VIRTUAL_WIDTH, 'center')
    love.graphics.printf('SKIIER', 0, 93, VIRTUAL_WIDTH, 'center')
    love.graphics.printf('SKIIER', 0, 97, VIRTUAL_WIDTH, 'center')

    love.graphics.setFont(gFonts['small'])
    love.graphics.printf('Press Space or Click', -1, self.y, VIRTUAL_WIDTH, 'center')
    love.graphics.printf('Press Space or Click', 1, self.y, VIRTUAL_WIDTH, 'center')
    love.graphics.printf('Press Space or Click', 0, self.y - 1, VIRTUAL_WIDTH, 'center')
    love.graphics.printf('Press Space or Click', 0, self.y + 1, VIRTUAL_WIDTH, 'center')

    love.graphics.setColor(255,255,255,255)
    love.graphics.setFont(gFonts['large'])

    love.graphics.printf('SILLY', 0, 0, VIRTUAL_WIDTH, 'center')
    love.graphics.printf('SKIIER', 0, 95, VIRTUAL_WIDTH, 'center')

    love.graphics.setFont(gFonts['small'])
    love.graphics.printf('Press Space or Click', 0, self.y, VIRTUAL_WIDTH, 'center')

end