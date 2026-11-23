#pragma once

#include "BalloonMotion.h"

namespace Balloon {

// Sets up the balloon's 3D mesh buffer and procedural latex texture
void initialize();

// Renders the balloon (body, knot, and string) at its current position and scale
void draw();

// Handles mouse wheel input to inflate or deflate the balloon
void onScroll(int direction);

// Advances the balloon's floating animation over elapsed frame time
void update(float deltaSeconds);

// Resets the balloon back to its starting size and position
void reset();

// Cleans up OpenGL GPU buffer and texture resources
void destroy();

// Returns a read-only reference to the balloon's current motion and scale state
const Motion& state();

}