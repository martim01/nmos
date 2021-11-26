#include "activation.h"
#include <array>
#include "utils.h"


const std::string MODE = "mode";
const std::string REQUESTED_TIME = "requested_time";
const std::string ACTIVATED_TIME = "activation_time";

using namespace pml::nmos;

const std::array<std::string,4> activation::STR_ACTIVATE = {"", "activate_immediate", "activate_scheduled_absolute", "activate_scheduled_relative"};



activation::activation()
{
    m_json[MODE] = Json::Value::null;
    m_json[REQUESTED_TIME] = Json::Value::null;
}

activation::activation(const activation& other) : m_json(other.GetJson())
{
}

activation& activation::operator=(const activation& other)
{
    if(&other != this)
    {
        m_json = other.GetJson();
    }
    return *this;
}

void activation::SetMode(enumActivate eMode)
{
    if(eMode == ACT_NULL || eMode >= STR_ACTIVATE.size())
    {
        m_json[MODE] = Json::Value::null;
    }
    else
    {
        m_json[MODE] = STR_ACTIVATE[eMode];
    }

}

void activation::SetRequestedTime(std::experimental::optional<std::chrono::time_point<std::chrono::high_resolution_clock>> when)
{
    if(when)
    {
        m_json[REQUESTED_TIME] = ConvertTimeToString(*when);
    }
    else
    {
        m_json[REQUESTED_TIME] = Json::Value::null;
    }
}

void activation::Clear()
{
    SetMode(ACT_NULL);
    SetRequestedTime({});
}

void activationResponse::Clear()
{
    SetMode(ACT_NULL);
    SetRequestedTime({});
    SetActivationTime({});
}

void activationResponse::SetActivationTime(std::experimental::optional<std::chrono::time_point<std::chrono::high_resolution_clock>> when)
{
    if(when)
    {
        m_json[ACTIVATED_TIME] = ConvertTimeToString(*when);
    }
    else
    {
        m_json[ACTIVATED_TIME] = Json::Value::null;
    }
}

const Json::Value activation::GetJson() const
{
    return m_json;
}

activation::enumActivate activation::GetMode() const
{
    auto mode = GetString(m_json, MODE);
    if(mode)
    {
        for(size_t i = 0; i < STR_ACTIVATE.size(); i++)
        {
            if(*mode == STR_ACTIVATE[i])
            {
                return (enumActivate)i;
            }
        }
    }
    return ACT_NULL;
}

std::experimental::optional<std::string> activation::GetRequestedTime() const
{
    return GetString(m_json, REQUESTED_TIME);
}

std::experimental::optional<std::chrono::time_point<std::chrono::high_resolution_clock>> activation::GetRequestedTimePoint() const
{
    auto sTime = GetString(m_json, REQUESTED_TIME);
    if(sTime)
    {
        return ConvertTaiStringToTimePoint(*sTime);
    }
    return {};
}

activationRequest::activationRequest() : activation()
{

}

void activationRequest::Setup(activation::enumActivate eMode, std::experimental::optional<std::chrono::time_point<std::chrono::high_resolution_clock>> when)
{
    SetMode(eMode);
    SetRequestedTime(when);
}

activationRequest::activationRequest(const activationRequest& other) : activation(other)
{
}

bool activationRequest::Patch(const Json::Value& jsData)
{
    if(CheckJsonRequired(jsData, {{MODE, {jsondatatype::_STRING, jsondatatype::_NULL}}}) == false ||
       CheckJsonOptional(jsData, {{REQUESTED_TIME, {jsondatatype::_STRING, jsondatatype::_NULL}}}) == false)
    {
        return false;
    }

    m_json[MODE] = jsData[MODE];
    m_json[REQUESTED_TIME] = jsData[REQUESTED_TIME];
    return true;
}

activationResponse::activationResponse() : activation()
{
    m_json[ACTIVATED_TIME] = Json::Value::null;
}

activationResponse::activationResponse(const activationResponse& other) : activation(other)
{
}


bool activationResponse::Patch(const Json::Value& jsData)
{
    if(CheckJsonRequired(jsData, {{MODE, {jsondatatype::_STRING, jsondatatype::_NULL}}}) == false ||
       CheckJsonOptional(jsData, {{REQUESTED_TIME, {jsondatatype::_STRING, jsondatatype::_NULL}}}) == false)
    {
        return false;
    }

    m_json[MODE] = jsData[MODE];
    m_json[REQUESTED_TIME] = jsData[REQUESTED_TIME];
    return true;
}

std::experimental::optional<std::string> activationResponse::GetActivationTime() const
{
    return GetString(m_json, ACTIVATED_TIME);
}

std::experimental::optional<std::chrono::time_point<std::chrono::high_resolution_clock>> activationResponse::GetActivationTimePoint() const
{
    auto Time = GetString(m_json, ACTIVATED_TIME);
    if(Time)
    {
        return ConvertTaiStringToTimePoint(*Time);
    }
    return {};
}
