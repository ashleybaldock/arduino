#ifndef LightMenu_h
#define LightMenu_h

class MenuItem {
    public:
        // May be null (no menu to move to in that direction)
        MenuItem* up;
        MenuItem* down;
        MenuItem* left;
        MenuItem* right;

        // Function pointer for select action?

        // Display lines
        const char* line1;  // E.g. "Preset 16"
        const char* line2;
};

#endif
