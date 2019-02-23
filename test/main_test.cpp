#include <iostream>
#include <assert.h>
#include <libdeadbrain/config.h>
#include <unistd.h>
#include "midi.h"

extern int midi_main();
extern int raw_test();
extern void libusb_test();

using namespace std;

int libdb_main(){
	assert(DeadbrainCtrl::tryOpenDevice());
//	DeadbrainCtrl::loadConfigFromDevice();
//	cout<<DeadbrainCtrl::isChannelEnabled(0)<<"\n";
//	DeadbrainCtrl::enableChannel(0);
//	assert(DeadbrainCtrl::isChannelEnabled(0));
	DeadbrainCtrl::loadConfigFromDevice();
	DeadbrainCtrl::closeDevice();
}

int main(){
//	midi_main();
//	raw_test();
//	libusb_test();
	libdb_main();
	return 0;
}
