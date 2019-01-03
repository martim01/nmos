#include "connection.h"
#include "log.h"

const std::string connection::STR_ACTIVATE[4] = {"", "activate_immediate", "activate_scheduled_absolute", "activate_scheduled_relative"};

connection::connection() :
    bMasterEnable(true),
    eActivate(connection::ACT_NULL)
{

}

bool connection::Patch(const Json::Value& jsData)
{
    Log::Get(Log::DEBUG) << "Patch: connection" << std::endl;

    bool bIsOk(true);

    if(jsData.isObject())
    {
        if(jsData["master_enable"].isBool())
        {
            bMasterEnable = jsData["master_enable"].asBool();
        }
        else if(jsData["master_enable"].empty() == false)
        {
            Log::Get(Log::DEBUG) << "Patch: master_enable incorrect type." << std::endl;
            bIsOk = false;
        }

        if(jsData["activation"].isObject())
        {
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
                    Log::Get(Log::DEBUG) << "Patch: activation mode not found." << std::endl;
                }
                bIsOk &= bFound;
            }
            else
            {
                if(jsData["activation"]["mode"].isNull() == false)
                {
                    Log::Get(Log::DEBUG) << "Patch: activation mode incorrect type" << std::endl;
                    bIsOk = false;
                }
            }
            if(jsData["activation"]["requested_time"].isString())
            {
                sRequestedTime = jsData["activation"]["requested_time"].asString();
                // @todo check time is correct
            }
            else if(jsData["activation"]["requested_time"].isNull() == false && jsData["activation"]["requested_time"].empty() == false)
            {
                bIsOk = false;
                Log::Get(Log::DEBUG) << "Patch: activation requested_time incorrect type" << std::endl;
            }
        }
        else
        {
            bIsOk = false;
        }
    }
    else
    {
        Log::Get(Log::DEBUG) << "Patch: json not an object" << std::endl;
        bIsOk = false;
    }

    Log::Get(Log::DEBUG) << "Patch: connection result " << bIsOk << std::endl;
    return bIsOk;
}


Json::Value connection::GetJson() const
{
    Json::Value jsConnect;

    jsConnect["activation"] = Json::objectValue;
    if(eActivate == ACT_NULL)
    {
        jsConnect["activation"]["mode"] = Json::nullValue;
        jsConnect["activation"]["requested_time"] = Json::nullValue;

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
            jsConnect["activation"]["requested_time"] = sRequestedTime;

            if(sActivationTime.empty() == false)
            {
                jsConnect["activation"]["activation_time"] = sActivationTime;
            }
        }
    }

    jsConnect["master_enable"] = bMasterEnable;

    return jsConnect;
}



connectionSender::connectionSender() : connection()
{

}

bool connectionSender::Patch(const Json::Value& jsData)
{
    Log::Get(Log::DEBUG) << "Patch: connectionSender" << std::endl;
    bool bIsOk = connection::Patch(jsData);
    if(bIsOk)
    {
        bIsOk &= tpSender.Patch(jsData["transport_params"]);

        if(jsData["receiver_id"].isString())
        {
            sReceiverId = jsData["receiver_id"].asString();
        }
        else if((jsData["receiver_id"].isNull()  == false && jsData["receiver_id"].empty() == false))
        {
            bIsOk = false;
            Log::Get(Log::DEBUG) << "Patch: receiver_id incorrect type" << std::endl;
        }
    }
    return bIsOk;
}

Json::Value connectionSender::GetJson() const
{
    Json::Value jsConnect(connection::GetJson());

    if(sReceiverId.empty())
    {
        jsConnect["receiver_id"] = Json::nullValue;
    }
    else
    {
        jsConnect["receiver_id"] = sReceiverId;
    }
    jsConnect["transport_params"] = Json::arrayValue;
    jsConnect["transport_params"].append(tpSender.GetJson());
    return jsConnect;
}


connectionReceiver::connectionReceiver() : connection()
{

}

bool connectionReceiver::Patch(const Json::Value& jsData)
{
    bool bIsOk = connection::Patch(jsData);
    if(bIsOk)
    {
        bIsOk &= tpReceiver.Patch(jsData["transport_params"]);

        if(jsData["sender_id"].isString())
        {
            sSenderId = jsData["sender_id"].asString();
        }
        else
        {
            if(jsData["sender_id"].isNull() == false && jsData["sender_id"].empty() == false)
            {
                bIsOk = false;
                Log::Get(Log::DEBUG) << "Patch: sender_id incorrect type" << std::endl;
            }
        }

        if(jsData["transport_file"].isObject())
        {
            if(jsData["transport_file"]["type"].isString())
            {
                sTransportFileType = jsData["transport_file"]["type"].asString();
            }
            else if(jsData["transport_file"]["type"].isNull() == false)
            {
                bIsOk = false;
                Log::Get(Log::DEBUG) << "Patch: transport_file type incorrect type" << std::endl;
            }
            if(jsData["transport_file"]["data"].isString())
            {
                sTransportFileData = jsData["transport_file"]["data"].asString();
            }
            else if(jsData["transport_file"]["data"].isNull() == false)
            {
                bIsOk = false;
                Log::Get(Log::DEBUG) << "Patch: transport_file data incorrect type" << std::endl;
            }

            // @todo decode transport file and overwrite staged parameters with the relevant settings
        }
    }
    return bIsOk;
}



Json::Value connectionReceiver::GetJson()  const
{
    Json::Value jsConnect(connection::GetJson());

    if(sSenderId.empty())
    {
        jsConnect["sender_id"] = Json::nullValue;
    }
    else
    {
        jsConnect["sender_id"] = sSenderId;
    }

    jsConnect["transport_file"] = Json::objectValue;
    if(sTransportFileType.empty() || sTransportFileData.empty())
    {
        jsConnect["transport_file"]["type"] = Json::nullValue;
        jsConnect["transport_file"]["data"] = Json::nullValue;
    }
    else
    {
        jsConnect["transport_file"]["type"] = sTransportFileType;
        jsConnect["transport_file"]["data"] = sTransportFileData;
    }

    jsConnect["transport_params"] = Json::arrayValue;
    jsConnect["transport_params"].append(tpReceiver.GetJson());
    return jsConnect;
}









connection& connection::operator=(const connection& other)
{
    bMasterEnable = other.bMasterEnable;
    eActivate = other.eActivate;
    sRequestedTime = other.sRequestedTime;
    sActivationTime = other.sActivationTime;
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
