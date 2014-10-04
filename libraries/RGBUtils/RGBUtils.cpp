#include "RGBUtils.h"

// TODO fade up from black to full using log scale

void Fade::start(unsigned long current_time) {
    start_time = current_time;
    started = true;
}

int Fade::get_current(unsigned long current_time, Colour& out, float& percent) {
    // Fade hasn't been started yet, return start colour
    if (!started) {
        out = from;
        return RGBUtils_Fade_NotStarted;
    }
    // Error condition
    if (current_time < start_time) {
        return RGBUtils_Fade_Err_TimeOverflow;
    }
    // Fade completed, always return end colour
    if (current_time > start_time + duration) {
        out = to;
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

void set_next(Fade* next_fade) {
    next = next_fade;
}


void FadeSequence::start(unsigned long current_time) {
    start_time = current_time;
    started = true;
}

int FadeSequence::get_current(unsigned long current_time, Colour& out, float& percent) {
    int ret = 0;
    ret = current->get_current(current_time, out, percent);

    // Move to next fade if current one has been completed
    if (ret == RGBUtils_Fade_Completed) {
        current = current->next;
        current->start(current_time);   // TODO should this be overall start time + some offset?
        return RGBUtils_FadeSequence_Next;
    }

    return 0;
}

void FadeSequence::add(Fade* fade_to_add) {
    if (last == 0) {
        first = fade_to_add;
        current = fade_to_add;
        last = fade_to_add;
        fade_to_add->set_next(fade_to_add);
    } else {
        // Always add at the end of sequence - so first comes next
        fade_to_add->set_next(first);
        last->set_next(fade_to_add);
        last = fade_to_add;
    }
}

void FadeSequence::set_delay(unsigned long delay_to_set) {
    delay = delay_to_set;
}


void MultiFade::start(unsigned long current_time) {
    for (int i = 0; i <= MultiFade_MAX; i++) {
        if (fade_sequences[i != 0]) {
            fade_sequences[i]->start(current_time);
        }
    }
}

int MultiFade::get_current(int index, unsigned long current_time, Colour& out, float& percent) {
    if (index >= 0 && index <= MultiFade_MAX]) {
        if (fade_sequences[index != 0]) {
            fade_sequences[index]->get_current(current_time, out, percent);
        } else {
            return RGBUtils_MultiFade_Err_IndexNotInitialised;
        }
    } else {
        return RGBUtils_MultiFade_Err_IndexInvalid;
    }
}

int MultiFade::set_fade_sequence(int index, FadeSequence* sequence) {
    if (index >= 0 && index <= MultiFade_MAX]) {
        fade_sequences[index] = sequence;
        return 0;
    } else {
        return RGBUtils_MultiFade_Err_IndexInvalid;
    }
}


void NineFade::start(unsigned long current_time) {
    start_time = current_time;
    started = true;
}

void NineFade::set_rgb(Colour& out, unsigned char r, unsigned char g, unsigned char b) {
    out.r = r;
    out.g = g;
    out.b = b;
}

int NineFade::current(unsigned long current_time, Colour& out1, Colour& out2, Colour& out3,
                      Colour& out4, Colour& out5, Colour& out6, Colour& out7, Colour& out8,
                      Colour& out9, float& percent) {
    // Fade hasn't been started yet, return start colour
    if (!started) {
        out1 = from;
        out2 = from;
        out3 = from;
        out4 = from;
        out5 = from;
        out6 = from;
        out7 = from;
        out8 = from;
        out9 = from;
        return RGBUtils_Fade_NotStarted;
    }
    // Error condition
    if (current_time < start_time) {
        return RGBUtils_Fade_Err_TimeOverflow;
    }
    // Fade completed, always return end colour
    if (current_time > start_time + duration) {
        out1 = to;
        out2 = to;
        out3 = to;
        out4 = to;
        out5 = to;
        out6 = to;
        out7 = to;
        out8 = to;
        out9 = to;
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
