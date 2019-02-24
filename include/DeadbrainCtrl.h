#include <stdint.h>
#include <sys_ex_defs.h>

namespace DeadbrainCtrl{

bool tryOpenDevice();
void loadConfigFromDevice();

const channel_cfg& getChannelConfig(int channelIndex);
bool setChannelEnabled(unsigned char index, unsigned char value);
bool setScanTime(unsigned char channelIndex, unsigned char value);
bool setHitThreshold(unsigned char channelIndex, unsigned char value);
bool setNote(unsigned char channelIndex, unsigned char value);
bool setRetriggerPeriod(unsigned char channelIndex, unsigned char value);
bool saveConfigOnDevice();
bool startSignalDebug(unsigned char channelIndex);
bool stopSignalDebug();

void closeDevice();

}
