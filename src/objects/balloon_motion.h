#pragma once

#include <algorithm>

namespace Balloon {

// 3D world coordinates for positioning the balloon
struct Position {
    float x;
    float y;
    float z;
};

// Constants defining start/end positions, size limits (0.6x to 1.8x), and float speed
inline constexpr Position START_POSITION{-1.6f, 1.8f, -0.7f};
inline constexpr Position TARGET_POSITION{1.0f, 3.1f, -1.6f};
inline constexpr int MIN_SCALE_TICKS = 6;
inline constexpr int INITIAL_SCALE_TICKS = 10;
inline constexpr int MAX_SCALE_TICKS = 18;
inline constexpr float SCALE_STEP = 0.1f;
inline constexpr float FLOAT_DURATION = 3.0f;

// Manages the balloon's inflation size, release state, and smooth floating animation
struct Motion {
    int scaleTicks = INITIAL_SCALE_TICKS; // Current size step (default 10 = 1.0x)
    Position position = START_POSITION;   // Current 3D location in the room
    float progress = 0.0f;                // Travel completion from 0.0 (start) to 1.0 (target)
    bool released = false;                // True once inflated to max size (1.8x)

    // Converts integer size ticks into a decimal scale multiplier
    float scale() const {
        return scaleTicks * SCALE_STEP;
    }

    // Resets the balloon back to its initial size, starting spot, and un-released state
    void reset() {
        scaleTicks = INITIAL_SCALE_TICKS;
        position = START_POSITION;
        progress = 0.0f;
        released = false;
    }

    // Adjusts balloon size on mouse wheel input and triggers release at max size
    void scroll(int direction) {
        if (direction == 0) return;
        scaleTicks = std::clamp(scaleTicks + (direction > 0 ? 1 : -1),
                                MIN_SCALE_TICKS, MAX_SCALE_TICKS);
        if (scaleTicks == MAX_SCALE_TICKS) released = true;
    }

    // Smoothly moves the balloon from start to target over FLOAT_DURATION once released
    void update(float deltaSeconds) {
        if (!released || progress >= 1.0f) return;

        // Advance travel timer (clamped to avoid large jumps on frame lag)
        const float dt = std::clamp(deltaSeconds, 0.0f, 0.1f);
        progress = std::min(1.0f, progress + dt / FLOAT_DURATION);

        // Apply smoothstep easing for gradual acceleration and deceleration
        const float t = progress * progress * (3.0f - 2.0f * progress);

        // Interpolate 3D coordinates between start and target positions
        position.x = START_POSITION.x + (TARGET_POSITION.x - START_POSITION.x) * t;
        position.y = START_POSITION.y + (TARGET_POSITION.y - START_POSITION.y) * t;
        position.z = START_POSITION.z + (TARGET_POSITION.z - START_POSITION.z) * t;
    }
};
}