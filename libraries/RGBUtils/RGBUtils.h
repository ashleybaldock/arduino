#ifndef RGBUtils_h
#define RGBUtils_h

#define MultiFade_MAX 9
#define RGBUtils_Fade_Err_TimeOverflow 1
#define RGBUtils_Fade_NotStarted 10
#define RGBUtils_Fade_Completed 11
#define RGBUtils_FadeSequence_Next 20
#define RGBUtils_MultiFade_Err_IndexInvalid 30
#define RGBUtils_MultiFade_Err_IndexNotInitialised 31
#define RGBUtils_CurrentState_Err_Invalid_Channel 40

#define MAX_CHANNELS 10

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
        virtual int get_current(unsigned long, FadeState&) = 0;
};

// Represents a DMX lighting fixture
class Fixture {
    public:
        int base_address;
        Fadeable* sequence;

        Fixture() : base_address(0), sequence(0) {};
        Fixture(int base_address, Fadeable* sequence) : base_address(base_address),
            sequence(sequence) {};
};

// Class used by FadeSequence to represent a Fade step
class Fade {
    private:
        Colour from;
        bool started;
        unsigned long start_time;
        unsigned long duration;
    public:
        // duration is specified in ms (s/1000)
        Fade(Colour from, unsigned long duration) :
         from(from), duration(duration), started(false), start_time(0), next(this) {};

        void start(unsigned long);
        int get_current(unsigned long, Colour&, float&);

        Fade* next;
        void set_next(Fade*);
};

// Sequence of Fades (looped) with a delay/offset to start
// Automatically move to next Fade in sequence
class FadeSequence: public Fadeable {
    private:
        Fade* first;
        Fade* current;
        Fade* last;    // For insertion of new elements to linked list
        Fade* lead_in; // Fade to be executed first before loop is entered into
        bool started;
        unsigned long delay;
    public:
        FadeSequence() : first(0), current(0), last(0), delay(0), lead_in(0) {};
        FadeSequence(unsigned long delay) : first(0), current(0), last(0),
            delay(delay), lead_in(0) {};
        FadeSequence(const FadeSequence&);
        ~FadeSequence();

        void start(unsigned long);
        int get_current(unsigned long, FadeState&);

        int get_current(unsigned long, Colour&, float&);

        // Add another Fade to the end of the sequence
        void add(const Colour&, unsigned long);
        void set_lead_in(const Colour&, unsigned long);
        // Configure delay before sequence starts
        void set_delay(unsigned long);
};

// TODO needs destructor
class MultiFade: public Fadeable {
    private:
        FadeSequence* fade_sequences[MultiFade_MAX];
    public:
        MultiFade() : fade_sequences() {};

        void start(unsigned long);
        int get_current(unsigned long, FadeState&);

        int get_current(int, unsigned long, Colour&, float&);

        int set_fade_sequence(int, FadeSequence*);
};

#endif
