#pragma once

#include "BaseInkWidgetController.hpp"

namespace CyberEyeTracking::Workers
{
    class LoadingGameWorker : public BaseInkWidgetController
    {
    public:
        LoadingGameWorker(char* name)
            : BaseInkWidgetController(name){};
        ~LoadingGameWorker() = default;

        void Init();
        bool Loading();
    };
}
