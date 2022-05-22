#include <algorithm>
#include "median_filter.h"

MedianFilter::MedianFilter() {
    this->reset();
}

void MedianFilter::reset() {
    this->count = 0;
    this->values = {};
}

bool MedianFilter::insert(float in) {
    if (this->count == MEDIAN_COUNT) {
        return true;
    }
    // insert new value
    this->values[this->count] = in;
    this->count++;
    // check if there are enough values to calc
    return this->count == MEDIAN_COUNT;
}

float MedianFilter::calc() {
    std::sort(this->values.begin(), this->values.end());
    float out = this->values[MEDIAN_COUNT / 2];
    this->reset();
    return out;
}