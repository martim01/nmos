#pragma once
#include "nmosapiversion.h"
#include "nmosdlldefine.h"
#include <chrono>
#include "json/json.h"
#include "optional.hpp"


namespace pml
{
    namespace nmos
    {
        class activation
        {
            public:
                enum enumActivate {ACT_NULL, ACT_NOW, ACT_ABSOLUTE, ACT_RELATIVE};
                static const std::array<std::string,4> STR_ACTIVATE;
                activation();
                activation(const activation& other);
                activation& operator=(const activation& other);
                const Json::Value GetJson() const;

                enumActivate GetMode() const;
                std::experimental::optional<std::string> GetRequestedTime() const;
                std::experimental::optional<std::chrono::time_point<std::chrono::high_resolution_clock>> GetRequestedTimePoint() const;

                void SetMode(enumActivate eMode);
                void SetRequestedTime(std::experimental::optional<std::chrono::time_point<std::chrono::high_resolution_clock>> when);
                virtual void Clear();

                virtual bool Patch(const Json::Value& jsData)=0;
                protected:
                    Json::Value m_json;
        };

        class activationRequest : public activation
        {
            public:
                activationRequest();
                activationRequest(const Json::Value& jsResponse);
                void Setup(activation::enumActivate eMode, std::experimental::optional<std::chrono::time_point<std::chrono::high_resolution_clock>> when);

                activationRequest(const activationRequest& other);

                bool Patch(const Json::Value& jsData) override;

                static bool CheckJson(const Json::Value& jsObject);
        };

        class activationResponse : public activation
        {
            public:
                activationResponse();
                activationResponse(const activationResponse& other);
                activationResponse(const Json::Value& jsResponse);

                bool Patch(const Json::Value& jsData) override;

                std::experimental::optional<std::string> GetActivationTime() const;
                std::experimental::optional<std::chrono::time_point<std::chrono::high_resolution_clock>> GetActivationTimePoint() const;

                void SetActivationTime(std::experimental::optional<std::chrono::time_point<std::chrono::high_resolution_clock>> when);

                void Clear() override;

                static bool CheckJson(const Json::Value& jsObject);
        };

    };
};
