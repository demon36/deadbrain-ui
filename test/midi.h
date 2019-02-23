#ifndef MIDI_H_
#define MIDI_H_

#include <stdint.h>

void midi_qpush_note_msg(uint8_t channel, uint8_t velocity);
void midi_qpush_sysex_msg(uint8_t data0, uint8_t data1);
void midi_qpop_qpacket();//pops a message from queue and sends it to host

void midi_qprint();


#endif /* MIDI_H_ */
