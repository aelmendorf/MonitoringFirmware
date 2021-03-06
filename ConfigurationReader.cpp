#include "ConfigurationReader.h"

#define AnalogFile              "analog.txt"
#define OutputFile              "output.txt"
#define DigitalFile             "digital.txt"
#define ModuleFile              "module.txt"
#define ActionFile              "actions.txt"
#define NetConfigFile           "network.txt"
#define SizeFile                "size.txt"
#define LOG_FILENAME            "log.txt"

namespace MonitoringComponents {

    ConfigurationReader::ConfigurationReader(Logger* logger) {
        this->logger = logger;
        this->configLoaded = false;
        this->DigitalInSize = 0;
        this->OutputSize = 0;
        this->AnalogInSize = 0;
        this->ModuleSize = 0;
        this->NetConfigSize = 0;
    }

    ConfigurationReader::ConfigurationReader() {
        this->configLoaded = false;
        this->logger = nullptr;
        this->DigitalInSize = 0;
        this->OutputSize = 0;
        this->AnalogInSize = 0;
        this->ModuleSize = 0;
        this->NetConfigSize = 0;
    }

    void ConfigurationReader::PrintSizes() {
        std::cout << AnalogFile << " " << this->AnalogInSize << std::endl;
        std::cout << OutputFile << " " << this->OutputSize << std::endl;
        std::cout << DigitalFile << " " << this->DigitalInSize << std::endl;
        std::cout << ActionFile << " " << this->ActionSize << std::endl;
        std::cout << NetConfigFile << " " << this->NetConfigSize << std::endl;
    }

    void ConfigurationReader::GetFileSizes() {
        File file = SD.open(SizeFile);
        String value = "";
        int lineCount = 0;
        if (file) {
            std::cout << "File Opened" << std::endl;
            char ch;
            while (file.available()) {
                ch = file.read();
                if (ch == '\n') {
                    this->SetSize(lineCount, value.toInt());
                    lineCount++;
                    value = "";
                    if (lineCount > 5) {
                        break;
                    }
                }else {
                    value += ch;
                }
            }
            file.close();
        }else {
            //std::cout << "File Open Failed" << std::endl;
        }
    }

    void ConfigurationReader::CreatConfigFiles(){
        if (!SD.begin(SDCARD_SS_PIN)) {
            while (1);
        }
        std::cout << "SD Card Initialized" << std::endl;
        File file;
        file = SD.open(AnalogFile,FILE_WRITE);
        if (file) {
            std::cout << AnalogFile << " Created!" << std::endl;
            file.close();
        }
        file = SD.open(DigitalFile,FILE_WRITE);
        if (file) {
            std::cout << DigitalFile << " Created!" << std::endl;
            file.close();
        }
        file = SD.open(ActionFile,FILE_WRITE);
        if (file) {
            std::cout << ActionFile << " Created!" << std::endl;
            file.close();
        }
        file = SD.open(OutputFile,FILE_WRITE);
        if (file) {
            std::cout << OutputFile << " Created!" << std::endl;
            file.close();
        }
        file = SD.open(NetConfigFile,FILE_WRITE);
        if (file) {
            std::cout << NetConfigFile << " Created!" << std::endl;
            file.close();
        }

        file = SD.open(SizeFile, FILE_WRITE);
        if (file) {
            std::cout << SizeFile << " Created!" << std::endl;
            file.close();
        }

        file = SD.open(LOG_FILENAME, FILE_WRITE);
        if (file) {
            std::cout << LOG_FILENAME << " Created!" << std::endl;
            file.close();
        }

        std::cout << "Files should be created..." << std::endl;
    }

    char ConfigurationReader::nibbleTobyte(char c){
        if ((c >= '0') && (c <= '9'))
            return c - '0';
        if ((c >= 'A') && (c <= 'F'))
            return c + 10 - 'A';
        if ((c >= 'a') && (c <= 'a'))
            return c + 10 - 'a';
        return -1;
    }

    char ConfigurationReader::ToHex(char c1, char c2){
        if (ConfigurationReader::nibbleTobyte(c2) >= 0)
            return ConfigurationReader::nibbleTobyte(c1) * 16 + ConfigurationReader::nibbleTobyte(c2);
        return ConfigurationReader::nibbleTobyte(c1);
    }

    std::string ConfigurationReader::HexToString(char* data) {
        std::string result = "";
        for (int i = 0; ConfigurationReader::nibbleTobyte(data[i]) >= 0; i++)
        {
            result += ConfigurationReader::ToHex(data[i], data[i + 1]);
            if (ConfigurationReader::nibbleTobyte(data[i + 1]) >= 0)
                i++;
        }
        return result;
    }

    void ConfigurationReader::SetSize(int lineCount, int value) {
            switch (lineCount) {
            case 0: {
                this->DigitalInSize = value;
                break;
            }
            case 1: {
                this->AnalogInSize = value;
                break;
            }
            case 2: {
                this->OutputSize = value;
                break;
            }
            case 3: {
                this->ActionSize = value;
                break;
            }
            case 4: {
                this->NetConfigSize = value;
                break;
            }
        }
    }

    bool ConfigurationReader::Init() {
        this->GetFileSizes();
        this->PrintSizes();
        return true;
    }

    std::vector<AnalogInConfiguration> ConfigurationReader::DeserializeAnalogConfig() {
        if (this->AnalogInSize > 0) {
            DynamicJsonDocument doc(this->AnalogInSize);
            vector<AnalogInConfiguration> analogChannels;
            File file = SD.open(AnalogFile);
            if (file) {
                DeserializationError error = deserializeJson(doc, file);
                if (error) {
                    std::cout << "DeserializeAnalog Failed" << endl;
                    return analogChannels;
                }else {
                    size_t size = doc.as<JsonArray>().size();
                    for (JsonObject elem : doc.as<JsonArray>()) {
                        AnalogAlert alert1, alert2, alert3;

                        int input = elem[F("Input")]; 
                        ChannelAddress address;
                        address.channel = elem[F("Address")][F("Channel")];
                        address.slot = elem[F("Address")][F("Slot")];

                        int reg = elem[F("Register")];
                        int zeroValue = elem[F("Slope")];
                        int maxValue = elem[F("Offset")];
                        int analogFactor = elem[F("AnalogFactor")];
                        int bypassAlerts = elem[F("BypassAlerts")];
                        int connected = elem[F("Connected")];

                        AnalogInConfiguration config(input,address, reg, connected);

                        JsonObject A1 = elem[F("A1")];
                        alert1.setPoint = A1[F("Setpoint")];
                        alert1.actionId = A1[F("Action")];
                        alert1.bypass = A1[F("Bypass")];
                        alert1.enabled = A1[F("Enabled")];
                        alert1.setPointFactor = 1;

                        JsonObject A2 = elem[F("A2")];
                        alert2.setPoint = A2[F("Setpoint")];
                        alert2.actionId = A2[F("Action")];
                        alert2.bypass = A2[F("Bypass")];
                        alert2.enabled = A2[F("Enabled")];
                        alert2.setPointFactor = 1;

                        JsonObject A3 = elem["A3"];
                        alert3.setPoint = A3[F("Setpoint")];
                        alert3.actionId = A3[F("Action")];
                        alert3.bypass = A3[F("Bypass")];
                        alert3.enabled = A3[F("Enabled")];
                        alert3.setPointFactor = 1;

                        config.alert1 = alert1;
                        config.alert2 = alert2;
                        config.alert3 = alert3;

                        analogChannels.push_back(config);
                    }
                    file.close();
                    return analogChannels;
                }
            }else {
                return analogChannels;
            }
        }
    }

    std::vector<OutputConfiguration> ConfigurationReader::DeserializeOutputConfig() {
        if (this->OutputSize > 0) {
            DynamicJsonDocument doc(this->OutputSize);
            std::vector<OutputConfiguration> outputChannels;
            File file = SD.open(OutputFile);
            if (file) {
                DeserializationError error = deserializeJson(doc, file);
                if (error) {
                    //log error
                    std::cout << "Deserialize Output Failed" << endl;
                    return outputChannels;
                }
                else {
                    size_t size = doc.as<JsonArray>().size();
                    for (JsonObject elem : doc.as<JsonArray>()) {
                        int channel = elem[F("Output")]; 
                        ChannelAddress address;
                        address.channel = elem[F("Addr")][F("Channel")]; 
                        address.slot = elem[F("Addr")][F("Module Slot")]; 
                        int reg = elem[F("Register")]; 
                        int connected = elem[F("Connected")];
                        State startState = (elem[F("Start State")].as<bool>() == true) ? State::High : State::Low;
                        OutputConfiguration config(channel,address,reg,startState,connected);
                        outputChannels.push_back(config);
                    }
                    file.close();
                    return outputChannels;
                }
            }else {
                return outputChannels;
            }
        }
    }

    std::vector<DigitalInConfiguration> ConfigurationReader::DeserializeDigitalConfig() {
        if (this->DigitalInSize > 0) {
            DynamicJsonDocument doc(this->DigitalInSize);
            std::vector<DigitalInConfiguration>  configurations;
            File file = SD.open(DigitalFile);
            if (file) {
                DeserializationError error = deserializeJson(doc, file);
                if (error) {
                    std::cout << "Deserialize Digital Failed" << endl;
                    return configurations;
                }else {
                    size_t size = doc.as<JsonArray>().size();
                    for (JsonObject elem : doc.as<JsonArray>()) {
                        
                        ChannelAddress address;

                        int input = elem[F("Input")];
                        address.channel = elem[F("Address")][F("Channel")];
                        address.slot = elem[F("Address")][F("Slot")];
                        int reg = elem[F("Coil")];
                        int connected = elem[F("Connected")];

                        DigitalInConfiguration config(input,address,reg,connected);

                        DigitalAlert alert;
                        JsonObject Alert = elem[F("Alert")];
                        alert.triggerOn = (Alert[F("TriggerOn")].as<bool>() == true) ? TriggerOn::High : TriggerOn::Low;
                        alert.actionId = Alert[F("Action")];
                        alert.actionType = (ActionType)Alert[F("ActionType")].as<int>();
                        alert.bypass = Alert[F("Bypass")].as<bool>();
                        alert.enabled = Alert[F("Enabled")].as<bool>();
                        config.alert = alert;
                        if (alert.actionId == -1 || ((int)alert.actionType < 1 || (int)alert.actionType>6)) {
                            alert.enabled = false;
                        }
                        configurations.push_back(config);
                    }
                    file.close();
                    return configurations;
                }
            }else {
                return configurations;
            }
        }
    }

    std::vector<ModuleConfiguration> ConfigurationReader::DeserializeModuleConfig() {
        DynamicJsonDocument doc(this->ModuleSize);
        std::vector<ModuleConfiguration> modules;
        File file = SD.open(ModuleFile);
        if (file) {
            DeserializationError error = deserializeJson(doc, file);
            if (error) {
                return modules;
            }
            else {
                int size = doc.as<JsonArray>().size();
                for (JsonObject elem : doc.as<JsonArray>()) {
                    ModuleConfiguration config;
                    config.moduleName = (Module)elem[F("Module")];
                    config.slot = elem[F("Slot")];
                    config.channelCount = elem[F("ChannelCount")];
                    config.channelMapEnd = elem[F("ChannelMap Stop")];
                    config.moduleType = (ModuleType)elem[F("Type")];
                    modules.push_back(config);
                }
                file.close();
                return modules;
                
            }
        }else {
            return modules;
        }
    }

    std::vector<ActionConfiguration> ConfigurationReader::DeserializeActions(){
        DynamicJsonDocument doc(this->ActionSize);
        File file = SD.open(ActionFile);
        std::vector<ActionConfiguration> actions;
        DeserializationError error = deserializeJson(doc, file);
        if (error) {
            std::cout << "Deserialize Action Failed" << std::endl;
            return actions;
        }
        for (JsonObject elem : doc.as<JsonArray>()) {
            ActionConfiguration config;
            config.actionId = elem["ActionId"]; // 0, 1, 2, 3, 4, 5
            config.actionType =(ActionType)elem["ActionType"].as<int>();
            JsonObject O1 = elem[F("O1")];

            config.addr1.channel = O1[F("Address")][F("Channel")];
            config.addr1.slot = O1[F("Address")][F("Slot")];
            config.onLevel1= (O1[F("OnLevel")].as<bool>()==true)? State::High:State::Low;
            config.offLevel1 = (O1[F("OffLevel")].as<bool>() == true) ? State::High : State::Low;

            JsonObject O2 = elem[F("O2")];

            config.addr2.channel = O2[F("Address")][F("Channel")];
            config.addr2.slot = O2[F("Address")][F("Slot")];
            config.onLevel2 = (O2[F("OnLevel")].as<bool>() == true) ? State::High : State::Low;
            config.offLevel2 = (O2[F("OffLevel")].as<bool>() == true) ? State::High : State::Low;

            JsonObject O3 = elem[F("O3")];

            config.addr3.channel = O3[F("Address")][F("Channel")];
            config.addr3.slot = O3[F("Address")][F("Slot")];
            config.onLevel3 = (O3[F("OnLevel")].as<bool>() == true) ? State::High : State::Low;
            config.offLevel3 = (O3[F("OffLevel")].as<bool>() == true) ? State::High : State::Low;

            config.startState= (elem[F("Start State")].as<bool>()==true)? State::High:State::Low;
            config.type = (OutputType)elem[F("OutputType")];
            config.modbusAddress.address = elem[F("ModbusRegister")];
            config.modbusAddress.type = RegisterType::DiscreteInput;
            actions.push_back(config);
        }
        file.close();
        return actions;
    }

    NetConfiguration ConfigurationReader::DeserializeNetConfiguration() {
        NetConfiguration netConfig;
        if (this->NetConfigSize > 0) {
            DynamicJsonDocument doc(this->NetConfigSize);
            File file = SD.open(NetConfigFile);
            if (file) {
                DeserializationError error = deserializeJson(doc, file);
                //if (error) {
                //    std::cout << "Deserialize NetConfig Failed: " << std::endl;
                //    std::cout << error.c_str() << std::endl;
                //    return netConfig;
                //} else {
                    JsonObject root_0 = doc[0];
                    const char* IpAddress = root_0["Ip Address"]; // "172.20.5.56"
                    const char* DNS = root_0["DNS"]; // "172.20.3.5"
                    const char* Mac = root_0["Mac"]; // "6052D0607093"
                    std::cout << "Mac Before: " << Mac << std::endl;

                    netConfig.mac[0]=  ConfigurationReader::ToHex(Mac[0], Mac[1]);
                    netConfig.mac[1] = ConfigurationReader::ToHex(Mac[2], Mac[3]);
                    netConfig.mac[2] = ConfigurationReader::ToHex(Mac[4], Mac[5]);
                    netConfig.mac[3] = ConfigurationReader::ToHex(Mac[6], Mac[7]);
                    netConfig.mac[4] = ConfigurationReader::ToHex(Mac[8], Mac[9]);
                    netConfig.mac[5] = ConfigurationReader::ToHex(Mac[10], Mac[11]);
                    //std::cout << "After: " << ConfigurationReader::HexToString((char*)netConfig.mac) << std::endl;
                    std::cout << "After" << std::endl;
                    for (int i = 0; i < 6; i++) {
                        Serial.print(String(netConfig.mac[i],HEX));
                    }
                    std::cout << std::endl;


                    const char* Gateway = root_0["Gateway"]; 
                    int InputRegsters = root_0["InputRegsters"]; // 121
                    int Coils = root_0["Coils"]; // 1000

                    netConfig.ip.fromString(IpAddress);
                    std::cout << "IP: " << IpAddress << std::endl;
                    netConfig.gateway.fromString(DNS);
                    std::cout << "DNS: " << DNS << std::endl;
                    netConfig.gateway.fromString(Gateway);
                    std::cout << "Gateway: " << Gateway << std::endl;
                    netConfig.coils = Coils;
                    std::cout <<" Coils: " << Coils << std::endl;
                    netConfig.inputRegisters = InputRegsters;
                    std::cout << "Registers: " << netConfig.inputRegisters << std::endl;

                    // netConfig.mac[0] = (byte)String(MacOct1).toInt();
                     //netConfig.mac[1] = (byte)String(MacOct2).toInt();
                     //netConfig.mac[2] = (byte)String(MacOct3).toInt();
                     //netConfig.mac[3] = (byte)String(MacOct4).toInt();
                     //netConfig.mac[4] = (byte)String(MacOct5).toInt();
                     //netConfig.mac[5] = (byte)String(MacOct6).toInt();

                    //JsonObject root_0 = doc[0];
                    //const char* root_0_Ip_Address = root_0["Ip Address"]; // "172.20.5.70"
                    //const char* root_0_DNS = root_0["DNS"]; // "172.20.3.5"
                    //const char* root_0_Mac = root_0["Mac"]; // "6052D0607093"
                    //const char* root_0_Gateway = root_0["Gateway"]; // "172.20.5.1"
                    //int root_0_InputRegsters = root_0["InputRegsters"]; // 121
                    //int root_0_Coils = root_0["Coils"]; // 1000
                    file.close();
                    return netConfig;
                //}
            } else {
                std::cout << "Error opening netconfig" << std::endl;
                return netConfig;
            }

        } else {
            return netConfig;
        }
    }

    ConfigurationReader::~ConfigurationReader() {   }
};