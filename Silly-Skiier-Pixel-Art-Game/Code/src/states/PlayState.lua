PlayState = Class{__includes = BaseState}

function PlayState:enter(params)
    self.trails = params['trails']
end

function PlayState:init()
    self.skiman = Skiier()
    self.timer = 0
    self.obstacles = {}
    self.trails = {}
    self.prevx = 0
    self.currx = 0
    self.dy = -200 * TIME_ELAPSED / 90 - 150
    self.distance = 0
end

function PlayState:update(dt)
    TIME_ELAPSED = TIME_ELAPSED + dt
    self.dy = -100 * TIME_ELAPSED / 90 - 150
    table.insert(self.trails, Trail(self.skiman.x, self.skiman.y))
    self.skiman:update(dt)
    self.timer = self.timer + dt
    if self.timer > 0 then
        self.timer = self.timer - math.random()/3 - 1.5 + TIME_ELAPSED/120
        for i=1,math.random(2,4) do
            self.currx = math.random(-30,350)
            if math.abs(self.currx - self.prevx) < 120 then
                self.currx = (i % 2 == 1 and self.prevx + 120 or self.prevx -120)
            end
            table.insert(self.obstacles, Obstacle(self.currx,(i*25) - math.random(25)))
            self.prevx = self.currx
        end
    end
    for k, trail in pairs(self.trails) do
        trail:update(dt, self.dy)
    end
    for k, obstacle in pairs(self.obstacles) do
        obstacle:update(dt, self.dy)
        self.skiman:collides(obstacle.x + 22 ,obstacle.y + 74)
    end

    self.distance = self.distance + self.dy * dt

    for k, obstacle in pairs(self.obstacles) do
        if obstacle.delete then
            table.remove(self.obstacles, k)
        end
    end

    for k, trail in pairs(self.trails) do
        if trail.delete then
            table.remove(self.trails, k)
        end
    end

    if self.skiman.collided then
        TIME_ELAPSED = 0.000001
        sounds['hit']:play()
        gStateMachine:change('score', {distance = self.distance})
    end

end

function PlayState:render()
    for k, trail in pairs(self.trails) do
        trail:render()
    end
    for k, obstacle in pairs(self.obstacles) do
        if obstacle.y < -50 then
            obstacle:render()
        end
    end
    self.skiman:render()
    for k, obstacle in pairs(self.obstacles) do
        if obstacle.y >= -50 then
            obstacle:render()
        end
    end
end