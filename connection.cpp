#include "connection.h"

const std::string connection::STR_ACTIVATE[4] = {"", "activate_immediate", "activate_scheduled_absolute", "activate_scheduled_relative"};

connection::connection() :
    bMasterEnable(true),
    eActivate(connection::ACT_NULL)
{

}

bool connection::Patch(const Json::Value& jsData)
{
    bool bIsOk(true);

    if(jsData.isObject())
    {
        if(jsData["master_enable"].isBool())
        {
            bMasterEnable = jsData["master_enable"].asBool();
        }
        else
        {
            bIsOk = (jsData["master_enable"].empty());
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
                bIsOk &= bFound;
            }
            else
            {
                bIsOk &= (jsData["activation"]["mode"].isNull());
            }
            if(jsData["activation"]["requested_time"].isString())
            {
                sRequestedTime = jsData["activation"]["requested_time"].asString();
                // @todo check time is correct
            }
            else
            {
                bIsOk &= (jsData["activation"]["mode"].isNull() || jsData["activation"]["mode"].empty());
            }
        }
        else
        {
            bIsOk = false;
        }
    }
    else
    {
        bIsOk = false;
    }
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
    bool bIsOk = connection::Patch(jsData);
    if(bIsOk)
    {
        bIsOk &= tpSender.Patch(jsData["transport_params"]);

        if(jsData["receiver_id"].isString())
        {
            sReceiverId = jsData["receiver_id"].asString();
        }
        else
        {
            bIsOk &= (jsData["receiver_id"].isNull() || jsData["receiver_id"].empty());
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
            bIsOk &= (jsData["sender_id"].isNull() || jsData["sender_id"].empty());
        }

        if(jsData["transport_file"].isObject())
        {
            if(jsData["transport_file"]["type"].isString())
            {
                sTransportFileType = jsData["transport_file"]["type"].asString();
            }
            else
            {
                bIsOk &= (jsData["transport_file"]["type"].isNull());
            }
            if(jsData["transport_file"]["data"].isString())
            {
                sTransportFileData = jsData["transport_file"]["data"].asString();
            }
            else
            {
                bIsOk &= (jsData["transport_file"]["data"].isNull());
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









