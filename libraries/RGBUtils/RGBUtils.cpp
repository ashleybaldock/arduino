#include "RGBUtils.h"

// TODO fade up from black to full using log scale

int FadeState::get_num_channels() {
    return num_channels;
}

int FadeState::set_num_channels(int num) {
    if (num < MAX_CHANNELS && num > 0) {
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

void Fade::start(unsigned long current_time) {
    start_time = current_time;
    started = true;
}

int Fade::get_current(unsigned long current_time, Colour& out, float& percent) {
    Colour to = next->from;
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

void Fade::set_next(Fade* next_fade) {
    next = next_fade;
}


void FadeSequence::start(unsigned long current_time) {
    started = true;
    if (lead_in != 0) {
        current = lead_in;
    } else {
        current = first;
    }
    current->start(current_time + delay);
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

int FadeSequence::get_current(unsigned long current_time, FadeState& out) {
    float percent;
    Colour current_colour;
    get_current(current_time, current_colour, percent);

    out.set_num_channels(1);
    out.set_channel(0, current_colour, percent);

    return 0;
}

void FadeSequence::add(const Colour& from, unsigned long duration) {
    Fade* fade = new Fade(from, duration);

    // Link up to existing list
    if (last == 0) {
        first = fade;
        last = fade;
        fade->set_next(fade);
    } else {
        // Always add at the end of sequence - so first comes next
        fade->set_next(first);
        last->set_next(fade);
        last = fade;
    }
}

void FadeSequence::set_lead_in(const Colour& from, unsigned long duration) {
    Fade* fade = new Fade(from, duration);

    if (lead_in == 0) {
        // Set new lead in
        lead_in = fade;
        lead_in->set_next(first);
    } else {
        // Replace existing lead in
        delete lead_in;
        lead_in = fade;
        lead_in->set_next(first);
    }
}

void FadeSequence::set_delay(unsigned long delay_to_set) {
    delay = delay_to_set;
}

// Copy constructor
FadeSequence::FadeSequence(const FadeSequence& other) {
    // Copy basic params
    started = other.started;
    delay = other.delay;
    // Go through linked list and create new items
    if (first == 0) {
        first = 0;
        current = 0;
        last = 0;
    } else {
        // Create first node
        first = new Fade(*other.first);

        // Pointer to node currently looking at
        Fade* c = first;
        Fade* f = other.first->next;
        // Until we loop around
        while (f != other.first) {
            c->next = new Fade(*f);
            f = f->next;
            c = c->next;
        }
        // Finish off the loop
        last = c;
        last->next = first;

        current = first;    // TODO - need to copy this properly based on input value
    }
    if (other.lead_in != 0) {
        lead_in = new Fade(*other.lead_in);
        lead_in->next = first;
    }
}


void MultiFade::start(unsigned long current_time) {
    for (int i = 0; i < MultiFade_MAX; i++) {
        if (fade_sequences[i] != 0) {
            fade_sequences[i]->start(current_time);
        }
    }
}

int MultiFade::get_current(unsigned long current_time, FadeState& out) {
    float percent;
    Colour current;

    out.set_num_channels(MultiFade_MAX);
    for (int i = 0; i < MultiFade_MAX; i++) {
        if (fade_sequences[i] != 0) {
            fade_sequences[i]->get_current(current_time, current, percent);
            out.set_channel(i, current, percent);
        } else {
            out.set_channel(i, Colour(), 0);
        }
    }
}

int MultiFade::get_current(int index, unsigned long current_time, Colour& out, float& percent) {
    if (index >= 0 && index < MultiFade_MAX) {
        if (fade_sequences[index] != 0) {
            return fade_sequences[index]->get_current(current_time, out, percent);
        } else {
            return RGBUtils_MultiFade_Err_IndexNotInitialised;
        }
    } else {
        return RGBUtils_MultiFade_Err_IndexInvalid;
    }
}

int MultiFade::set_fade_sequence(int index, FadeSequence* sequence) {
    if (index >= 0 && index < MultiFade_MAX) {
        fade_sequences[index] = sequence;
        return 0;
    } else {
        return RGBUtils_MultiFade_Err_IndexInvalid;
    }
}


