#include <string>
#include <utility>

#include <tau/source/entry.hpp>
#include <tau/source/map.hpp>

namespace source {
Map::Map(std::string text) noexcept {
    this->add("<repl>", std::move(text));
}

auto Map::add(Entry entry) noexcept -> Id {
    entries_.push_back(std::move(entry));

    return entries_.size() - 1;
}

auto Map::add(std::string name, std::string text) noexcept -> Id {
    return this->add(Entry{std::move(name), std::move(text)});
}

auto Map::get(Id id) const noexcept -> const Entry& {
    return entries_[id];
}
}  // namespace source
