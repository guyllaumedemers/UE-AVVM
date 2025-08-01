﻿//Copyright(c) 2025 gdemers
//
//Permission is hereby granted, free of charge, to any person obtaining a copy
//of this software and associated documentation files(the "Software"), to deal
//in the Software without restriction, including without limitation the rights
//to use, copy, modify, merge, publish, distribute, sublicense, and /or sell
//copies of the Software, and to permit persons to whom the Software is
//furnished to do so, subject to the following conditions :
//
//The above copyright notice and this permission notice shall be included in all
//copies or substantial portions of the Software.
//
//THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
//AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//SOFTWARE.
#include "Archetypes/AVVMPresenter.h"

#include "AVVM.h"
#include "AVVMSubsystem.h"
#include "MVVMViewModelBase.h"

void UAVVMPresenter::SafeBeginPlay()
{
	UE_LOG(LogUI, Log, TEXT("Adding UAVVMPresenter::%s"), *GetClass()->GetName());

	{
		FAVVMPresenterContextArgs ContextArgs;
		ContextArgs.WorldContext = UAVVMPresenter::GetWorld();
		ContextArgs.Presenter = this;
		ViewModel = UAVVMSubsystem::Static_RegisterPresenter(ContextArgs);
	}

	{
		BP_RegisterNotificationChannels();
	}

	if (bAllowDeferredBroadcast)
	{
		BP_BroadcastDeferredNotifications();
	}
}

void UAVVMPresenter::SafeEndPlay()
{
	UE_LOG(LogUI, Log, TEXT("Removing UAVVMPresenter::%s"), *GetClass()->GetName());

	{
		// @gdemers ensure cleanup during destroy sequence so our UI remains in a valid state!
		StopPresenting();
	}

	{
		FAVVMPresenterContextArgs ContextArgs;
		ContextArgs.WorldContext = UAVVMPresenter::GetWorld();
		ContextArgs.Presenter = this;
		UAVVMSubsystem::Static_UnregisterPresenter(ContextArgs);
	}

	{
		BP_UnregisterNotificationChannels();
	}
}
