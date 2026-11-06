#pragma once

#include <string>
#include <string_view>

#include <pl/Config.hpp>

namespace super_jump {

struct ModConfig {
    int version = 1;
};

nlohmann::json makeDefaultConfigJson();
nlohmann::json makeConfigSchemaJson();

} // namespace super_jump

template <> struct pl::config::Schema<super_jump::ModConfig> {
    static constexpr std::string_view title = "Super Jump Config";
    static constexpr std::string_view description = {};

    static constexpr FieldSchema field(std::string_view name) {
        if (name == "version")
            return {.title = "Version", .readOnly = true};
        return {};
    }
};
