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
void aggregate_metric (const Iter begin, const Iter end, T& agg, std::string name,
    std::function<const T(const T,const T)> transform) {

    for (auto it = begin; it != end; it++)
        agg = transform (agg, it->metric (name));
}

#endif