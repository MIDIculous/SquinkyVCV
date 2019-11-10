#include "InputScreen.h"
#include "PitchInputWidget.h"
#include "UIPrefs.h"

using Vec = ::rack::math::Vec;
//using Button = ::rack::ui::Button;
using Widget = ::rack::widget::Widget;
using Label = ::rack::ui::Label;

PitchInputWidget::PitchInputWidget(
    const ::rack::math::Vec& pos, 
    const ::rack::math::Vec& siz,
    const std::string& label, 
    bool relative,
    std::vector<InputControl*>& inputControls)
{
    box.pos = pos;
    // TODO: we should set our own height
    box.size = siz;

    DEBUG("adding pitch input widget at pos= %.2f,%.2f size=%.2f,%.2f", pos.x, pos.y, siz.x, siz.y);

    // Make the input controls for octave, pitch, and constrain
    // TODO: these are leaking
    // TODO2: these are all fakes
    inputControls.push_back(new InputControlFacade());
    inputControls.push_back(new InputControlFacade());
    inputControls.push_back(new InputControlFacade());


    // add label
    float x = InputScreen::centerColumn;
    float y = 0;
    
    addMainLabel(label, Vec(x, y));
  
    // add octave
    addOctaveControl(Vec(x, y));

    // add chromatic semi

    // add scale degrees

    // add checkbox
}

void PitchInputWidget::addMainLabel(const std::string& labelText, const ::rack::math::Vec& pos)
{
    Label* labelCtrl = new Label();
    labelCtrl->box.pos = Vec(0, pos.y);
    labelCtrl->text = labelText.c_str();
    labelCtrl->color = UIPrefs::XFORM_TEXT_COLOR;
    this->addChild(labelCtrl);
    DEBUG("adding label at %.2f,%.2f", labelCtrl->box.pos.x, labelCtrl->box.pos.y);

    labelCtrl->box.size.x = pos.x - 10;
    DEBUG(" label text = %s, size=%.2f,%.2f", labelText.c_str(), labelCtrl->box.size.x, labelCtrl->box.size.y); 
    labelCtrl->alignment = Label::RIGHT_ALIGNMENT;
   
}

static std::vector<std::string> octavesRel = {
    "+7 oct", "+6 oct", "+5 oct",
    "+4 oct", "+3 oct", "+2 oct", "+1 oct",
    "+0 oct",
    "-1 oct", "-2 oct", "-3 oct", "-4 oct",
    "-5 oct", "-6 oct", "-7 oct"
};

static std::vector<std::string> semisRel = {
    "+12 semi", "+11 semi", "+10 semi","+9 semi",
    "+8 semi", "+7 semi", "+6 semi","+5 semi",
    "+4 semi", "+3 semi", "+2 semi","+1 semi",
     "+0 semi", 
     "-1 semi", "-2 semi", "-3 semi","-4 semi",
     "-5 semi", "-6 semi", "-7 semi","-8 semi",
     "-9 semi","-10 semi","-11 semi", "-12 semi"
};

void PitchInputWidget::addOctaveControl(const ::rack::math::Vec& pos)
{
    DEBUG("adding octaves at %.2f,%.2f", pos.x, pos.y);
    auto pop = new InputPopupMenuParamWidget();
    pop->setLabels( octavesRel);
    pop->box.size.x = 76;    // width
    pop->box.size.y = 22;     // should set auto like button does
    pop->setPosition(pos);
    pop->text = "+0 oct";
    this->addChild(pop);

    //inputControls.push_back(pop);
}

PitchInputWidget::InputControlFacade::~InputControlFacade()
{

}
float PitchInputWidget::InputControlFacade::getValue() const
{
    return 0;
}

void PitchInputWidget::InputControlFacade::setValue(float) 
{

}

void PitchInputWidget::InputControlFacade::enable(bool enabled) 
{

}


void PitchInputWidget::InputControlFacade::setCallback(std::function<void(void)>) 
{

}

  