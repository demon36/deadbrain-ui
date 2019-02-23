#include <stdint.h>
#include <sys_ex_defs.h>

namespace DeadbrainCtrl{

bool tryOpenDevice();
void loadConfigFromDevice();

const channel_cfg& getChannelConfig(int channelIndex);
void setChannelEnabled(int index, bool value);
void setScanTime(int channelIndex, int value);
void setHitThreshold(int channelIndex, int value);
void setNote(int channelIndex, int value);
void setRetriggerPeriod(int channelIndex, int value);
void startSignalDebug(int channelIndex);
void stopSignalDebug();

void closeDevice();

}
