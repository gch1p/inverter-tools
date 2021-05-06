// SPDX-License-Identifier: BSD-3-Clause

#ifndef INVERTER_TOOLS_P18_CLIENT_H_
#define INVERTER_TOOLS_P18_CLIENT_H_

#include "../voltronic/device.h"
#include "types.h"
#include "response.h"

#include <memory>
#include <vector>
#include <string>


namespace p18 {

class Client {
private:
    std::shared_ptr<voltronic::Device> device_;
    static std::string packArguments(p18::CommandType commandType, std::vector<std::string>& arguments);

public:
    void setDevice(std::shared_ptr<voltronic::Device> device);
    std::shared_ptr<response_type::BaseResponse> execute(p18::CommandType commandType, std::vector<std::string>& arguments);
    std::pair<std::shared_ptr<char>, size_t> runOnDevice(std::string& raw);
};

}


#endif //INVERTER_TOOLS_P18_CLIENT_H_
