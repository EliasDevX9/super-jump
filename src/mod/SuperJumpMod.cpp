#include "mod/SuperJumpMod.h"

#include <cstdlib>
#include <string>

#include <pl/Mod.hpp>
#include <pl/ModMenu.hpp>
#include <pl/memory/Hook.hpp>
#include <pl/memory/Signature.hpp>

namespace super_jump {

namespace {

constexpr const char *kModuleId = "super_jump";

// Minecraft 1.26.60.04 (arm64-v8a), libminecraftpe.so.
// Small function that returns the base jump value (0.42f) or 0.2f when the flag is set;
// otherwise it tail-calls the real computation. Unique match in the whole .text section.
constexpr const char *kJumpSignature =
    "A1 00 00 36 A8 99 99 52 88 C9 A7 72 00 01 27 1E C0 03 5F D6";

using JumpFn = float (*)(void *, bool);
JumpFn gOriginalJump = nullptr;

float jumpDetour(void *self, bool flag) {
    const float value = gOriginalJump ? gOriginalJump(self, flag) : 0.42f;
    return value * SuperJumpMod::instance().multiplier();
}

} // namespace

SuperJumpMod &SuperJumpMod::instance() {
    static SuperJumpMod impl;
    return impl;
}

SuperJumpMod::SuperJumpMod() : mSelf(*ll::mod::NativeMod::current()) {}

bool SuperJumpMod::load() {
    getSelf().getLogger().info("Super Jump loaded (target: 1.26.60.04)");
    return true;
}

bool SuperJumpMod::enable() {
    auto &log = getSelf().getLogger();

    const bool ok =
        pl::modmenu::ModuleBuilder(kModuleId, "Super Jump")
            .description("Multiplica la fuerza de salto. Experimental, solo Minecraft 1.26.60.04.")
            .defaultEnabled(true)
            .onToggle([this](std::string_view, bool enabled) { mEnabled = enabled; })
            .config("multiplier", "Multiplicador de salto", pl::modmenu::ConfigType::SliderFloat,
                    "1.8", "1.0", "6.0")
            .onConfigChanged([this](std::string_view, std::string_view key, std::string_view value) {
                if (key == "multiplier") {
                    const float v = std::strtof(std::string(value).c_str(), nullptr);
                    if (v >= 0.5f && v <= 10.0f)
                        mMultiplier = v;
                }
            })
            .registerModule();
    if (!ok) {
        log.error("Could not register the Mod Menu module");
        return false;
    }
    mRegistered = true;

    const uintptr_t addr = pl::memory::resolveSignature(kJumpSignature, "libminecraftpe.so");
    if (addr == 0) {
        log.error("Jump signature not found. This mod only supports Minecraft 1.26.60.04.");
        return true; // module stays visible, but does nothing
    }

    const int rc = pl::memory::hook(reinterpret_cast<pl::memory::FuncPtr>(addr),
                                    reinterpret_cast<pl::memory::FuncPtr>(&jumpDetour),
                                    reinterpret_cast<pl::memory::FuncPtr *>(&gOriginalJump));
    if (rc != 0) {
        log.error("Hook failed ({})", rc);
        return true;
    }
    mHooked = true;
    log.info("Jump hook installed at {:#x}", addr);
    return true;
}

bool SuperJumpMod::disable() {
    if (mHooked) {
        pl::memory::unhook(reinterpret_cast<pl::memory::FuncPtr>(
                               pl::memory::resolveSignature(kJumpSignature, "libminecraftpe.so")),
                           reinterpret_cast<pl::memory::FuncPtr>(&jumpDetour));
        mHooked = false;
    }
    if (mRegistered) {
        pl::modmenu::unregisterModule(kModuleId);
        mRegistered = false;
    }
    return true;
}

bool SuperJumpMod::unload() { return true; }

} // namespace super_jump
