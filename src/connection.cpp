#include "connection.h"
#include "log.h"
#include "sdp.h"
#include "utils.h"

using namespace pml::nmos;
const std::string connection::STR_ACTIVATE[4] = {"", "activate_immediate", "activate_scheduled_absolute", "activate_scheduled_relative"};

connection::connection() :
    bMasterEnable(false),
    eActivate(connection::ACT_NULL),
    nProperties(connection::FP_ALL),
    bClient(false)
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
    pmlLog(pml::LOG_DEBUG) << "NMOS: " << "Patch: connection" ;

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
            pmlLog(pml::LOG_DEBUG) << "NMOS: " << "Patch: master_enable incorrect type." ;
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
                    pmlLog(pml::LOG_DEBUG) << "NMOS: " << "Patch: activation mode not found." ;
                }
                bIsOk &= bFound;
            }
            else
            {
                if(jsData["activation"]["mode"].isNull() == false)
                {
                    pmlLog(pml::LOG_DEBUG) << "NMOS: " << "Patch: activation mode incorrect type" ;
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
                pmlLog(pml::LOG_DEBUG) << "NMOS: " << "Patch: activation requested_time incorrect type" ;
            }
        }
        else if(jsData["activation"].empty() == false)
        {
            pmlLog(pml::LOG_DEBUG) << "NMOS: " << "Patch: activation must be an object if it exists" ;
            bIsOk = false;
        }
    }
    else
    {
        pmlLog(pml::LOG_DEBUG) << "NMOS: " << "Patch: json not an object" ;
        bIsOk = false;
    }

    pmlLog(pml::LOG_DEBUG) << "NMOS: " << "Patch: connection result " << bIsOk ;
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
            if(!bClient)
            {
                if(sActivationTime.empty())
                {
                    jsConnect["activation"]["activation_time"] = Json::nullValue;
                }
                else
                {
                    jsConnect["activation"]["activation_time"] = sActivationTime;
                }
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

                if(sActivationTime.empty() == false && !bClient)
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

                if(!bClient)
                {
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
    pmlLog(pml::LOG_DEBUG) << "NMOS: " << "Patch: connectionSender: NOW" ;
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
            pmlLog(pml::LOG_DEBUG) << "NMOS: " << "Patch: transport_params not an array." ;
            bIsOk = false;
        }


        if(jsData["receiver_id"].isString())
        {
            pmlLog(pml::LOG_DEBUG) << "NMOS: " << "Patch: receiver_id string: " << jsData["receiver_id"].asString() ;
            nProperties |= FP_ID;
            sReceiverId = jsData["receiver_id"].asString();
        }
        else if(JsonMemberExistsAndIsNull(jsData, "receiver_id"))
        {
            pmlLog(pml::LOG_DEBUG) << "NMOS: " << "Patch: receiver_id null: " ;
            nProperties |= FP_ID;
            sReceiverId.clear();
        }
        else if(jsData.isMember("receiver_id"))
        {
            bIsOk = false;
            pmlLog(pml::LOG_DEBUG) << "NMOS: " << "Patch: receiver_id incorrect type" ;
        }

    }
    else
    {
    pmlLog(pml::LOG_DEBUG) << "NMOS: " << "Patch: error" ;
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
        bMasterEnable = true;   //should this be set to true here?

        nProperties |= FP_TRANSPORT_FILE;
        if(jsData["transport_file"]["type"].isString())
        {
            sTransportFileType = jsData["transport_file"]["type"].asString();
        }
        else if(jsData["transport_file"].isMember("type") && jsData["transport_file"]["type"].isNull() == false)
        {
            bIsOk = false;
            pmlLog(pml::LOG_DEBUG) << "NMOS: " << "Patch: transport_file type incorrect type" ;
        }
        if(jsData["transport_file"]["data"].isString())
        {
            sTransportFileData = jsData["transport_file"]["data"].asString();
        }
        else if(jsData["transport_file"].isMember("data") && jsData["transport_file"]["data"].isNull() == false)
        {
            bIsOk = false;
            pmlLog(pml::LOG_DEBUG) << "NMOS: " << "Patch: transport_file data incorrect type" ;
        }

        if(sTransportFileType == "application/sdp" && sTransportFileData.empty() == false)
        {
            pmlLog(pml::LOG_DEBUG) << "NMOS: " << "Patch: transport_file data correct" ;
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
            pmlLog(pml::LOG_DEBUG) << "NMOS: " << "Patch: transport_params not an array." ;
            bIsOk = false;
        }
        else
        {
            tpReceiver.bRtpEnabled = bMasterEnable;
        }

        if(jsData["sender_id"].isString())
        {
            nProperties |= FP_ID;
            sSenderId = jsData["sender_id"].asString();
            pmlLog(pml::LOG_DEBUG) << "NMOS: " << "PatchReceiver Connection: sender_id is " << sSenderId ;
        }
        else if(JsonMemberExistsAndIsNull(jsData, "sender_id"))
        {
            pmlLog(pml::LOG_DEBUG) << "NMOS: " << "PatchReceiver Connection: sender_id is NULL" ;
            sSenderId.clear();
        }
        else if(jsData.isMember("sender_id") != false)
        {
            bIsOk = false;
            pmlLog(pml::LOG_DEBUG) << "NMOS: " << "Patch: sender_id incorrect type" ;
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
