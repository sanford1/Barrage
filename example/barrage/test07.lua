theta = 0

function final()
   if (getTurn() == 60) then
      -- vanish()
      kill()
   end
end

function blue()
   turn = getTurn()
   if (turn == 30) then
      currentDirection = getDirection()
      launch(currentDirection - 110, 2, final)

      setFunction(final)
   end
end

function main()
   setPosition(320, 240)
   turn = getTurn()

   if (math.fmod(turn, 6) == 0) then
      for i = 0, 11 do
         launch(theta + i * 30, 2, blue)
         theta = theta + 0.1
      end
   end
end