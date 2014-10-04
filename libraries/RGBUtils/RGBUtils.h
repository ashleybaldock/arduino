#ifndef RGBUtils_h
#define RGBUtils_h

#define MultiFade_MAX 9
#define RGBUtils_Fade_Err_TimeOverflow 1
#define RGBUtils_Fade_NotStarted 10
#define RGBUtils_Fade_Completed 11
#define RGBUtils_FadeSequence_Next 20
#define RGBUtils_MultiFade_Err_IndexInvalid 30
#define RGBUtils_MultiFade_Err_IndexNotInitialised 31

class Colour {
    public:
        unsigned char r;
        unsigned char g;
        unsigned char b;

        Colour(int r, int g, int b) : r(r), g(g), b(b) {};
        Colour() : r(0), g(0), b(0) {};
};

class Fade {
    private:
        Colour from;
        Colour to;
        bool started;
        unsigned long start_time;
        unsigned long duration;
        Fade* next;
    public:
        // duration is specified in ms (s/1000)
        Fade(Colour from, Colour to, unsigned long duration) :
         from(from), to(to), duration(duration), started(false), start_time(0), next(0) {};

        void start(unsigned long);
        int get_current(unsigned long, Colour&, float&);

        void set_next(Fade*);
};

// Sequence of Fades (looped) with a delay/offset to start
// Automatically move to next Fade in sequence
// TODO needs copy constructor
class FadeSequence {
    private:
        Fade* first;
        Fade* current;
        Fade* last;    // For insertion of new elements to linked list
        bool started;
        unsigned long delay;
    public:
        FadeSequence() : current(0), last(0), delay(0) {};
        FadeSequence(unsigned long delay) : current(0), last(0), delay(delay) {};

        void start(unsigned long);
        int get_current(unsigned long, Colour&, float&);

        // Add another fade to the end of this sequence
        void add(Fade*);
        // Configure delay before sequence starts
        void set_delay(unsigned long);
};

class MultiFade {
    private:
        Fade* fade_sequences[MultiFade_MAX];
    public:
        MultiFade() : fade_sequences() {};

        void start(unsigned long);
        int get_current(int, unsigned long, Colour&, float&);

        int set_fade_sequence(int, FadeSequence*);
};

// Fade from one colour to another segment-by-segment
class NineFade {
    private:
        Colour from;
        Colour to;
        bool started;
        unsigned long start_time;
        unsigned long duration;
        unsigned long segment_duration;
    public:
        NineFade(Colour from, Colour to, unsigned long duration, unsigned long segment_duration) :
         from(from), to(to), duration(duration), segment_duration(segment_duration), started(false), start_time(0), next(0) {};

        void start(unsigned long);
        int current(unsigned long, Colour&, Colour&, Colour&, Colour&, Colour&, Colour&, Colour&, Colour&, Colour&, float&);

        NineFade* next;
};

#endif
