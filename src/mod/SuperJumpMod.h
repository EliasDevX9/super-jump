#pragma once

#include <atomic>
#include <string_view>

#include <pl/Mod.hpp>

namespace super_jump {

class SuperJumpMod {
  public:
    static SuperJumpMod &instance();

    SuperJumpMod();

    [[nodiscard]] ll::mod::NativeMod &getSelf() const { return mSelf; }

    bool load();
    bool enable();
    bool disable();
    bool unload();

    // Called from the hook detour.
    [[nodiscard]] float multiplier() const { return mEnabled ? mMultiplier.load() : 1.0f; }

  private:
    ll::mod::NativeMod &mSelf;
    std::atomic<bool> mEnabled{true};
    std::atomic<float> mMultiplier{1.8f};
    bool mRegistered = false;
    bool mHooked = false;
};

} // namespace super_jump
