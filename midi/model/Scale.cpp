#include "PitchUtils.h"
#include "Scale.h"
#include "ScaleRelativeNote.h"

#include <assert.h>


Scale::Scale()
{
}

void Scale::init(Scales scale, int keyRoot)
{
    std::vector<int> notes = getBasePitches(scale);
    int degree = 0;
    for (auto it : notes) {
        const int semi = keyRoot + it;
        // how do we come up with the 
        ScalePtr sc = getptr();
        ScaleRelativeNotePtr srn = std::make_shared<ScaleRelativeNote>(degree, 0, sc);
        abs2srn[semi] = srn;
        ++degree;
    }
}

ScalePtr Scale::getScale(Scale::Scales scale, int root)
{
    ScalePtr p(new Scale());

    p->init(scale, root);
    return ScalePtr(p);
}

ScaleRelativeNotePtr Scale::getScaleRelativeNote(int semitone)
{
    assert(abs2srn.size());         // was this initialized?
    PitchUtils::NormP normP(semitone);

    auto it = abs2srn.find(normP.semi);
    if (it != abs2srn.end()) {
        ScalePtr scale = shared_from_this();
        return ScaleRelativeNotePtr(new ScaleRelativeNote(it->second->degree, normP.oct, scale));
    }

    // since these are semi-normaled, lets try the next octave
    it = abs2srn.find(normP.semi + 12);
    if (it != abs2srn.end()) {    
        ScalePtr scale = shared_from_this();
        return ScaleRelativeNotePtr(new ScaleRelativeNote(it->second->degree, normP.oct - 1, scale));
    }

    // need to make an invalid one
    return ScaleRelativeNotePtr(new ScaleRelativeNote());
}

int Scale::getSemitone(const ScaleRelativeNote& note)
{
    // TODO: make smarter
    for (auto it : abs2srn) {
        int semi = it.first;
        ScaleRelativeNotePtr srn = it.second;
        if (srn->degree == note.degree) {
           // printf("found it!!\n");
           // assert(note.octave == 0);
            return semi + 12 * note.octave;
        }
    }
    //printf("didn't find it\n");
    return -1;
}


std::vector<int> Scale::getBasePitches(Scales scale)
{
    std::vector<int> ret;
    switch(scale) {
        case Scales::Major:
            ret = {0, 2, 4, 5, 7, 9, 11};
            break;
        case Scales::Minor:
            ret = {0, 2, 3, 5, 7, 8, 10};
            break;
        case Scales::Phrygian:
            ret = {0, 1, 3, 5, 7, 8, 10};
            break;
        case Scales::Mixolydian:
            ret = {0, 2, 4, 5, 7, 9, 10};
            break;
        case Scales::Locrian:
            ret = {0, 1, 3, 5, 6, 8, 10};
            break;
        case Scales::Lydian:
            ret = {0, 2, 4, 6, 7, 9, 11};
            break;
        case Scales::Dorian:
            ret = {0, 2, 3, 5, 7, 9, 10};
            break;
        default:
            assert(false);
     }
    return ret;
}

 int Scale::degreesInScale() const
 {
     return int(abs2srn.size());
 }

int Scale::transposeInScale(int semitone, int scaleDegreesToTranspose)
{
    auto srn = this->getScaleRelativeNote(semitone);
    if (!srn->valid) {
        return transposeInScaleChromatic(semitone, scaleDegreesToTranspose);
    }

    int transposedOctave = srn->octave;
    int transposedDegree = srn->degree;

    transposedDegree += scaleDegreesToTranspose;
    while (transposedDegree >= degreesInScale()) {
        // I think this is wrong!
        assert(false);
        transposedDegree -= degreesInScale();
        transposedOctave++;
    }

    while (transposedDegree < 0) {
        transposedDegree += degreesInScale();
        transposedOctave--;
    }

    auto srn2 = std::make_shared<ScaleRelativeNote>(transposedDegree, transposedOctave, shared_from_this());
    return this->getSemitone(*srn2); 
}

int Scale::transposeInScaleChromatic(int semitone, int scaleDegreesToTranspose)
{
    assert(!getScaleRelativeNote(semitone)->valid);

// TODO: make this debug only
    auto srnPrev = getScaleRelativeNote(semitone-1);
    auto srnNext = getScaleRelativeNote(semitone-1);

    // For all the scales we have so far, notes out of scale are
    // always surrounded by notes in scale. Not true for all, however.
    assert(srnPrev->valid && srnNext->valid);

    // If we can fit between these, we will.
    // If now, we will always round down.
    const int transposePrev = transposeInScale(semitone-1, scaleDegreesToTranspose);
    const int transposeNext = transposeInScale(semitone+1, scaleDegreesToTranspose);
    return (transposePrev + transposeNext) / 2;
}


XformLambda Scale::makeTransposeLambdaChromatic(int transposeSemitones)
{
    assert(false);
    return nullptr;
}
XformLambda Scale::makeTransposeLambdaScale(int scaleDegrees, int keyRoot, Scales mode)
{
assert(false);
    return nullptr;
}