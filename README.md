# Barrage [![Build Status](https://travis-ci.org/sanford1/Barrage.svg?branch=master)](https://travis-ci.org/sanford1/Barrage)

Lua scripting for shmup barrage patterns. Rewrite of the C++ version ([BulletLua](https://github.com/sanford1/BulletLua)).

> This C project aims to create a sane scriptable interface to define bullet patterns in shoot 'em
> up games. These patterns, while difficult to dodge in game and hard to appreciate while in the
> heat of the moment, can be beautiful to spectate.

## Building this project

Dependencies: python, [ninja](https://martine.github.io/ninja/), [lua](http://www.lua.org/) (or [luajit](http://luajit.org/))

### Grab this repository + submodules

    git clone <this-repo>
    git submodule update --init --recursive

### Generate the ninja build file

    python bootstrap.py (--debug) (--cxx=<compiler>) (--ci) (--luajit)

Arguments in parenthesis are optional. The default compiler is [gcc](https://gcc.gnu.org/) and this project assumes [C11](https://en.wikipedia.org/wiki/C11_(C_standard_revision)) support. The `ci` switch links `lua5.2` instead of `lua` if your system does not default to lua 5.2. The `luajit` switch links luajit in place of lua.

The bootstrap script is python2/3 compatible.

### Build the library and unit test

Simply run `ninja`.

## Usage

Currently you can link the C library or use Lua wrapper (named `barrageC`). More wrappers once this project is more stable.

### C

#### Setup

Link libbarrage.so and make sure your compiler can find the correct header files in the `include` directory.

#### Example (pseudo-)code

    #include <barrage/Barrage.h>
    #include <barrage/SpacialPartition.h>

    struct Barrage barrage;

    // Passing NULL to createBarrage will return a pointer to a heap-allocated barrage.
    br_createBarrage(&barrage);

    // Optional collision detection manager.
    struct SpacialPartition sp;
    br_createSpacialPartition(&sp)

    // Add a new collision model (currently only different sized rectangles)
    int newModelIndex = br_addModel(&sp, (struct Rect){0, 0, 100, 100});

    // Launch a bullet with the default collision hitbox (0)
    br_createBulletFromScript(&barrage, script, 320.0f, 120.0f, 0);

    while (running)
    {
        // Update
        br_tick(&barrage, &sp);

        // or br_tick(&barrage, NULL) if you don't want to register this barrage's bullets to
        // the collision detection manager.

        // Check collision between registered bullets and the rect defined by (x, y, w, h)
        if (br_checkCollision(&sp, x, y, w, h)
        {
            // You were hit!
        }

        // Draw loop
        while (br_hasNext(&barrage))
        {
            struct Bullet* b = br_yield(&barrage);
            drawImage(b->x, b->y);
        }
    }

    // Cleanup, Pass false as we didn't allocate data on heap.
    br_deleteBarrage(barrage, false);
    br_deleteSpacialPartition(sp, false);

### Lua

#### Importing the shared library

First things first is that lua needs to be able to find libbarrage.so. Whether that means installing the shared library to a system directory or telling lua where it can find the file is up to you. For example, when running your script you can direct the lua executable to find the shared library like this:

    LUA_CPATH="../lib/libbarrage.so" lua myFile.lua

Keep in mind the lua executable version should match the lua version linked to build this project. This matters if you are using something like [LÖVE](https://love2d.org/), which uses luajit internally. To use Barrage with LÖVE you have to build libbarrage.so with luajit instead of lua. The module is named `barrageC`.

#### Example (pseudo-)code:

    -- Allow lua to access the exposed API in libbarrage.so.
    local barrage = require "barrageC"

    local myBarrage = nil
    local spacialPartition = nil

    function load(arg)
        -- Load a barrage script from a file.
        myBarrage = barrage.newBarrage()

        -- Create collision detection manager.
        spacialPartition = barrage.newSpacialPartition()

        -- Add a new collision model (currently only different sized rectangles)
        local newModelIndex = barrage:addModel(100, 100);

        -- Create a bullet from a script file with the default collision hitbox.
        myBarrage:launchFile("path/to/file.lua", 320.0, 120.0, 0)
    end

    function update(dt)
        local x, y = getMousePosition()
        myBarrage:setPlayerPosition(x, y)

        -- Update barrage bullets.
        myBarrage:tick(spacialPartition)

        -- Check collision between registered bullets and the rect defined by (x, y, w, h)
        if (spacialPartition:checkCollision(x, y, w, h)) then
            -- Do something cool. Explode! Order a pizza!
        end
    end

    function draw(dt)
        while myBarrage:hasNext() do
            -- yield will give us plenty of variables to play with.
            --    x, y is the position of the center of this bullet.
            --    vx, vy is the velocity of the this bullet.
            --    `alpha` is a range [0.0, 1.0] describing the current state of this bullet.
            --        0.0 = Dead, 1.0 = Alive, (0.0, 1.0) = Dying (vanishing, no hitbox)
            --    `model` is the model index of this bullet.
            local x, y, vx, vy, alpha, model = myBarrage:yield()

            -- Draw our bullet.
            -- Keep in mind the coordinate we are given from yield is defined
            -- to be the center of the bullet.
            setDrawColor(255, 255, 255, alpha)
            drawImage(x, y)
        end
    end

A more involved example can be found [here](https://github.com/sanford1/flaming-octo-avenger)

## Barrage Scripting

### Basics

A bare-bones barrage script is structured like a lua module:

    myBarrage = {
        -- Optional
        onLoad = function ()
            -- Do some things
        end
        main = function ()
            -- Do more things
        end
    }
    return myBarrage

When you create a barrage, it will immediately evaluate the file and run the `onLoad` function in your table if it exists. Next, it will create a bullet and associate the `main` function with it. What this means is that on each frame (tick), `main` will be run. Some documentation for bullet functions can be found below and a set of example scripts can be found in `example/barrage`

### Available bullet functions

#### Position functions

    -- Set Bullet Position.
    setPosition(float x, float y)

    -- Get Position.
    x, y = getPosition()

    -- Get Target Position
    tx, ty = getTargetPosition()

#### Velocity functions

    -- Set Bullet Velocity.
    setVelocity(float vx, float vy)

    -- Get Velocity Components.
    vx, vy = getVelocity()

    -- Bullet Direction
    setDirection(float dir)
    setDirectionRelative(float dir)
    getDirection()

    -- Bullet Speed
    setSpeed(float s)
    setSpeedRelative(float s)
    getSpeed()

    -- Set the current bullet to aim at the "player"
    aimAtTarget()

    -- Set the current bullet to aim at a point
    aimAtPoint(float x, float y)

    -- Set speed and direction of bullet such that it will reach (x, y) in 'n' steps (ticks).
    linearInterpolate(float x, float y, unsigned int n)

#### Frame counting

    -- Get the amount of frames since this bullet's creation or most recent function switch.
    getFrameCount()

    -- Reset this bullet's frame counter
    resetFrameCount()

#### Barrage difficulty

    -- Get the current barrage "difficulty", From [0.0, 1.0].
    getRank()

#### Switch functions + creating new bullets

    -- Switch current running function. This also resets the bullet's frame counter.
    setFunction(function)

    -- Shoot a bullet from current bullet's position moving in direction (d) at speed (s) running function (func).

    launch(int modelIndex, float d, float s, func)

    -- Shoot a bullet and aim it at the "player"
    launchAtTarget(int modelIndex, float s, func)

    -- Shoot (segments) number of bullets concentrically.
    launchCircle(int modelIndex, int segments, float s, func)

#### Removing Bullets

    -- Fade out this bullet. Kill it slowly (over n frames, n = 30 by default).
    vanish(int n)

    -- Immediately destroy this bullet.
    kill()

#### Getting Bullet State

    -- Returns true if bullet is dead.
    dead = isDead()

    -- Returns true if bullet is dying. Dying = not dead AND not alive.
    -- Dead bullets have no hitbox and have been marked to die in the future.
    dying = isDying()

#### Setting Bullet collision models

    -- A `model` is simply an integer attached to a bullet denoting which collision rectangle
    -- to use during the collision detection phase. This may also be used by a higher level
    -- program to decide, for example, how to draw this bullet.
    setModel(int modelIndex)
    modelIndex = getModel()

#### Passing data to Barrage scripts
    -- Experimental and not fully functional. There exists a storeFloat function within the
    -- Barrage interface which can store a single value for scripts to use and this function
    -- can retrieve that value.
    value = loadFloat()

#### Do nothing

    Pass `nil` to setFunction or launch*. nullFunc was removed.
