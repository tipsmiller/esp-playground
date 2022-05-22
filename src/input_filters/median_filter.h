#include <array>

static const int MEDIAN_COUNT = 5;

class MedianFilter {
    private:
        std::array<float, MEDIAN_COUNT> values {};
        int count;
    public:
        MedianFilter();
        bool insert(float in);
        float calc();
        void reset();
};