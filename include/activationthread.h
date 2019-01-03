#pragma once
#include <chrono>
#include <string>
#include <memory>

class EventPoster;


static void ActivationThreadReceiver(const std::chrono::time_point<std::chrono::high_resolution_clock>& tp, const std::string& sReceiverId, std::shared_ptr<EventPoster> pPoster);

