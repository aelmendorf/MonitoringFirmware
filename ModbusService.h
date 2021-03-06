#pragma once
#include <ArduinoSTL.h>
#include <SPI.h>
#include <Ethernet.h>
#include <ArduinoRS485.h>
#include <ArduinoModbus.h>
#include "Configuration.h"
#include "Data.h"

namespace MonitoringComponents {
	class ModbusService {
	public:
		ModbusService() :ethServer(502) {
		}

		static ModbusService* const Instance() {
			if (instance == nullptr) {
				instance = new ModbusService;
			}
			return instance;
		}

		static bool Connected() {
			return true;
		}

		static void Initialize(NetConfiguration netConfig) {
			//IPAddress ip(172, 20, 5, 178);
			//IPAddress subnet(255, 255, 0, 0);
			//IPAddress dns(172, 20, 3, 5);
			//IPAddress gateway(172, 20, 5, 1);
			//byte macAddress[6] = { 0x60, 0x52, 0xD0, 0x06, 0x70, 0x93 };
			auto instance = ModbusService::Instance();
			Ethernet.init(5);   //CS pin for P1AM-ETH
			Ethernet.setDnsServerIP(netConfig.dns);
			//Ethernet.setGatewayIP(netConfig.gateway);
			bool success = true;
			IPAddress subnet(255, 255, 255, 0);
			//Ethernet.begin(netConfig.mac,netConfig.ip,netConfig.dns,netConfig.gateway,subnet);
			int success=Ethernet.begin(netConfig.mac, 1000, 1000);

			//Ethernet.begin(netConfig.mac, netConfig.ip, netConfig.dns, netConfig.gateway);
			if (success) {
				instance->ethServer.begin();
				IPAddress address = Ethernet.localIP();
				address.printTo(Serial);
				std::cout << std::endl;
				if (!instance->modbusServer.begin()) {
					//log error
					instance->initialized = false;
				} else {
					instance->initialized = true;
				}
				instance->modbusServer.configureCoils(0, netConfig.coils);
				instance->modbusServer.configureDiscreteInputs(0, netConfig.inputRegisters);
			} else {
				instance->initialized = false;
			}
		}

		static void Poll() {
			auto instance = ModbusService::Instance();
			if (instance->initialized) {
				EthernetClient client = instance->ethServer.available();
				if (client) {
					instance->modbusServer.accept(client);
					while (client.connected()) {
						instance->modbusServer.poll();
					}
				}
			}
		}

		static void UpdateHoldingRegister(int addr, uint16_t value) {
			auto instance = ModbusService::Instance();
			if (instance->initialized) {
				instance->modbusServer.holdingRegisterWrite(addr, value);
			}
		}

		static void UpdateInputRegister(int addr, uint16_t value) {
			auto instance = ModbusService::Instance();
			if (instance->initialized) {
				instance->modbusServer.inputRegisterWrite(addr, value);
			}
		}

		static void UpdateCoil(int addr, bool value) {
			auto instance = ModbusService::Instance();
			if (instance->initialized) {
				instance->modbusServer.coilWrite(addr, value);
			}
		}

		static void UpdateDiscreteInput(int addr, bool value) {
			auto instance = ModbusService::Instance();
			if (instance->initialized) {
				instance->modbusServer.discreteInputWrite(addr, value);
			}

		}

		//static void ReadHoldingRegister(int addr);
		//static void ReadInputRegister(int addr);
		//static void ReadCoil(int addr);
		//static void ReadDiscreteInput(int addr);

	private:
		static ModbusService* instance;
		EthernetServer ethServer;
		ModbusTCPServer modbusServer;
		bool initialized = false;
	};
};



