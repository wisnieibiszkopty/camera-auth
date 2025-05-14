struct DistanceStats {
    float mean;
    float median;
    float min;
    float max;

    DistanceStats()
        : mean(0), median(0), min(0), max(0) {}

    DistanceStats(float mean, float median, float min, float max)
        : mean(mean), median(median), min(min), max(max) {}
};