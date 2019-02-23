#include <iostream>
#include <vector>
#include <memory>
#include <unistd.h>
#include <rtmidi/RtMidi.h>
#include <condition_variable>
#include <atomic>
#include <iomanip>

//#include <jackaudioio.hpp>
//#include <jackmidiport.hpp>

#include <libdeadbrain/config.h>

using namespace std;

#define DEADBRAIN_DEV_NAME "V-USB-MIDI"

namespace DeadbrainCtrl{

RtMidiIn deviceInputPort;
RtMidiOut deviceOutputPort;

deadbrain_config config;

vector<unsigned char> replyBuffer;
condition_variable deviceRepliedCondition;
std::mutex conditionMutex;
atomic<bool> requestPending(false);

void openInputPort(){
	RtMidiIn rtMidi(RtMidi::LINUX_ALSA);
	unsigned int nPorts = rtMidi.getPortCount();
	for ( unsigned int i=0; i<nPorts; i++ ) {
		if(rtMidi.getPortName(i).find(DEADBRAIN_DEV_NAME) != rtMidi.getPortName(i).npos){
			deviceInputPort.openPort(i);
			return;
		}
	}
	throw runtime_error("deadbrain device input port not found");
}

void openOutputPort(){
	RtMidiOut rtMidi;
	unsigned int nPorts = rtMidi.getPortCount();
	for ( unsigned int i=0; i<nPorts; i++ ) {
		if(rtMidi.getPortName(i).find(DEADBRAIN_DEV_NAME) != rtMidi.getPortName(i).npos){
			deviceOutputPort.openPort(i);
			return;
		}
	}
	throw runtime_error("deadbrain device output port not found");
}

void replyCallback(double timeStamp, std::vector<unsigned char>* reply, void *userData){
//	if(!requestPending){
//		return;
//	}

	for(int i = 0; i < reply->size(); i++){
		cout<<std::hex<<std::uppercase<<setfill('0')<<setw(2)<<(short)reply->at(i)<<" ";
	}
	cout<<endl;
//	reply->clear();
//
//	replyBuffer.insert(replyBuffer.end(), reply->begin(), reply->end());
//
//	if(reply->at(reply->size()-1) == 0xF7){
//		requestPending = false;
//		std::unique_lock<std::mutex> lck(conditionMutex);
//		deviceRepliedCondition.notify_all();
//	}
}


vector<unsigned char> sendRequest(const vector<unsigned char>& message){
	replyBuffer.clear();
	requestPending = true;
	deviceOutputPort.sendMessage(&message);
	std::unique_lock<std::mutex> lck(conditionMutex);
	deviceRepliedCondition.wait(lck);
	return replyBuffer;
}

vector<unsigned char> formRequest(const vector<unsigned char>& content){
	vector<unsigned char> requestBuffer({0xF0});
	for(auto& byte : content){
		requestBuffer.push_back(byte);
	}
	//append zeros so that content between F0 and F7 becomes 4
	while(requestBuffer.size() < 3){
		requestBuffer.push_back(0x00);
	}

	requestBuffer.push_back(0xF7);
	return requestBuffer;
}

/**
 * opens the deadbrain drum module device
 * @returns success status
 */
bool tryOpenDevice(){

	try{
		openInputPort();
		openOutputPort();
	}catch (RtMidiError &error) {
		error.printMessage();
		return false;
	}

	deviceInputPort.ignoreTypes(false, false, false);
	deviceInputPort.setCallback(replyCallback);
	vector<unsigned char> dummyMsg = {0xF0, 0xF7};
	//send dummy message because first coming packet always get lost
	deviceOutputPort.sendMessage(&dummyMsg);
	return true;
}

const deadbrain_config& loadConfigFromDevice(){
	vector<unsigned char> reply = sendRequest(formRequest(vector<unsigned char>{SYSEX_GET_CONFIG}));
	return config;
}

//
//bool isChannelEnabled(int index);
//void enableChannel(int index);
//
//void setScanTime(int channelIndex, int value);
//void setHitThreshold(int channelIndex, int value);
//void setNote(int channelIndex, int value);
//void setRetriggerPeriod(int channelIndex, int value);
//
//int getScanTime();
//int getHitThreshold();
//int getNote();
//int getRetriggerPeriod();

void closeDevice(){

}

}
