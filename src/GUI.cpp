#include <DeadbrainCtrl.h>
#include <gtkmm.h>
#include <string>
#include "DeadbrainCtrl.h"

namespace GUI{

int currentDebugChannel = NUM_CHANNELS;//non-existent channel
int currentConfigChannel = 0;

Glib::RefPtr<Gtk::Application> GUIApp;
Gtk::Window* mainWindow = nullptr;
Gtk::ToggleButton* enabledToggleBtn = nullptr;
Gtk::SpinButton* scanTimeSpinBtn = nullptr;
Gtk::SpinButton* thresholdSpinBtn = nullptr;
Gtk::SpinButton* noteSpinBtn = nullptr;
Gtk::SpinButton* retriggerSpinBtn = nullptr;

int init(){
	GUIApp = Gtk::Application::create();
	Glib::RefPtr<Gtk::Builder> builder = Gtk::Builder::create_from_file("./res/GUI.glade");
	gtk_builder_connect_signals(builder->gobj(), NULL);


	Gtk::ComboBoxText* debugChannelComboBox = nullptr;
	Gtk::ComboBoxText* configChannelComboBox = nullptr;

	builder->get_widget("mainWindow", mainWindow);
	builder->get_widget("debugChannelComboBox", debugChannelComboBox);
	builder->get_widget("configChannelComboBox", configChannelComboBox);
	builder->get_widget("enabledToggleBtn", enabledToggleBtn);
	builder->get_widget("scanTimeSpinBtn", scanTimeSpinBtn);
	builder->get_widget("thresholdSpinBtn", thresholdSpinBtn);
	builder->get_widget("noteSpinBtn", noteSpinBtn);
	builder->get_widget("retriggerSpinBtn", retriggerSpinBtn);

	for(size_t i = 0; i < NUM_CHANNELS; i++){
		debugChannelComboBox->append(std::to_string(i).c_str());
		configChannelComboBox->append(std::to_string(i).c_str());
	}
	debugChannelComboBox->append("Off");
	debugChannelComboBox->set_active(currentDebugChannel);//last item: Off
	configChannelComboBox->set_active(currentConfigChannel);
	return GUIApp->run(*mainWindow);
}

void refreshConfigValues(){
	const channel_cfg& cfg = DeadbrainCtrl::getChannelConfig(currentConfigChannel);
	enabledToggleBtn->set_active(cfg.enabled);
	scanTimeSpinBtn->set_value(cfg.scan_time);
	thresholdSpinBtn->set_value(cfg.threshold);
	noteSpinBtn->set_value(cfg.note);
	retriggerSpinBtn->set_value(cfg.retrigger);
}

void displayErrMessage(std::string value){
	Gtk::MessageDialog dialog(value.c_str());
	dialog.set_transient_for(*mainWindow);
	dialog.run();
}

extern "C"{//signal handlers set using glade

void onMainWindowShow(){
	if(DeadbrainCtrl::tryOpenDevice()){
		DeadbrainCtrl::loadConfigFromDevice();
		refreshConfigValues();
	}else{
		displayErrMessage("Failed to open Deadbrain midi device");
		GUIApp->quit();
	};
}

void onAppClose(){
	//TODO: make sure this function gets called
	DeadbrainCtrl::closeDevice();
}

void onDebugChannelChange(GtkComboBox* widget, gpointer user_data){
	int selectedIndex = gtk_combo_box_get_active(widget);
	if(selectedIndex == currentDebugChannel){
		return;
	}

	if(currentDebugChannel != NUM_CHANNELS){//debugging was on
		if(!DeadbrainCtrl::stopSignalDebug()){
			displayErrMessage("Failed to stop debugging");
			gtk_combo_box_set_active(widget, currentDebugChannel);
			return;
		}
	}

	if(selectedIndex != NUM_CHANNELS){//debug turn on request
		if(!DeadbrainCtrl::startSignalDebug(selectedIndex)){
			displayErrMessage("Failed to start debugging");
			gtk_combo_box_set_active(widget, currentDebugChannel);
			return;
		}
	}

	currentDebugChannel = selectedIndex;
}

void onConfigChannelChange(GtkComboBox* widget, gpointer user_data){
	int selectedIndex = gtk_combo_box_get_active(widget);
	currentConfigChannel = selectedIndex;
	refreshConfigValues();
}

void onChannelEnabledChange(GtkToggleButton* togglebutton, gpointer user_data){
	int isActive = gtk_toggle_button_get_active(togglebutton);
	if(!DeadbrainCtrl::setChannelEnabled(currentConfigChannel, (bool)isActive)){
		displayErrMessage("Failed to enable selected channel");
	}
}

void onScanTimeChange(GtkSpinButton* spin_button, gpointer user_data){
	int value = gtk_spin_button_get_value(spin_button);
	if(!DeadbrainCtrl::setScanTime(currentConfigChannel, value)){
		displayErrMessage("Failed to change scan time of selected channel");
	}
}

void onThresholdChange(GtkSpinButton* spin_button, gpointer user_data){
	int value = gtk_spin_button_get_value(spin_button);
	if(!DeadbrainCtrl::setHitThreshold(currentConfigChannel, value)){
		displayErrMessage("Failed to change threshol of selected channel");
	}
}

void onNoteChange(GtkSpinButton* spin_button, gpointer user_data){
	int value = gtk_spin_button_get_value(spin_button);
	if(!DeadbrainCtrl::setNote(currentConfigChannel, value)){
		displayErrMessage("Failed to change midi note of selected channel");
	}
}

void onRetriggerChange(GtkSpinButton* spin_button, gpointer user_data){
	int value = gtk_spin_button_get_value(spin_button);
	if(!DeadbrainCtrl::setRetriggerPeriod(currentConfigChannel, value)){
		displayErrMessage("Failed to change re-trigger period of selected channel");
	}
}

void onSaveBtnClick(){
	if(!DeadbrainCtrl::saveConfigOnDevice()){
		displayErrMessage("Failed to change save configuration on device EEPROM");
	}
}

void onExitBtnClick(){
	GUIApp->quit();
}

}

}

int main(){

	DeadBrainConfig settings = {{0}};
	settings[0].scan_time = 8;
	uint8_t* config_ptr = (uint8_t*)settings;
	uint8_t s = config_ptr[1];
	uint8_t len = sizeof(DeadBrainConfig);

	GUI::init();
}
