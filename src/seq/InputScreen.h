#pragma once

#include <functional>
#include <memory>

#include "rack.hpp"

#include <widget/FramebufferWidget.hpp>


class InputControl;
class MidiSequencer;
struct NVGcolor;

using InputControlPtr = std::shared_ptr<InputControl>;
using MidiSequencerPtr = std::shared_ptr<MidiSequencer>;

struct InputScreen : public ::rack::widget::OpaqueWidget
{
public:
   InputScreen(const ::rack::math::Vec& pos,
        const ::rack::math::Vec& size,
        MidiSequencerPtr seq,
        std::function<void()> dismisser);
    ~InputScreen();

    
    void draw(const Widget::DrawArgs &args) override;

    /**
     * Mouse handler. For debugging since we don't have buttons yet.
     */
   // void onButton(const ::rack::event::Button &e) override;

   std::vector<float> getValues() const;

protected:
   MidiSequencerPtr sequencer;
   std::function<void()> dismisser = nullptr;
   std::vector<InputControl*> inputControls;

   void addPitchInput(const ::rack::math::Vec& pos, const std::string& label);
   ::rack::ui::Label* addLabel(const ::rack::math::Vec& v, const char* str, const NVGcolor& color);
};

using InputScreenPtr = std::shared_ptr<InputScreen>;



