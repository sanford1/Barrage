#include <barrage/Bullet.h>

#include <stdlib.h>
#include <math.h>
#include <float.h>

#include <barrage/MathUtils.h>

#include <lauxlib.h>

const int DEFAULT_FRAMES_UNTIL_DEATH = 30;

void bl_resetBullet(struct Bullet* b)
{
    bl_setBulletData(b, 0.0f, 0.0f, 0.0f, 0.0f);

    b->model = 0;
    b->frame = 0;
    b->luaFuncRef = LUA_NOREF;
}

void bl_setBullet(struct Bullet* b)
{
    b->next = NULL;
    bl_resetBullet(b);
}

void bl_setBulletData(struct Bullet* b, float x, float y, float vx, float vy)
{
    b->x  = x;
    b->y  = y;
    b->vx = vx;
    b->vy = vy;

    bl_fixSpeed(b);
}

void bl_copyBullet(struct Bullet* to, struct Bullet* from)
{
    to->x  = from->x;
    to->y  = from->y;
    to->vx = from->vx;
    to->vy = from->vy;

    to->model = from->model;
    to->frame = from->frame;

    to->luaFuncRef = from->luaFuncRef;
}

void bl_setNext(struct Bullet* b, struct Bullet* next)
{
    b->next = next;
}

struct Bullet* bl_getNext(struct Bullet* b)
{
    return b->next;
}

void bl_setPosition(struct Bullet* b, float x, float y)
{
    b->x = x;
    b->y = y;
}

void bl_setVelocity(struct Bullet* b, float vx, float vy)
{
    b->vx = vx;
    b->vy = vy;

    bl_fixSpeed(b);
}

void bl_setSpeedAndDirection(struct Bullet* b, float speed, float dir)
{
    b->vx =  speed * sin(dir);
    b->vy = -speed * cos(dir);

    bl_fixSpeed(b);
}

void bl_setSpeed(struct Bullet* b, float speed)
{
    float mag = bl_getSpeed(b);

    b->vx = (b->vx * speed) / mag;
    b->vy = (b->vy * speed) / mag;

    bl_fixSpeed(b);
}

void bl_setSpeedRelative(struct Bullet* b, float speed)
{
    float mag = bl_getSpeed(b);

    b->vx = (b->vx * (speed + mag)) / mag;
    b->vy = (b->vy * (speed + mag)) / mag;

    bl_fixSpeed(b);
}

float bl_getSpeed(struct Bullet* b)
{
    return sqrt(b->vx * b->vx +
                b->vy * b->vy);
}

void bl_setDirection(struct Bullet* b, float dir)
{
    float speed = bl_getSpeed(b);
    b->vx =  speed * sin(dir);
    b->vy = -speed * cos(dir);
}

void bl_setDirectionRelative(struct Bullet* b, float dir)
{
    bl_setDirection(b, dir + bl_getDirection(b));
}

void bl_aimAtPoint(struct Bullet* b, float tx, float ty)
{
    bl_setDirection(b, bl_getAimDirection(b, tx, ty));
}

float bl_getAimDirection(struct Bullet* b, float tx, float ty)
{
    return bl_PI - atan2(tx - b->x, ty - b->y);
}

void bl_linearInterpolate(struct Bullet* b, float tx, float ty, int steps)
{
    bl_setVelocity(b,
                   (tx - b->x) / steps,
                   (ty - b->y) / steps);
}

float bl_getDirection(struct Bullet* b)
{
    return bl_PI - atan2(b->vx, b->vy);
}

void bl_vanish(struct Bullet* b, int framesTilDeath)
{
    // Same logic as bl_kill.
    if (!bl_isDying(b))
        b->frame = -framesTilDeath - 1 - 1;
}

void bl_kill(struct Bullet* b)
{
    // A bullet's frame counter needs to be updated _after_ running its Lua
    // function to keep the mental model of the bullet consistent. For example,
    // when we initially run a bullet, we expect the turn counter to start at 0
    // since technically, no frames have elapsed for that bullet.

    // As such, since the frame counter determines bullet state, when we kill a
    // bullet, we want the frame counter to be one behind the state we want it
    // to be in. I.e. after the function is run, the turn counter is immediately
    // updated and will be flagged as dead (or dying) at the correct time.
    b->frame = DEAD - 1;
}

int bl_isDead(struct Bullet* b)
{
    return b->frame == DEAD;
}

int bl_isDying(struct Bullet* b)
{
    return b->frame < 0;
}

void bl_setModel(struct Bullet* b, int modelIndex)
{
    b->model = modelIndex;
}

int bl_getModel(struct Bullet* b)
{
    return b->model;
}

void bl_resetFrameCount(struct Bullet* b)
{
    b->frame = 0;
}

int bl_getFrameCount(struct Bullet* b)
{
    return b->frame;
}

// void setColor(unsigned char newR, unsigned char newG, unsigned char newB);

void bl_setLuaFunction(struct Bullet* b, int luaFuncRef)
{
    b->luaFuncRef = luaFuncRef;
    b->frame = 0;
}

void bl_update(struct Bullet* b)
{
    b->x += b->vx;
    b->y += b->vy;

    b->frame++;
}

// Adjust speed if near zero as setDirection depends on at least one component
// of our velocity vector is non-zero.
void bl_fixSpeed(struct Bullet* b)
{
    // See https://randomascii.wordpress.com/2012/02/25/comparing-floating-point-numbers-2012-edition/

    // Direction (alone) is dependent on components, so if speed is set to 0.0f,
    // setting a direction will not do anything.
    if (fabs(b->vy) < FLT_EPSILON)
    {
        b->vy = FLT_EPSILON;
    }
}
