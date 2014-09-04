#ifndef RGBUtils_h
#define RGBUtils_h

#define RGBUtils_Fade_Err_TimeOverflow 1
#define RGBUtils_Fade_NotStarted 10
#define RGBUtils_Fade_Completed 11

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
    public:
        // duration is specified in ms (s/1000)
        Fade(Colour from, Colour to, unsigned long duration) :
         from(from), to(to), duration(duration), started(false), start_time(0) {};

        void start(unsigned long);
        int current(unsigned long, Colour&, float&);
};

#endif
