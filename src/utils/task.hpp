#pragma once

#include <functional>

#include "Geode/utils/Task.hpp"

using namespace geode::prelude;

namespace jukebox {

template <typename T, typename P, typename NT, typename NP>
Task<NT, NP> switchMap(Task<T, P> old,
                       std::function<Task<NT, NP>(T* res)> creator) {
    using OldTask = Task<T, P>;
    using RetTask = Task<NT, NP>;
    return RetTask::runWithCallback([creator, old](auto finish, auto p1,
                                                   auto cancelled) {
        old.listen(
            [creator, finish, p1, cancelled](T* result) {
                creator(result).listen([finish](auto r2) { finish(*r2); },
                                       [p1](auto p2) { p1(*p2); },
                                       [cancelled]() { cancelled(); });
            },
            [](auto) {}, [finish]() { finish(typename RetTask::Cancel()); });
    });
}

}  // namespace jukebox
