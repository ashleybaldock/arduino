#include "RGBUtils.h"

// TODO fade up from black to full using log scale

int FadeState::get_num_channels() {
    return num_channels;
}

int FadeState::set_num_channels(int num) {
    if (num <= MAX_CHANNELS && num > 0) {
        num_channels = num;
        return 0;
    } else {
        return RGBUtils_CurrentState_Err_Invalid_Channel;
    }
}

int FadeState::get_channel(int index, Colour& out, float& percent_out) {
    if (index < num_channels && index >= 0) {
        out = channels[index];
        percent_out = percents[index];
        return 0;
    } else {
        return RGBUtils_CurrentState_Err_Invalid_Channel;
    }
}

int FadeState::set_channel(int index, Colour current_colour, float percent_complete) {
    if (index < num_channels && index >= 0) {
        channels[index] = current_colour;
        percents[index] = percent_complete;
        return 0;
    } else {
        return RGBUtils_CurrentState_Err_Invalid_Channel;
    }
}


void Fade::reset() {
    start_time = 0;
}

void Fade::start(unsigned long current_time) {
    start_time = current_time;
}

void Fade::set(const Colour& colour, unsigned long dur) {
    reset();
    from = colour;
    duration = dur;
}


void FadeSequence::start(unsigned long current_time) {
    started = true;
    steps[current].start(current_time + delay);
}

void FadeSequence::reset() {
    for (int i = 0; i < num_steps; i++) {
        steps[i].reset();
    }
    started = false;
    delay = 0;
    current = 1;    // No lead-in by default
    num_steps = 0;
}

int FadeSequence::get_next_step() {
    int next = current + 1;
    if (next > num_steps) {
        return 1;   // 0 is lead-in, which we don't loop to
    }
    return next;
}

int FadeSequence::get_current(unsigned long current_time, Colour& out, float& percent) {
    Colour from = steps[current].from;
    Colour to = steps[get_next_step()].from;
    unsigned long start_time = steps[current].start_time;
    unsigned long duration = steps[current].duration;

    // Fade hasn't been started yet, return start colour
    if (!started) {
        out = from;
        return RGBUtils_Fade_NotStarted;
    }
    // Error condition
    if (current_time < start_time) {
        out = from;
        return RGBUtils_Fade_Err_TimeOverflow;
    }
    // Fade completed, always return end colour
    if (current_time > (start_time + duration)) {
        out = to;
        // Start next fade in sequence
        current = get_next_step();
        steps[current].start(current_time);
        return RGBUtils_FadeSequence_Next;
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

int FadeSequence::get_current(unsigned long current_time, FadeState& out) {
    Colour current_colour;
    float percent_complete;
    int ret = get_current(current_time, current_colour, percent_complete);

    out.set_num_channels(1);
    out.set_channel(0, current_colour, percent_complete);

    return 0;
}

int FadeSequence::set_step(int idx, const Colour& colour, unsigned long dur) {
    if (idx + 1 < MAX_FADE_STEPS + 1) {
        steps[idx + 1].set(colour, dur);
    }
}
int FadeSequence::set_step_count(int count) {
    if (count < MAX_FADE_STEPS + 1) {
        num_steps = count;
    }
}

int FadeSequence::set_lead_in(const Colour& from, unsigned long duration) {
    if (!started) {
        current = 0;
    }
    steps[0].set(from, duration);
}

int FadeSequence::set_delay(unsigned long delay_to_set) {
    delay = delay_to_set;
}


void MultiFade::reset() {
    for (int i = 0; i < num_channels; i++) {
        fade_sequences[i].reset();
    }
}

void MultiFade::start(unsigned long current_time) {
    for (int i = 0; i < num_channels; i++) {
        fade_sequences[i].start(current_time);
    }
}

int MultiFade::get_current(unsigned long current_time, FadeState& out) {
    float percent;
    Colour current;

    out.set_num_channels(num_channels);
    for (int i = 0; i < num_channels; i++) {
        fade_sequences[i].get_current(current_time, current, percent);
        out.set_channel(i, current, percent);
    }
}

int MultiFade::set_step(int idx, int step_idx, const Colour& colour, unsigned long duration) {
    if (idx < num_channels) {
        return fade_sequences[idx].set_step(step_idx, colour, duration);
    } else {
        return RGBUtils_MultiFade_Err_IndexInvalid;
    }
}

int MultiFade::set_step_count(int idx, int num_steps) {
    if (idx < num_channels) {
        return fade_sequences[idx].set_step_count(num_steps);
    } else {
        return RGBUtils_MultiFade_Err_IndexInvalid;
    }
}

int MultiFade::set_lead_in(int idx, const Colour& colour, unsigned long duration) {
    if (idx < num_channels) {
        return fade_sequences[idx].set_lead_in(colour, duration);
    } else {
        return RGBUtils_MultiFade_Err_IndexInvalid;
    }
}

int MultiFade::set_delay(int idx, unsigned long delay) {
    if (idx < num_channels) {
        return fade_sequences[idx].set_delay(delay);
    } else {
        return RGBUtils_MultiFade_Err_IndexInvalid;
    }
}


