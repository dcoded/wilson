#pragma once
#ifndef INCLUDE_METRICS_H

#include <map>

template <typename T>
class metric_source {
private:
    std::map <std::string, T> metrics_;
public:
    void define_metric (const std::string name, const T preset) {
        metrics_[name] = preset;
    }
    T& metric (const std::string name) {
        if (metrics_.find (name) == metrics_.end ())
            std::cout << "[metrics] '" << name << "' is not a defined metric\n";
        return metrics_[name];
    }
};

template <typename T, typename Iter>
const T aggregate_metric (const Iter begin, const Iter end, std::string name, T initial,
    std::function<const T(const T,const T)> transform) {

    T agg = initial;

    for (auto it = begin; it != end; it++) {
        agg = transform (agg, it->metric (name));
    }

    return agg;
}

#endif