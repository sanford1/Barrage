local barrage = require "barrageC"

local bulletImg = nil
local b = nil

function love.load(arg)
   bulletImg = love.graphics.newImage('assets/bullet.png')
   barrage.setRngSeed()
   b = barrage.new(arg[2], 320.0, 120.0)
end

function love.update(dt)
   local x, y = love.mouse.getPosition()
   b:setPlayerPosition(x, y)

   b:tick()
end

function love.draw(dt)
   while b:hasNext() do
      local x, y = b:yield()
      love.graphics.draw(bulletImg, x, y, 0, 0.5, 0.5)
   end
end
