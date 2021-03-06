#pragma once

#include "MidiSequencer4.h"
#include "Seq4.h"
#include "TimeUtils.h"
#include "WidgetComposite.h"
#include "math.hpp"
#include "rack.hpp"

#include <functional>

class S4Button;
class MidiTrack;
class MidiTrack4Options;
using MidiTrackPtr = std::shared_ptr<MidiTrack>;
using MidiTrack4OptionsPtr = std::shared_ptr<MidiTrack4Options>;

class S4ButtonDrawer : public ::rack::OpaqueWidget {
public:
    S4ButtonDrawer(const rack::math::Vec& size, const rack::math::Vec& pos, S4Button* button) : button(button) {
        this->box.size = size;
    }
    void draw(const DrawArgs& args) override;

private:
    void paintButtonFace(NVGcontext*);
    void paintButtonBorder(NVGcontext*);
    void paintButtonText(NVGcontext*);

    S4Button* const button;
};

class S4Button : public ::rack::OpaqueWidget {
public:
    friend class S4ButtonDrawer;
    friend class RepeatItem;
    friend class EditMenuItems;
    S4Button(const rack::math::Vec& size,
             const rack::math::Vec& pos,
             int r, int c,
             MidiSong4Ptr s,
             std::shared_ptr<Seq4<WidgetComposite>> seq4Comp);

    /**
     * pass callback here to handle clicking on LED
     */
    using callback = std::function<void(bool isCtrlKey)>;
    void setClickHandler(callback);
    //   void setEditHander(std::function<void()>);
    void setSelection(bool);

    void onButton(const rack::event::Button& e) override;
    void onDragHover(const rack::event::DragHover& e) override;
    void onDragEnter(const rack::event::DragEnter& e) override;
    void onDragLeave(const rack::event::DragLeave& e) override;
    void onSelectKey(const rack::event::SelectKey& e) override;

    bool isSelected() const {
        return _isSelected;
    }

    void step() override;

    void setNewSeq(MidiSequencer4Ptr newSeq) {
        song = newSeq->song;
    }

    void doEditClip();

private:
    rack::widget::FramebufferWidget* fw = nullptr;
   
    S4ButtonDrawer* drawer = nullptr;
    callback clickHandler = nullptr;
    // std::function<void()> editHandler = nullptr;
    bool isDragging = false;

//  row(r), col(c), song(s), seq4Comp(seq4
    const int row;
    const int col;
    MidiSong4Ptr song;
    std::shared_ptr<Seq4<WidgetComposite>> seq4Comp;

    /**
     * state variables that affect drawing
     */
    bool _isSelected = false;
    std::string contentLength;
    int numNotes = 0;
    bool isPlaying = false;
    bool iAmNext = false;
    int repeatCount = 1;
    int repetitionNumber = 1;

    bool handleKey(int key, int mods, int action);
    void doCut();
    void doCopy();
    void doPaste();
    //  void doEditClip();
    MidiTrackPtr getTrack() const;
    MidiTrack4OptionsPtr getOptions() const;
    void invokeContextMenu();

    int getRepeatCountForUI();
    void setRepeatCountForUI(int);
    void otherItems(::rack::ui::Menu* menu);
};

/***************************************************************************
 * 
 * S4ButtonGrid
 * 
 * 
 * bridge between the widget and the buttons
 * 
 ****************************************************************************/

#include "MidiSong4.h"

class S4ButtonGrid {
public:
    void init(
        rack::app::ModuleWidget* widget,
        rack::engine::Module* module,
        MidiSong4Ptr s,
        std::shared_ptr<Seq4<WidgetComposite>> seq4Comp);
    void setNewSeq(MidiSequencer4Ptr newSeq);
    const static int buttonSize = 50.f;
    const static int buttonMargin = 10;

private:
    std::function<void(bool isCtrlKey)> makeButtonHandler(int row, int column);
    S4Button* getButton(int row, int col);
    S4Button* buttons[MidiSong4::numTracks][MidiSong4::numSectionsPerTrack] = {{}};
    void onClick(bool isCtrlKey, int row, int col);
    // void onEditClip();

    std::shared_ptr<Seq4<WidgetComposite>> seq4Comp;
};

inline S4Button* S4ButtonGrid::getButton(int row, int col) {
    assert(row >= 0 && row < 4 && col >= 0 && col < 4);
    return buttons[row][col];
}

inline void S4ButtonGrid::setNewSeq(MidiSequencer4Ptr newSeq) {
    for (int row = 0; row < MidiSong4::numTracks; ++row) {
        for (int col = 0; col < MidiSong4::numTracks; ++col) {
            buttons[row][col]->setNewSeq(newSeq);
        }
    }
}
