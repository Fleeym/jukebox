#pragma once

#include <functional>

#include "Geode/utils/Task.hpp"

using namespace geode::prelude;

namespace jukebox {

template <typename T, typename P, typename NT, typename NP>
Task<NT, NP> switchMap(Task<T, P> old,
                       std::function<Task<NT, NP>(T* res)> mapper) {
    using OldTask = Task<T, P>;
    using RetTask = Task<NT, NP>;
    return RetTask::runWithCallback([mapper, old](
                                        std::function<void(NT)> mapResult,
                                        std::function<void(NP)> mapProgress,
                                        std::function<void()> mapperCancelled) {
        old.listen(
            [mapper, mapResult, mapProgress, mapperCancelled](T* result) {
                mapper(result).listen(
                    [mapResult](NT* mapped) { mapResult(std::move(*mapped)); },
                    [mapProgress](NP* mapped) { mapProgress(*mapped); },
                    [mapperCancelled]() { mapperCancelled(); });
            },
            [](auto) {}, [mapperCancelled]() { mapperCancelled(); });
    });
}

}  // namespace jukebox
