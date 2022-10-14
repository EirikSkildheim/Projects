Obstacle = Class{}

love.graphics.setDefaultFilter('nearest', 'nearest')

local images = {
    love.graphics.newImage('graphics/obstacles/tree1.png'),
    love.graphics.newImage('graphics/obstacles/tree2.png'),
    love.graphics.newImage('graphics/obstacles/tree3.png'),
    love.graphics.newImage('graphics/obstacles/tree4.png'),
    love.graphics.newImage('graphics/obstacles/tree5.png'),
    love.graphics.newImage('graphics/obstacles/tree6.png'),
    love.graphics.newImage('graphics/obstacles/tree7.png'),
    love.graphics.newImage('graphics/obstacles/tree8.png'),
    love.graphics.newImage('graphics/obstacles/tree9.png'),
    love.graphics.newImage('graphics/obstacles/tree10.png'),
    love.graphics.newImage('graphics/obstacles/tree11.png'),
    love.graphics.newImage('graphics/obstacles/tree12.png'),
    love.graphics.newImage('graphics/obstacles/tree13.png'),
    love.graphics.newImage('graphics/obstacles/tree14.png'),
    love.graphics.newImage('graphics/obstacles/tree15.png'),
    love.graphics.newImage('graphics/obstacles/rock1.png'),
    love.graphics.newImage('graphics/obstacles/rock2.png'),
    love.graphics.newImage('graphics/obstacles/snowman.png')
}

function Obstacle:init(x,y)
    self.image = images[math.random(18)]
    self.x = x
    self.y = 180 + y
    self.dy = 0
    self.delete = false
end

function Obstacle:update(dt, dy)
    self.dy = dy
    self.y = self.y + self.dy * dt
    if self.y + 90 < 0 then
        self.delete = true
    end
end

function Obstacle:render()
    love.graphics.draw(self.image, self.x, self.y)
end