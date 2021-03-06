#pragma once
#include <ArduinoSTL.h>
#include <map>
#include <ArduinoModbus.h>
#include <ArduinoRS485.h>
#include "Timer.h"
#include "Configuration.h"
#include "ConfigurationReader.h"
#include "Action.h"
#include "ModbusService.h"


namespace MonitoringComponents {

	enum class ControllerState {
		Alarming,Warning,Maintenance,Okay
	};

	enum class Transition {
		To,
		From
	};

	struct SystemActionIds {
		int alarmIndex = 0;
		int warningIndex = 0;
		int okayIndex = 0;
		int softWarnIndex = 0;
		int maintenanceIndex = 0;

		void setId(ActionType type,int i) {
			switch (type) {
				case ActionType::Alarm: {
					this->alarmIndex = i;
					break;
				}
				case ActionType::Maintenance: {
					this->maintenanceIndex = i;
					break;
				}
				case ActionType::Warning: {
					this->warningIndex = i;
					break;
				}
				case ActionType::SoftWarn: {
					this->softWarnIndex = i;
					break;
				}
				case ActionType::Okay: {
					this->okayIndex = i;
					break;
				}
				default: {
					break;
				}
			}
		}
		int getId(ActionType type) {
			switch (type) {
				case ActionType::Alarm: {
					return this->alarmIndex;
				}
				case ActionType::Maintenance: {
					return this->alarmIndex;
				}
				case ActionType::Warning: {
					return this->alarmIndex;
				}
				case ActionType::SoftWarn: {
					return this->alarmIndex;
				}
				case ActionType::Okay: {
					return this->alarmIndex;
				}
				default: {
					return -1;
				}
			}
		}
	};

	class MonitoringController:public MonitoringComponent {
	public:
		MonitoringController() :MonitoringComponent(), _on_channel_cbk([](ChannelMessage) {}){}
		void Setup();
		void Run();
		void Initialize();
	private:
		void Build();
		void OnChannelCallback(ChannelCallback cbk);
		void ProcessChannelMessage(ChannelMessage channelMessage);
		void ProcessStateChanges();
		void InvokeSystemAction(ActionType actionType);
	private:
		typedef std::vector<ChannelAddress> Registrations;
		std::vector<DiscreteInputChannel*> discreteInputs;
		std::vector<AnalogInputChannel*> analogInputs;
		std::vector<DiscreteOutputChannel*> outputChannels;

		std::vector<Action*> actions;
		//std::map<ActionType, Action*> actionMap;

		std::map<int,Registrations*> actionTracking;
		std::map<ActionType, int> systemActMap;
		std::map<ActionType, bool> systemActionLatches;

		ChannelCallback _on_channel_cbk;
		ControllerState controllerState;
		ControllerState nextState;
		//Timer resetTimer;
		Timer printTimer;
		Timer checkStateTimer;
		Timer resetTimer;

		void privateLoop();
	};
};



