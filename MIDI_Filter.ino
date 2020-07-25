/*
  Firmware for MIDIchlorian filters/thru boxes
  Copyright (C) 2020 Filip Wieland

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include <timer.h>
#include <MIDI.h>
#include <SoftwareSerial.h>

using namespace MIDI_NAMESPACE;

Timer timer;

SerialMIDI<HardwareSerial> serialMidiIn(Serial);
MidiInterface<SerialMIDI<HardwareSerial>> midiIn(serialMidiIn);

SoftwareSerial serialOut(4, 5);
SerialMIDI<SoftwareSerial> serialMidiOut(serialOut);
MidiInterface<SerialMIDI<SoftwareSerial>> midiOut(serialMidiOut);

const auto SWITCH_FILTER_TRANSPORT = 10;
const auto SWITCH_FILTER_VELOCITY = 11;
const auto SWITCH_FILTER_CONTROL = 12;

typedef Message<MIDI_NAMESPACE::DefaultSettings::SysExMaxSize> MidiMessage;
void handleMessage(const MidiMessage &message);

void ledOn();
void ledOff();

void setup() {
  // Status LED to tell us when events are being received
  pinMode(LED_BUILTIN, OUTPUT);

  // Switches to figure out just what sort of stuff
  pinMode(SWITCH_FILTER_TRANSPORT, INPUT_PULLUP);
  pinMode(SWITCH_FILTER_VELOCITY, INPUT_PULLUP);
  pinMode(SWITCH_FILTER_CONTROL, INPUT_PULLUP);

  // Set up an interface to receive MIDI from on all channels
  midiIn.begin(MIDI_CHANNEL_OMNI);
  midiIn.turnThruOff();

  // Handles incoming messages and performs filtering
  midiIn.setHandleMessage(handleMessage);

  // Set up an interface to send filtered MIDI packets to; this
  // will be split and then buffered up with an optocoupler/transistor
  // pair to drive several outputs in parallel
  midiOut.begin(MIDI_CHANNEL_OMNI);
  midiOut.turnThruOff();

  timer.setCallback(ledOff);
}

void loop() {
  // Call this as often as possible to minimise latency
  midiIn.read();
  timer.update();
}

void handleMessage(const MidiMessage &message) {
  // Read the toggle switches
  bool allowTransport = (digitalRead(SWITCH_FILTER_TRANSPORT) == LOW);
  bool allowVelocity = (digitalRead(SWITCH_FILTER_VELOCITY) == LOW);
  bool allowControl = (digitalRead(SWITCH_FILTER_CONTROL) == LOW);

  // Copy the message as we will be modifying it
  MidiMessage outMessage;
  outMessage.channel = message.channel;
  outMessage.type = message.type;
  outMessage.data1 = message.data1;
  outMessage.data2 = message.data2;
  outMessage.valid = message.valid;

  // Perform filtering actions based on the switches
  switch (message.type) {
    // Always filter out system exclusive messages.
    // If you want to load samples to your sampler,
    // connect it to a computer directly!
    case SystemExclusive:
    case SystemExclusiveEnd:
      outMessage.valid = false;
      break;

    // If velocity is disabled, all notes passing through
    // will be set to full velocity
    case NoteOn:
    case NoteOff:
      if (!allowVelocity) {
        ledOn();
        outMessage.data2 = 127;
      }
      break;

    // If control codes are disabled, parameter automation
    // messages will be stripped out and your MIDI controller's
    // knobs won't work on the devices attached to the filter.
    // Also prevents bank switching, so there's that.
    case ControlChange:
    case ProgramChange:
      if (!allowControl) {
        ledOn();
        outMessage.valid = false;
      }
      break;

    // If transport controls are disabled, messages that
    // start, stop, or reset sequencers will be stripped out.
    // Perfect if you don't want to interrupt the sequence
    // running on your synth on channel 1 by pressing stop
    // on your arturia keystep or similar device.
    case Start:
    case Continue:
    case Stop:
    case SystemReset:
      if (!allowTransport) {
        ledOn();
        outMessage.valid = false;
      }
      break;

    default:
      break;
  }

  // Sends the message out into the world
  if (outMessage.valid) {
    midiOut.send(outMessage.type, outMessage.data1, outMessage.data2, outMessage.channel);
  }
}

void ledOn() {
  digitalWrite(LED_BUILTIN, HIGH);
  timer.setTimeout(100);
  timer.start();
}

void ledOff() {
  digitalWrite(LED_BUILTIN, LOW);
}
