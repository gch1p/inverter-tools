// SPDX-License-Identifier: BSD-3-Clause

#include <memory>
#include <utility>
#include <sstream>
#include <iomanip>
#include <cmath>
#include <stdexcept>

#include "client.h"
#include "types.h"
#include "defines.h"
#include "exceptions.h"
#include "response.h"
#include "../voltronic/crc.h"

#define MKRESPONSE(type) std::shared_ptr<response_type::BaseResponse>(new response_type::type(raw, rawSize))

#define RESPONSE_CASE(type) \
    case CommandType::Get ## type: \
        response = MKRESPONSE(type); \
        break; \


namespace p18 {

void Client::setDevice(std::shared_ptr<voltronic::Device> device) {
    device_ = std::move(device);
}

std::shared_ptr<response_type::BaseResponse> Client::execute(p18::CommandType commandType, std::vector<std::string>& arguments) {
    std::ostringstream buf;
    buf << std::setfill('0');

    int iCommandType = static_cast<int>(commandType);
    bool isSetCommand = iCommandType >= 100;

    auto pos = raw_commands.find(commandType);
    if (pos == raw_commands.end())
        throw std::runtime_error("packedCommand " + std::to_string(iCommandType) + " not found");

    std::string packedCommand = pos->second;
    std::string packedArguments = packArguments(commandType, arguments);

    size_t len = sizeof(voltronic::CRC) + 1 + packedCommand.size() + packedArguments.size();

    buf << "^";
    buf << (isSetCommand ? "S" : "P");
    buf << std::setw(3) << len;
    buf << packedCommand;
    buf << packedArguments;

    std::string packed = buf.str();

    auto result = runOnDevice(packed);
    std::shared_ptr<response_type::BaseResponse> response;

    const auto raw = result.first;
    const auto rawSize = result.second;

    switch (commandType) {
        RESPONSE_CASE(ProtocolID)
        RESPONSE_CASE(CurrentTime)
        RESPONSE_CASE(TotalGenerated)
        RESPONSE_CASE(YearGenerated)
        RESPONSE_CASE(MonthGenerated)
        RESPONSE_CASE(DayGenerated)
        RESPONSE_CASE(SerialNumber)
        RESPONSE_CASE(CPUVersion)
        RESPONSE_CASE(RatedInformation)
        RESPONSE_CASE(GeneralStatus)
        RESPONSE_CASE(WorkingMode)
        RESPONSE_CASE(FaultsAndWarnings)
        RESPONSE_CASE(FlagsAndStatuses)
        RESPONSE_CASE(RatedDefaults)
        RESPONSE_CASE(AllowedChargeCurrents)
        RESPONSE_CASE(AllowedACChargeCurrents)
        RESPONSE_CASE(ParallelRatedInformation)
        RESPONSE_CASE(ParallelGeneralStatus)
        RESPONSE_CASE(ACChargeTimeBucket)
        RESPONSE_CASE(ACSupplyTimeBucket)

        case CommandType::SetACSupply:
        case CommandType::SetFlag:
        case CommandType::SetDefaults:
        case CommandType::SetBatteryMaxChargeCurrent:
        case CommandType::SetBatteryMaxACChargeCurrent:
        case CommandType::SetACOutputFreq:
        case CommandType::SetBatteryMaxChargeVoltage:
        case CommandType::SetACOutputVoltage:
        case CommandType::SetOutputSourcePriority:
        case CommandType::SetBatteryChargeThresholds:
        case CommandType::SetChargeSourcePriority:
        case CommandType::SetSolarPowerPriority:
        case CommandType::SetACInputVoltageRange:
        case CommandType::SetBatteryType:
        case CommandType::SetOutputMode:
        case CommandType::SetBatteryCutOffVoltage:
        case CommandType::SetSolarConfig:
        case CommandType::ClearGenerated:
        case CommandType::SetDateTime:
        case CommandType::SetACChargeTimeBucket:
        case CommandType::SetACSupplyTimeBucket:
            response = MKRESPONSE(SetResponse);
            break;
    }

    if (!response->validate())
        throw InvalidResponseError("validate() failed");

    response->unpack();
    return std::move(response);
}

std::pair<std::shared_ptr<char>, size_t> Client::runOnDevice(std::string& raw) {
    size_t bufSize = 256;
    std::shared_ptr<char> buf(new char[bufSize]);
    size_t responseSize = device_->run(
            (const u8*)raw.c_str(), raw.size(),
            (u8*)buf.get(), bufSize);

    return std::pair<std::shared_ptr<char>, size_t>(buf, responseSize);
}

std::string Client::packArguments(p18::CommandType commandType, std::vector<std::string>& arguments) {
    std::ostringstream buf;
    buf << std::setfill('0');

    switch (commandType) {
        case CommandType::GetYearGenerated:
        case CommandType::SetOutputSourcePriority:
        case CommandType::SetSolarPowerPriority:
        case CommandType::SetACInputVoltageRange:
        case CommandType::SetBatteryType:
        case CommandType::SetACSupply:
            buf << arguments[0];
            break;

        case CommandType::GetMonthGenerated:
        case CommandType::GetDayGenerated:
            buf << arguments[0];
            for (int i = 1; i <= (commandType == CommandType::GetMonthGenerated ? 1 : 2); i++)
                buf << std::setw(2) << std::stoi(arguments[i]);
            break;

        case CommandType::GetParallelGeneralStatus:
        case CommandType::GetParallelRatedInformation:
            buf << std::stoi(arguments[0]);
            break;

        case CommandType::SetFlag:
            buf << (arguments[1] == "1" ? "E" : "D");
            buf << arguments[0];
            break;

        case CommandType::SetBatteryMaxChargeCurrent:
        case CommandType::SetBatteryMaxACChargeCurrent:
            buf << arguments[0] << ",";
            buf << std::setw(3) << std::stoi(arguments[1]);
            break;

        case CommandType::SetACOutputFreq:
            buf << std::setw(2) << std::stoi(arguments[0]);
            break;

        case CommandType::SetBatteryMaxChargeVoltage:
        case CommandType::SetBatteryChargeThresholds: {
            for (int i = 0; i < 2; i++) {
                double val = std::stod(arguments[i]);
                buf << std::setw(3) << (int)round(val*10);
                if (i == 0)
                    buf << ",";
            }
            break;
        }

        case CommandType::SetACOutputVoltage: {
            buf << std::setw(4) << (std::stoi(arguments[0])*10);
            break;
        }

        case CommandType::SetChargeSourcePriority:
        case CommandType::SetOutputMode:
            buf << arguments[0] << "," << arguments[1];
            break;

        case CommandType::SetBatteryCutOffVoltage: {
            double v = std::stod(arguments[0]);
            buf << std::setw(3) << ((int)round(v*10));
            break;
        }

        case CommandType::SetSolarConfig: {
            size_t len = arguments[0].size();
            buf << std::setw(2) << len << arguments[0];
            if (len < 20) {
                for (int i = 0; i < 20-len; i++)
                    buf << "0";
            }
            break;
        }

        case CommandType::SetDateTime: {
            for (int i = 0; i < 6; i++) {
                int val = std::stoi(arguments[i]);
                if (i == 0)
                    val -= 2000;
                buf << std::setw(2) << val;
            }
            break;
        }

        case CommandType::SetACChargeTimeBucket:
        case CommandType::SetACSupplyTimeBucket:
            for (int i = 0; i < 4; i++) {
                buf << std::setw(2) << std::stoi(arguments[i]);
                if (i == 1)
                    buf << ",";
            }
            break;

        default:
            break;
    }

    return buf.str();
}

}