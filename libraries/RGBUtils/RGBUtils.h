#ifndef RGBUtils_h
#define RGBUtils_h

#define RGBUtils_Fade_Err_TimeOverflow 1
#define RGBUtils_Fade_NotStarted 10
#define RGBUtils_Fade_Completed 11
#define RGBUtils_FadeSequence_Next 20
#define RGBUtils_MultiFade_Err_IndexInvalid 30
#define RGBUtils_MultiFade_Err_IndexNotInitialised 31
#define RGBUtils_MultiFade_Err_CountExceedsMaxChannels 32
#define RGBUtils_CurrentState_Err_Invalid_Channel 40

#define MAX_CHANNELS 9
#define MAX_FADE_STEPS 5
#define MAX_MULTI_FADE_STEPS 3

#define Fixture_Single 1
#define Fixture_Multi  2

class Colour {
    public:
        unsigned char r;
        unsigned char g;
        unsigned char b;

        Colour(int r, int g, int b) : r(r), g(g), b(b) {};
        Colour() : r(0), g(0), b(0) {};
};

// Returned by Fadeable objects - gives current state of RGB channel(s)
class FadeState {
    private:
        int num_channels;
        Colour channels[MAX_CHANNELS];
        float percents[MAX_CHANNELS];
    public:
        int get_num_channels();
        int set_num_channels(int);
        int get_channel(int, Colour&, float&);
        int set_channel(int, Colour, float);
};

class Fadeable {
    public:
        virtual void start(unsigned long) = 0;
        virtual void reset() = 0;
        virtual int get_current(unsigned long, FadeState&) = 0;
};

// Represents a DMX lighting fixture
class Fixture {
    public:
        int address;
        Fadeable* sequence;
        int type;

        Fixture() : address(0), sequence(0), type(Fixture_Single) {};
        Fixture(int address, Fadeable* sequence, int type) : address(address), sequence(sequence), type(type) {};
};

class Fade {
    friend class FadeSequence;
    private:
        Colour from;
        unsigned long duration;
        unsigned long start_time;
    public:
        // duration is specified in ms (s/1000)
        Fade(): duration(0), start_time(0) {};

        void reset();
        void start(unsigned long);
        void set(const Colour&, unsigned long);
};

// Fade sequences can be up to MAX_FADE_STEPS long
class FadeSequence: public Fadeable {
    private:
        Fade steps[MAX_FADE_STEPS + 1]; // steps[0] is the lead-in fade
        bool started;
        unsigned long delay;
        unsigned char num_steps;
        const static unsigned char max_steps = MAX_FADE_STEPS;
        unsigned char current;
        int get_next_step();
    public:
        FadeSequence() { reset(); };

        void reset();
        void start(unsigned long);
        int get_current(unsigned long, FadeState&);
        int get_current(unsigned long, Colour&, float&);

        int set_step(int idx, const Colour&, unsigned long);
        int set_step_count(int count);
        int set_lead_in(const Colour&, unsigned long);
        int set_delay(unsigned long);
};

// Like FadeSequence but fewer steps (memory constraints)
class MultiFadeSequence: public FadeSequence {
    private:
        Fade steps[MAX_MULTI_FADE_STEPS + 1]; // steps[0] is the lead-in fade
        const static unsigned char max_steps = MAX_MULTI_FADE_STEPS;
    public:
        MultiFadeSequence() { reset(); };
};

class MultiFade: public Fadeable {
    private:
        MultiFadeSequence fade_sequences[MAX_CHANNELS];
        unsigned char num_channels;
    public:
        MultiFade() : fade_sequences(), num_channels(9) {};
        MultiFade(unsigned char num_channels) : fade_sequences(), num_channels(num_channels) {};

        void reset();
        void start(unsigned long);
        int get_current(unsigned long, FadeState&);

        int set_channel_count(unsigned char);
        int set_step(int, int, const Colour&, unsigned long);
        int set_step_count(int, int);
        int set_lead_in(int, const Colour&, unsigned long);
        int set_delay(int, unsigned long);
};

#endif
