#include "DeadbrainCtrl.h"

#include <iostream>
#include <vector>
#include <unistd.h>
#include <rtmidi/RtMidi.h>
#include <condition_variable>
#include <atomic>
#include <iomanip>

using namespace std;

#define DEADBRAIN_DEV_NAME "V-USB-MIDI"

namespace DeadbrainCtrl{

RtMidiIn deviceInputPort;
RtMidiOut deviceOutputPort;

DeadBrainConfig config;

vector<unsigned char> replyBuffer;
condition_variable deviceRepliedCondition;
std::mutex conditionMutex;
atomic<bool> requestPending(false);

void replyCallback(double timeStamp, std::vector<unsigned char>* reply, void *userData){
	bool debugMessage = false;

	if(reply->at(1) == SYSEX_DEBUG_MSG){
		cout<<"DEBUG MSG ** ";
		debugMessage = true;
	}
	for(size_t i = 0; i < reply->size(); i++){
		cout<<std::hex<<std::uppercase<<setfill('0')<<setw(2)<<(short)reply->at(i)<<" ";
	}
	cout<<endl;

	if(!requestPending){
		cout<<"no pending requests, ignoring message"<<endl;
		return;
	}

	if(debugMessage){
		return;
	}

	replyBuffer.insert(replyBuffer.end(), reply->begin(), reply->end());
	if(reply->back() == 0xF7){
		requestPending = false;
		std::unique_lock<std::mutex> lck(conditionMutex);
		deviceRepliedCondition.notify_all();
	}
}


vector<unsigned char> sendRequest(const vector<unsigned char>& message){
	if(requestPending){
		throw runtime_error("sending a new request while another request is still pending");
	}

	replyBuffer.clear();
	requestPending = true;

	vector<unsigned char> requestBuffer({0xF0});
	for(auto& byte : message){
		requestBuffer.push_back(byte);
	}
	requestBuffer.push_back(0xF7);

	deviceOutputPort.sendMessage(&requestBuffer);
	std::unique_lock<std::mutex> lck(conditionMutex);
	deviceRepliedCondition.wait(lck);
	return replyBuffer;
}

/**
 * opens the deadbrain drum module device
 * @returns success status
 */
bool tryOpenDevice(){
	try{
		RtMidiIn rtMidiIn, rtMidiOut;
		unsigned int nPorts = rtMidiIn.getPortCount();
		for ( unsigned int i=0; i<nPorts; i++ ) {
			if(rtMidiIn.getPortName(i).find(DEADBRAIN_DEV_NAME) != rtMidiIn.getPortName(i).npos){
				deviceInputPort.openPort(i);
			}
		}

		nPorts = rtMidiOut.getPortCount();
		for ( unsigned int i=0; i<nPorts; i++ ) {
			if(rtMidiOut.getPortName(i).find(DEADBRAIN_DEV_NAME) != rtMidiOut.getPortName(i).npos){
				deviceOutputPort.openPort(i);
			}
		}

		if(!deviceInputPort.isPortOpen()){
			throw runtime_error("deadbrain device input port not found");
		}

		if(!deviceOutputPort.isPortOpen()){
			throw runtime_error("deadbrain device output port not found");
		}

	}catch (exception &ex) {
		cerr<<ex.what()<<endl;
		return false;
	}

	deviceInputPort.ignoreTypes(false, true, true);
	deviceInputPort.setCallback(replyCallback);
	vector<unsigned char> dummyMsg = {0xF0, 0xF7};
	//send dummy message because first coming packet always get lost
//	deviceOutputPort.sendMessage(&dummyMsg);
//	sleep(1);
	return true;
}

void loadConfigFromDevice(){
	vector<unsigned char> reply = sendRequest({SYSEX_GET_CONFIG});
	DeadBrainConfig* tempConfigPtr = reinterpret_cast<DeadBrainConfig*>(&reply[1]);//escape 0xF0
	for(size_t i = 0; i < NUM_CHANNELS; i++){
		config[i] = (*tempConfigPtr)[i];
	}
}

const channel_cfg& getChannelConfig(int channelIndex){
	return config[channelIndex];
}

bool setChannelEnabled(unsigned char index, unsigned char value){
	if(config[index].enabled == value){
		return true;
	}
	config[index].enabled = value;
	return sendRequest({SYSEX_SET_ENABLED, index, value})[1] == SYSEX_REPLY_OK;
}

bool setScanTime(unsigned char channelIndex, unsigned char value){
	if(config[channelIndex].scan_time == value){
		return true;
	}
	config[channelIndex].scan_time = value;
	return sendRequest({SYSEX_SET_SCAN_TIME, channelIndex, value})[1] == SYSEX_REPLY_OK;
}

bool setHitThreshold(unsigned char channelIndex, unsigned char value){
	if(config[channelIndex].threshold == value){
		return true;
	}
	config[channelIndex].threshold = value;
	return sendRequest({SYSEX_SET_THRESHOLD, channelIndex, value})[1] == SYSEX_REPLY_OK;
}
bool setNote(unsigned char channelIndex, unsigned char value){
	if(config[channelIndex].note == value){
		return true;
	}
	config[channelIndex].note = value;
	return sendRequest({SYSEX_SET_NOTE, channelIndex, value})[1] == SYSEX_REPLY_OK;
}

bool setRetriggerPeriod(unsigned char channelIndex, unsigned char value){
	if(config[channelIndex].retrigger == value){
		return true;
	}
	config[channelIndex].retrigger = value;
	return sendRequest({SYSEX_SET_RETRIGGER, channelIndex, value})[1] == SYSEX_REPLY_OK;
}
bool saveConfigOnDevice(){
	return sendRequest({SYSEX_SAVE_CONFIG})[1] == SYSEX_REPLY_OK;
}

bool startSignalDebug(unsigned char channelIndex){
	return sendRequest({SYSEX_DEBUG_START, channelIndex})[1] == SYSEX_REPLY_OK;
}

bool stopSignalDebug(){
	return sendRequest({SYSEX_DEBUG_END})[1] == SYSEX_REPLY_OK;
}

void closeDevice(){
	deviceInputPort.closePort();
	deviceOutputPort.closePort();
}

}
