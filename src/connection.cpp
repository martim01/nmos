#include "connection.h"
#include "log.h"
#include "sdp.h"
#include "utils.h"

const std::string connection::STR_ACTIVATE[4] = {"", "activate_immediate", "activate_scheduled_absolute", "activate_scheduled_relative"};

connection::connection() :
    bMasterEnable(false),
    eActivate(connection::ACT_NULL),
    nProperties(connection::FP_ALL)
{

}

connection::connection(int flagProperties) :
    bMasterEnable(false),
    eActivate(connection::ACT_NULL),
    nProperties(flagProperties)
{

}

connection::connection(const connection& conReq) :
    bMasterEnable(conReq.bMasterEnable),
    eActivate(conReq.eActivate),
    sRequestedTime(conReq.sRequestedTime),
    sActivationTime(conReq.sActivationTime),
    tpActivation(conReq.tpActivation),
    nProperties(conReq.nProperties)
{

}


bool connection::Patch(const Json::Value& jsData)
{
    Log::Get(Log::LOG_DEBUG) << "Patch: connection" << std::endl;

    bool bIsOk(true);

    if(jsData.isObject())
    {
        if(jsData["master_enable"].isBool())
        {
            nProperties |= FP_ENABLE;
            bMasterEnable = jsData["master_enable"].asBool();
        }
        else if(jsData["master_enable"].empty() == false)
        {
            Log::Get(Log::LOG_DEBUG) << "Patch: master_enable incorrect type." << std::endl;
            bIsOk = false;
        }

        if(jsData["activation"].isObject())
        {
            nProperties |= FP_ACTIVATION;
            if(jsData["activation"]["mode"].isString())
            {
                bool bFound(false);
                for(int i = 0; i < 4; i++)
                {
                    if(jsData["activation"]["mode"].asString() == STR_ACTIVATE[i])
                    {
                        bFound = true;
                        eActivate = (enumActivate)i;
                        break;
                    }
                }
                if(!bFound)
                {
                    Log::Get(Log::LOG_DEBUG) << "Patch: activation mode not found." << std::endl;
                }
                bIsOk &= bFound;
            }
            else
            {
                if(jsData["activation"]["mode"].isNull() == false)
                {
                    Log::Get(Log::LOG_DEBUG) << "Patch: activation mode incorrect type" << std::endl;
                    bIsOk = false;
                }
            }
            if(jsData["activation"]["requested_time"].isString())
            {
                sRequestedTime = jsData["activation"]["requested_time"].asString();
                // @todo check time is correct
            }
            else if(jsData["activation"].isMember("requested_time") && jsData["activation"]["requested_time"].isNull() == false)
            {
                bIsOk = false;
                Log::Get(Log::LOG_DEBUG) << "Patch: activation requested_time incorrect type" << std::endl;
            }
        }
        else if(jsData["activation"].empty() == false)
        {
            Log::Get(Log::LOG_DEBUG) << "Patch: activation must be an object if it exists" << std::endl;
            bIsOk = false;
        }
    }
    else
    {
        Log::Get(Log::LOG_DEBUG) << "Patch: json not an object" << std::endl;
        bIsOk = false;
    }

    Log::Get(Log::LOG_DEBUG) << "Patch: connection result " << bIsOk << std::endl;
    return bIsOk;
}


Json::Value connection::GetJson(const ApiVersion& version) const
{
    Json::Value jsConnect;

    if(FP_ACTIVATION & nProperties)
    {
        jsConnect["activation"] = Json::objectValue;
        if(eActivate == ACT_NULL)
        {
            jsConnect["activation"]["mode"] = Json::nullValue;
            if(sActivationTime.empty())
            {
                jsConnect["activation"]["activation_time"] = Json::nullValue;
            }
            else
            {
                jsConnect["activation"]["activation_time"] = sActivationTime;
            }
            if(sRequestedTime.empty() == false)
            {
                jsConnect["activation"]["requested_time"] = sRequestedTime;
            }
            else
            {
                jsConnect["activation"]["requested_time"] = Json::nullValue;
            }

        }
        else
        {
            jsConnect["activation"]["mode"] = STR_ACTIVATE[eActivate];
            if(eActivate == ACT_NULL)
            {
                jsConnect["activation"]["requested_time"] = Json::nullValue;

                if(sActivationTime.empty() == false)
                {
                    jsConnect["activation"]["activation_time"] = Json::nullValue;
                }
            }
            else
            {
                if(sRequestedTime.empty() == false)
                {
                    jsConnect["activation"]["requested_time"] = sRequestedTime;
                }
                else
                {
                    jsConnect["activation"]["requested_time"] = Json::nullValue;
                }

                if(sActivationTime.empty() == false)
                {
                    jsConnect["activation"]["activation_time"] = sActivationTime;
                }
                else
                {
                    jsConnect["activation"]["activation_time"] = Json::nullValue;
                }
            }
        }
    }
    if(FP_ENABLE & nProperties)
    {
        jsConnect["master_enable"] = bMasterEnable;
    }

    return jsConnect;
}



connectionSender::connectionSender() : connection()
{

}

connectionSender::connectionSender(int flagProperties) : connection(flagProperties)
{

}

connectionSender::connectionSender(const connectionSender& conReq) : connection(conReq),
    tpSender(conReq.tpSender),
    sReceiverId(conReq.sReceiverId)
{

}


bool connectionSender::Patch(const Json::Value& jsData)
{
    Log::Get(Log::LOG_DEBUG) << "Patch: connectionSender: NOW" << std::endl;
    bool bIsOk = (connection::Patch(jsData) && CheckJson(jsData, {"master_enable", "activation", "transport_params", "receiver_id"}));
    if(bIsOk)
    {

        if(jsData["transport_params"].isArray())
        {
            nProperties |= FP_TRANSPORT_PARAMS;
            bIsOk &= tpSender.Patch(jsData["transport_params"][0]);

            // @todo second tp for SMPTE2022
            tpSender.bRtpEnabled = bMasterEnable;   //this should be settable if 2 or more transportparams
        }
        else if(jsData["transport_params"].empty() == false)
        {
            Log::Get(Log::LOG_DEBUG) << "Patch: transport_params not an array." << std::endl;
            bIsOk = false;
        }


        if(jsData["receiver_id"].isString())
        {
            Log::Get(Log::LOG_DEBUG) << "Patch: receiver_id string: " << jsData["receiver_id"].asString() << std::endl;
            nProperties |= FP_ID;
            sReceiverId = jsData["receiver_id"].asString();
        }
        else if(JsonMemberExistsAndIsNull(jsData, "receiver_id"))
        {
            Log::Get(Log::LOG_DEBUG) << "Patch: receiver_id null: " << std::endl;
            nProperties |= FP_ID;
            sReceiverId.clear();
        }
        else if(jsData.isMember("receiver_id"))
        {
            bIsOk = false;
            Log::Get(Log::LOG_DEBUG) << "Patch: receiver_id incorrect type" << std::endl;
        }

    }
    else
    {
    Log::Get(Log::LOG_DEBUG) << "Patch: error" << std::endl;
    }
    return bIsOk;
}

Json::Value connectionSender::GetJson(const ApiVersion& version) const
{
    Json::Value jsConnect(connection::GetJson(version));

    if(FP_ID & nProperties)
    {
        if(sReceiverId.empty())
        {
            jsConnect["receiver_id"] = Json::nullValue;
        }
        else
        {
            jsConnect["receiver_id"] = sReceiverId;
        }
    }
    if(FP_TRANSPORT_PARAMS & nProperties)
    {
        jsConnect["transport_params"] = Json::arrayValue;
        jsConnect["transport_params"].append(tpSender.GetJson(version));
    }
    return jsConnect;
}


connectionReceiver::connectionReceiver() : connection(), sTransportFileType("application/sdp")
{

}

connectionReceiver::connectionReceiver(int flagProperties) : connection(flagProperties)
{

}

connectionReceiver::connectionReceiver(const connectionReceiver& conReq) : connection(conReq),
    tpReceiver(conReq.tpReceiver),
    sSenderId(conReq.sSenderId),
    sTransportFileType(conReq.sTransportFileType),
    sTransportFileData(conReq.sTransportFileData)
{

}

bool connectionReceiver::Patch(const Json::Value& jsData)
{
    bool bIsOk(CheckJson(jsData, {"master_enable", "activation", "transport_params", "sender_id", "transport_file"}));
    //need to decode the SDP and make the correct transport param changes here so that any connection json will overwrite them
    if(bIsOk && jsData["transport_file"].isObject())
    {
        nProperties |= FP_TRANSPORT_FILE;
        if(jsData["transport_file"]["type"].isString())
        {
            sTransportFileType = jsData["transport_file"]["type"].asString();
        }
        else if(jsData["transport_file"].isMember("type") && jsData["transport_file"]["type"].isNull() == false)
        {
            bIsOk = false;
            Log::Get(Log::LOG_DEBUG) << "Patch: transport_file type incorrect type" << std::endl;
        }
        if(jsData["transport_file"]["data"].isString())
        {
            sTransportFileData = jsData["transport_file"]["data"].asString();
        }
        else if(jsData["transport_file"].isMember("data") && jsData["transport_file"]["data"].isNull() == false)
        {
            bIsOk = false;
            Log::Get(Log::LOG_DEBUG) << "Patch: transport_file data incorrect type" << std::endl;
        }

        if(sTransportFileType == "application/sdp" && sTransportFileData.empty() == false)
        {
            Log::Get(Log::LOG_DEBUG) << "Patch: transport_file data correct" << std::endl;
            SdpManager::SdpToTransportParams(sTransportFileData, tpReceiver);
        }
    }

    bIsOk &= connection::Patch(jsData);
    if(bIsOk)
    {

        if(jsData["transport_params"].isArray())
        {
            nProperties |= FP_TRANSPORT_PARAMS;
            bIsOk &= tpReceiver.Patch(jsData["transport_params"][0]);
            // @todo second tp for SMPTE2022
            tpReceiver.bRtpEnabled = bMasterEnable;   //this should be settable if 2 or more transportparams
        }
        else if(jsData["transport_params"].empty() == false)
        {
            Log::Get(Log::LOG_DEBUG) << "Patch: transport_params not an array." << std::endl;
            bIsOk = false;
        }

        if(jsData["sender_id"].isString())
        {
            nProperties |= FP_ID;
            sSenderId = jsData["sender_id"].asString();
            Log::Get(Log::LOG_DEBUG) << "PatchReceiver Connection: sender_id is " << sSenderId << std::endl;
        }
        else if(JsonMemberExistsAndIsNull(jsData, "sender_id"))
        {
            Log::Get(Log::LOG_DEBUG) << "PatchReceiver Connection: sender_id is NULL" << std::endl;
            sSenderId.clear();
        }
        else if(jsData.isMember("sender_id") != false)
        {
            bIsOk = false;
            Log::Get(Log::LOG_DEBUG) << "Patch: sender_id incorrect type" << std::endl;
        }
    }


    return bIsOk;
}



Json::Value connectionReceiver::GetJson(const ApiVersion& version)  const
{
    Json::Value jsConnect(connection::GetJson(version));
    if(FP_TRANSPORT_FILE & nProperties)
    {
        jsConnect["transport_file"] = Json::objectValue;
        if(sTransportFileType.empty())
        {
            jsConnect["transport_file"]["type"] = Json::nullValue;

        }
        else
        {
            jsConnect["transport_file"]["type"] = sTransportFileType;
        }
        if(sTransportFileData.empty() == false)
        {
            jsConnect["transport_file"]["data"] = sTransportFileData;
        }
        else
        {
                jsConnect["transport_file"]["data"] = Json::nullValue;
        }
    }

    if(FP_ID & nProperties)
    {
        if(sSenderId.empty())
        {
            jsConnect["sender_id"] = Json::nullValue;
        }
        else
        {
            jsConnect["sender_id"] = sSenderId;
        }
    }

    if(FP_TRANSPORT_PARAMS & nProperties)
    {
        jsConnect["transport_params"] = Json::arrayValue;
        jsConnect["transport_params"].append(tpReceiver.GetJson(version));
    }
    return jsConnect;
}









connection& connection::operator=(const connection& other)
{
    bMasterEnable = other.bMasterEnable;
    eActivate = other.eActivate;
    sRequestedTime = other.sRequestedTime;
    sActivationTime = other.sActivationTime;
    tpActivation = other.tpActivation;
    return *this;
}

connectionSender& connectionSender::operator=(const connectionSender& other)
{
    connection::operator=(other);
    tpSender = other.tpSender;
    sReceiverId = other.sReceiverId;

    return *this;
}

connectionReceiver& connectionReceiver::operator=(const connectionReceiver& other)
{
    connection::operator=(other);

    tpReceiver = other.tpReceiver;
    sSenderId = other.sSenderId;
    sTransportFileType = other.sTransportFileType;
    sTransportFileData = other.sTransportFileData;
    return *this;
}
