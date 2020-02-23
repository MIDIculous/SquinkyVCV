#include "ClockFinder.h"

#include "app.hpp"
#include "app/CableWidget.hpp"
#include "app/Scene.hpp"
#include "app/ModuleWidget.hpp"
#include "app/ParamWidget.hpp"
#include "app/PortWidget.hpp"
#include "engine/Engine.hpp"
#include "plugin/Model.hpp"
#include "widget/Widget.hpp"

using ModuleWidget = ::rack::app::ModuleWidget;
using Model = ::rack::plugin::Model;
using PortWidget = ::rack::app::PortWidget;
using CableWidget = ::rack::app::CableWidget;
using ParamWidget = ::rack::app::ParamWidget;
using Engine = ::rack::engine::Engine;

static ModuleWidget* findClocked()
{
    const std::string ck("Clocked");
    auto rack = ::rack::appGet()->scene->rack;
    for (::rack::widget::Widget* w2 : rack->moduleContainer->children) {
        ModuleWidget* modwid = dynamic_cast<ModuleWidget *>(w2);
        if (modwid) {
            Model* model = modwid->model;
            if (model->slug == ck) {
                return modwid;
            }
        } else {
            WARN("was not a module widget");
        }
    }   
    return nullptr;
}

// TODO: generalize to both seq
// clock, run, reset
static std::vector<PortWidget*> findSeqInputs(ModuleWidget* seq) {
    int found = 0;
     std::vector<PortWidget*> ret(3);

     for (auto input : seq->inputs) {
         switch(input->portId) {
            case 0:
                ret[0] = input;
                ++found;
                break;
            case 1:
                ret[2] = input;
                ++found;
                break;
            case 2:
                ret[1] = input;
                ++found;
                break;
         }
     }
     return (found == 3 ) ? ret : std::vector<PortWidget*>();
}

// Since we already know which one to use for clock, we pass that int
static std::vector<PortWidget*> findClockedOutputs(ModuleWidget* clocked, PortWidget* clock) {
    int found = 1;
    std::vector<PortWidget*> ret(3);

    assert(clock);
    ret[0] = clock;
    for (auto output : clocked->outputs) {
        switch(output->portId) {
    #if 0
            case 1:         // clock 1
                ret[0] = output;
                ++found;
                break;
    #endif
            case 5:         // run
                ret[1] = output;
                ++found;
                break;
            case 4:         // reset
                ret[2] = output;
                ++found;
                break;
        }
    }
    return (found == 3 ) ? ret : std::vector<PortWidget*>();
}

static bool anyConnected(const std::vector<PortWidget*>& ports)
{
    for (auto port : ports) {
        auto cables = APP->scene->rack->getCablesOnPort(port);
        if (!cables.empty()) {
            return true;
        }
    }
    return false;
}

// index = 0 - clock 1 ... 2 - clock3 (skip master)
PortWidget* findClockOutput(ModuleWidget* clocked, int index)
{
    for (auto output : clocked->outputs) {
        if ((output->portId) == (index + 1)) {
            return output;
        }
    }
    assert(false);
    return nullptr;
}

/**
 * return.first will be a non null port widget to use for clock/
 * return.second will be true if the port is already patched
 */
static std::pair<PortWidget*, bool> findBestClockOutput(ModuleWidget* clocked)
{
    for (int i=0; i<3; ++i) {
        auto port = findClockOutput(clocked, i);
        auto cables = APP->scene->rack->getCablesOnPort(port);
        if (cables.empty()) {
            INFO("find best found an unused clock");
            return std::make_pair(port, false);
        }
    }
    INFO("find best clock found none unpatched");
    return std::make_pair(findClockOutput(clocked, 0), true);
}

static ParamWidget* getClockRateParam(ModuleWidget* clocked, int index)
{
    for (auto param : clocked->params) {
        if (!param->paramQuantity) {
            WARN("param has no quantity");
            return nullptr;
        }
        int id = param->paramQuantity->paramId;
        if (id == (index + 1)) {
            return param;
        }
    }
    assert(false);
    return nullptr;
}

static float clockDivToClockedParam(int div)
{
    float ret = 0;
    assert(div > 0);
    if (div <= 16) {
        ret = div + 1;
    } else if (div == 32) {
        ret = 7+17;
    } else if (div == 64) {
        ret = 7 + 17 + 9;
    } else {
        assert(false);
    }
    return ret;
}

void ClockFinder::go(ModuleWidget* host, int div)
{
    INFO("Clock Finder");
    ModuleWidget* clockedModule = findClocked();
    if (!clockedModule) {
        return;
    }

    auto clockOutput = findBestClockOutput(clockedModule);
    assert(clockOutput.first != nullptr);

    auto outputs = findClockedOutputs(clockedModule, clockOutput.first);
    auto inputs = findSeqInputs(host);
    if (outputs.size() != 3 || inputs.size() != 3) {

        WARN("bad I/O matchup. o=%d, i=%d", outputs.size(), inputs.size());
        return;
    }

    if (anyConnected(inputs)) {
        WARN("not patching, some are already patched\n");
        return;
    }
 
    const NVGcolor color = nvgRGB(0, 0, 0);
    for (int i=0; i<3; ++i) {
        CableWidget* cable = new CableWidget();
            cable->color = color;
        cable->setOutput(outputs[i]);
        cable->setInput(inputs[i]);
        APP->scene->rack->addCable(cable);
    }

    INFO("alreay patched = %d", clockOutput.second);
    if (!clockOutput.second) {
        // if the clock output was empty before us
        const int clockIndex = clockOutput.first->portId - 1;
        INFO("sell set clock %d", clockIndex);
        auto paramWidget = getClockRateParam(clockedModule, clockIndex);
        INFO("param w = %p", paramWidget);
        if (paramWidget) {
            auto module = clockedModule->module;
            const int paramId = paramWidget->paramQuantity->paramId;
            const float value = clockDivToClockedParam(div);
            APP->engine->setParam(module, paramId, value);
            INFO("just set ration to %.2f", value);
        }
    }
    INFO("done patching");
}