#include <sys_ex_defs.h>

namespace DeadbrainCtrl{

bool tryOpenDevice();
const deadbrain_config& loadConfigFromDevice();

bool isChannelEnabled(int index);
void enableChannel(int index);

void setScanTime(int channelIndex, int value);
void setHitThreshold(int channelIndex, int value);
void setNote(int channelIndex, int value);
void setRetriggerPeriod(int channelIndex, int value);

int getScanTime();
int getHitThreshold();
int getNote();
int getRetriggerPeriod();

void closeDevice();

}
