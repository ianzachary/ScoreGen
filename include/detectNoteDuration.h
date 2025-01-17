#ifndef NOTE_DETECTION_H
#define NOTE_DETECTION_H

#define _USE_MATH_DEFINES

#include <vector>
#include <iostream>
#include <cmath>
#include <map>
#include <string>
#include <sndfile.h>
#include <fftw3.h>

using namespace std;

struct Note {
    float startTime; // seconds
    float endTime;   // seconds
    string pitch;
    string noteType;
};

string getNoteName(double freq);
string determineNoteType(float noteDuration, int bpm, int beatsPerBar);
string detectPitch(vector<float> buf, size_t start, size_t end, int sample_rate);
vector<Note> detectNotes(const vector<float>& buf, int sample_rate, int channels);

#endif
