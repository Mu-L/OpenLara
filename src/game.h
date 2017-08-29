#ifndef H_GAME
#define H_GAME

#include "core.h"
#include "format.h"
#include "level.h"
#include "ui.h"

namespace Game {
    Level *level;

    void startLevel(Stream *lvl) {
        delete level;
        level = new Level(*lvl);
        UI::init(level);
        delete lvl;
    }

    void stopChannel(Sound::Sample *channel) {
        if (level) level->stopChannel(channel);
    }

    void init(Stream *lvl) {
        Core::init();
        Sound::callback = stopChannel;

        Core::settings.detail.ambient       = true;
        Core::settings.detail.lighting      = true;
        Core::settings.detail.shadows       = true;
        Core::settings.detail.water         = Core::support.texFloat || Core::support.texHalf;
        Core::settings.detail.contact       = false;

#ifdef __RPI__
        Core::settings.detail.ambient       = false;
        Core::settings.detail.shadows       = false;
#endif

        Core::settings.controls.retarget    = true;

        level = NULL;
        startLevel(lvl);
    }

    void init(char *lvlName = NULL, char *sndName = NULL) {
        if (!lvlName) lvlName = (char*)"LEVEL2.PSX";
        init(new Stream(lvlName));
    }

    void free() {
        delete level;
        Core::free();
    }

    void updateTick() {
        if (Input::state[cInventory])
            level->inventory.toggle();

        float dt = Core::deltaTime;
        if (Input::down[ikR]) // slow motion (for animation debugging)
            Core::deltaTime /= 10.0f;

        if (Input::down[ikT]) // fast motion
            for (int i = 0; i < 9; i++)
                level->update();

        level->update();

        Core::deltaTime = dt;
    }

    void update(float delta) {
        PROFILE_MARKER("UPDATE");
        Input::update();

        if (Input::down[ikV]) { // third <-> first person view
            level->camera->changeView(!level->camera->firstPerson);
            Input::down[ikV] = false;
        }

        Core::deltaTime = delta = min(1.0f, delta);
        UI::update();

        while (delta > EPS) {
            Core::deltaTime = min(delta, 1.0f / 30.0f);
            Game::updateTick();
            delta -= Core::deltaTime;
        }
    }

    void render() {
        PROFILE_MARKER("RENDER");
        PROFILE_TIMING(Core::stats.tFrame);
        Core::beginFrame();
        level->render();
        UI::renderTouch();

        #ifdef _DEBUG
            level->renderDebug();
        #endif

        Core::endFrame();
    }
}

#endif
