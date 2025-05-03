//Copyright(c) 2025 gdemers
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
#pragma once

#include "CoreMinimal.h"

#include "AVVM.h"
#include "AVVMOnlineInterface.h"
#include "MVVMViewModelBase.h"
#include "StructUtils/InstancedStruct.h"

#include "AVVMPlayerProfileViewModel.generated.h"

/**
 *	Class description:
 *
 *	UAVVMPlayerProfileViewModel. View Model class that display backend representation of the player.
 */
UCLASS()
class AVVMSAMPLERUNTIME_API UAVVMPlayerProfileViewModel : public UMVVMViewModelBase,
                                                          public IAVVMViewModelFNameHelper
{
	GENERATED_BODY()

public:
	virtual FName GetViewModelFName() const override { return TEXT("UAVVMPlayerProfileViewModel"); };
	// @gdemers we cast the received payload to our expected type so the MVVM plugin
	// can properly exposed aggregated variable of Notification Payload.
	void SetPlayerProfile(const TInstancedStruct<FAVVMNotificationPayload>& Payload);

protected:
	UPROPERTY(Transient, BlueprintReadOnly, FieldNotify)
	FAVVMPlayerProfile PlayerProfile;
};
