Trail = Class{}

love.graphics.setDefaultFilter('nearest', 'nearest')

local image = love.graphics.newImage('graphics/trail5.png')

function Trail:init(x,y)
    self.image = image
    self.x = x
    self.y = y
    self.dy = 0
    self.delete = false
end

function Trail:update(dt, dy)
    self.dy = dy
    self.y = self.y + self.dy * dt
    if self.y + 44 < 0 then
        self.delete = true
    end
end

function Trail:render()
    love.graphics.draw(self.image, self.x, self.y)
end