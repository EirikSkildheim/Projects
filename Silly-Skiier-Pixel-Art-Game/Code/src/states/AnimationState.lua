AnimationState = Class{__includes = BaseState}

love.graphics.setDefaultFilter('nearest', 'nearest')
local images = {
    love.graphics.newImage('graphics/skyBackground.png'),
    love.graphics.newImage('graphics/mountainBackground.png'),
    love.graphics.newImage('graphics/cloud1.png'),
    love.graphics.newImage('graphics/cloud2.png'),
    love.graphics.newImage('graphics/turns/0turn.png')
}

function AnimationState:enter(params)
    self.cloud1x = params['x1']
    self.cloud2x = params['x2']
end

function AnimationState:init()
    self.y = 0
    self.dy = -150
    self.skiiery = 50
    self.skiierdy = -40 / 1.8
    self.skyBackground = images[1]
    self.mountainBackground = images[2]
    self.cloud1 = images[3]
    self.cloud2 = images[4]
    self.cloudScroll = 20
    self.skiier = images[5]
    self.timer = 0
    self.countdown = 3
    self.trails = {}
end

function AnimationState:update(dt)
    self.cloud1x = (self.cloud1x + self.cloudScroll * dt) % 420
    self.cloud2x = (self.cloud2x + self.cloudScroll * dt) % 420
    self.y = self.y + self.dy * dt
    table.insert(self.trails, Trail(VIRTUAL_WIDTH/2 - 15, self.skiiery))
    for k, trail in pairs(self.trails) do
        trail:update(dt, self.dy)
    end
    self.skiiery = self.skiiery + self.skiierdy * dt
    self.timer = self.timer + dt
    if self.timer > 0.6 then
        self.timer = self.timer - 0.6
        self.countdown = self.countdown - 1
        if self.countdown ~= 0 then
            sounds['ready']:play()
        end
    end
    if self.countdown == 0 then
        sounds['go']:play()
        gStateMachine:change('play', {
            ['trails'] = self.trails
        })
    end
end

function AnimationState:render()
    love.graphics.draw(self.skyBackground, 0, self.y)
    love.graphics.draw(self.cloud1, self.cloud1x - 100, self.y + 15)
    love.graphics.draw(self.cloud2, self.cloud2x - 100, self.y + 25)
    love.graphics.draw(self.mountainBackground, 0, self.y)
    for k, trail in pairs(self.trails) do
        trail:render()
    end
    love.graphics.draw(self.skiier, VIRTUAL_WIDTH/2 - 15, self.skiiery)

    love.graphics.setFont(gFonts['large'])
    love.graphics.setColor(0,0,0,255)
    love.graphics.printf(self.countdown, -2, 60, VIRTUAL_WIDTH, 'center')
    love.graphics.printf(self.countdown, 2, 60, VIRTUAL_WIDTH, 'center')
    love.graphics.printf(self.countdown, 0, 58, VIRTUAL_WIDTH, 'center')
    love.graphics.printf(self.countdown, 0, 62, VIRTUAL_WIDTH, 'center')
    love.graphics.setColor(255,255,255,255)
    love.graphics.printf(self.countdown, 0, 60, VIRTUAL_WIDTH, 'center')
end