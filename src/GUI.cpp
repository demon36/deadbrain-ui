#include <DeadbrainCtrl.h>
#include <gtkmm.h>
#include <string>
#include "DeadbrainCtrl.h"

namespace GUI{

int currentDebugChannel = NUM_CHANNELS;//non-existent channel
int currentConfigChannel = 0;

Glib::RefPtr<Gtk::Application> GUIApp;
Gtk::ToggleButton* enabledToggleBtn = nullptr;
Gtk::SpinButton* scanTimeSpinBtn = nullptr;
Gtk::SpinButton* thresholdSpinBtn = nullptr;
Gtk::SpinButton* noteSpinBtn = nullptr;
Gtk::SpinButton* retriggerSpinBtn = nullptr;

int init(){
	GUIApp = Gtk::Application::create();
	Glib::RefPtr<Gtk::Builder> builder = Gtk::Builder::create_from_file("./res/GUI.glade");
	gtk_builder_connect_signals(builder->gobj(), NULL);

	Gtk::Window* mainWindow = nullptr;
	Gtk::ComboBoxText* debugChannelComboBox = nullptr;
	Gtk::ComboBoxText* configChannelComboBox = nullptr;

	builder->get_widget("mainWindow", mainWindow);
	builder->get_widget("debugChannelComboBox", debugChannelComboBox);
	builder->get_widget("configChannelComboBox", configChannelComboBox);
	builder->get_widget("enabledToggleBtn", enabledToggleBtn);
	builder->get_widget("scanTimeSpinBtn", scanTimeSpinBtn);
	builder->get_widget("thresholdSpinBtn", configChannelComboBox);
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

extern "C"{//signal handlers set using glade

void onMainWindowShow(){
	if(DeadbrainCtrl::tryOpenDevice()){
		DeadbrainCtrl::loadConfigFromDevice();
		refreshConfigValues();
	}else{
		Gtk::MessageDialog dialog("Failed to open Deadbrain midi device");
		dialog.run();
		GUIApp->quit();
	};
}

void onAppClose(){
	DeadbrainCtrl::closeDevice();
}

void onDebugChannelChange(GtkComboBox* widget, gpointer user_data){
	int selectedIndex = gtk_combo_box_get_active(widget);
	if(selectedIndex == currentDebugChannel){
		return;
	}

	if(currentDebugChannel != NUM_CHANNELS){//debugging was on
		DeadbrainCtrl::stopSignalDebug();
	}

	if(selectedIndex != NUM_CHANNELS){//debug turn on request
		DeadbrainCtrl::startSignalDebug(selectedIndex);
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
	DeadbrainCtrl::setChannelEnabled(currentConfigChannel, (bool)isActive);
}

void onScanTimeChange(GtkSpinButton* spin_button, gpointer user_data){
	int value = gtk_spin_button_get_value(spin_button);
	DeadbrainCtrl::setScanTime(currentConfigChannel, value);
}

void onThresholdChange(GtkSpinButton* spin_button, gpointer user_data){
	int value = gtk_spin_button_get_value(spin_button);
	DeadbrainCtrl::setHitThreshold(currentConfigChannel, value);
}

void onNoteChange(GtkSpinButton* spin_button, gpointer user_data){
	int value = gtk_spin_button_get_value(spin_button);
	DeadbrainCtrl::setNote(currentConfigChannel, value);
}

void onRetriggerChange(GtkSpinButton* spin_button, gpointer user_data){
	int value = gtk_spin_button_get_value(spin_button);
	DeadbrainCtrl::setRetriggerPeriod(currentConfigChannel, value);
}

}

}

int main(){
	GUI::init();
}
