#include "RGBUtils.h"

// TODO fade up from black to full using log scale

void Fade::start(unsigned long current_time) {
    start_time = current_time;
    started = true;
}

int Fade::current(unsigned long current_time, Colour& out, float& percent) {
    // Fade hasn't been started yet, return start colour
    /*if (!started) {
        out.r = from.r;
        out.g = from.g;
        out.b = from.b;
        return RGBUtils_Fade_NotStarted;
    }*/
    // Error condition
    if (current_time < start_time) {
        return RGBUtils_Fade_Err_TimeOverflow;
    }
    // Fade completed, always return end colour
    if (current_time > start_time + duration) {
        out.r = to.r;
        out.g = to.g;
        out.b = to.b;
        return RGBUtils_Fade_Completed;
    }
    // Fade in progress, work out current values
    unsigned long time_from_start = current_time - start_time;
    float percent_complete = (float)time_from_start / (float)duration;

    percent = percent_complete;

    int r_diff = to.r - from.r;
    int g_diff = to.g - from.g;
    int b_diff = to.b - from.b;

    out.r = from.r + r_diff * percent_complete;
    out.g = from.g + g_diff * percent_complete;
    out.b = from.b + b_diff * percent_complete;
    return 0;
}
