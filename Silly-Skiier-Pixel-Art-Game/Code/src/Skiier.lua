Skiier = Class{}

function Skiier:init()
    self.images = {
        ['0'] = love.graphics.newImage('graphics/turns/0turn.png'),
        ['l1'] = love.graphics.newImage('graphics/turns/l1turn.png'),
        ['l2'] = love.graphics.newImage('graphics/turns/l2turn.png'),
        ['l3'] = love.graphics.newImage('graphics/turns/l3turn.png'),
        ['l4'] = love.graphics.newImage('graphics/turns/l4turn.png'),
        ['r1'] = love.graphics.newImage('graphics/turns/r1turn.png'),
        ['r2'] = love.graphics.newImage('graphics/turns/r2turn.png'),
        ['r3'] = love.graphics.newImage('graphics/turns/r3turn.png'),
        ['r4'] = love.graphics.newImage('graphics/turns/r4turn.png'),
    }

    self.x = VIRTUAL_WIDTH / 2 - 15
    self.y = 10

    self.dx = 0
    self.dx2 = 10

    self.collided = false
end

function Skiier:update(dt)

    self.dx2 = self.dx2 + (self.dx2 < 0 and dt / 90 * -15 or dt / 90 * 15)

    if love.keyboard.wasPressed('space') or love.mouse.wasPressed(1) then
        sounds['turn']:play()
        self.dx2 = self.dx2 * - 1
    end


    self.dx = self.dx + self.dx2 * dt

    self.x = self.x + self.dx

    if self.x < -20 or self.x > 310 then
        self.collided = true
    end
end

function Skiier:render()
    if self.dx < -8 then
        love.graphics.draw(self.images['r4'], self.x, self.y)
    elseif self.dx < -5 then
        love.graphics.draw(self.images['r3'], self.x, self.y)
    elseif self.dx < -2.5 then
        love.graphics.draw(self.images['r2'], self.x, self.y)
    elseif self.dx < -0.65 then
        love.graphics.draw(self.images['r1'], self.x, self.y)
    elseif self.dx < 0.65 then
        love.graphics.draw(self.images['0'], self.x, self.y)
    elseif self.dx < 2.5 then
        love.graphics.draw(self.images['l1'], self.x, self.y)
    elseif self.dx < 5 then
        love.graphics.draw(self.images['l2'], self.x, self.y)
    elseif self.dx < 8 then
        love.graphics.draw(self.images['l3'], self.x, self.y)
    else
        love.graphics.draw(self.images['l4'], self.x, self.y)
    end
end

function Skiier:collides(x,y)
    if self.x + 26 > x and self.x + 4 < x + 16 and self.y + 20 < y + 16 and self.y + 35 > y then
        self.collided = true
    end
end