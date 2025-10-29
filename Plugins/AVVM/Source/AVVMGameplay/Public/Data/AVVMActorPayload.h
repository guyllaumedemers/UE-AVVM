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

#include "AVVMNotificationSubsystem.h"

#include "AVVMActorPayload.generated.h"

class APlayerController;
class APlayerState;

/**
 *	Class description:
 *
 *	FAVVMActorContext is a context struct that expose Actor specific information for UI display. To not be confused with the AVVMOnline api
 *	exposing backend representation of a player.
 */
USTRUCT(BlueprintType)
struct AVVMGAMEPLAY_API FAVVMActorContext
{
	GENERATED_BODY()
};

/**
 *	Class description:
 *
 *	IAVVMCanExposeActorPayload is an interface class that allow abstraction around an actor payload information. Its expected to be used
 *	with the AVVMNotification system to forward 'FAVVMActorPayload' information to View Models.
 *
 *	Warning : Keep in mind that this data access return by copy, and as such, should be avoided if recurrent access if required. The purpose
 *	of this is merely allow ease of use for data synchronization specific to immutable player information. (*Exception may apply if you are
 *	ready to pay the price of provide data by copy.)
 */
UINTERFACE(BlueprintType)
class AVVMGAMEPLAY_API UAVVMCanExposeActorPayload : public UInterface
{
	GENERATED_BODY()
};

class AVVMGAMEPLAY_API IAVVMCanExposeActorPayload
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	TInstancedStruct<FAVVMActorContext> GetExposedActorContext() const;
	virtual TInstancedStruct<FAVVMActorContext> GetExposedActorContext_Implementation() const PURE_VIRTUAL(GetExposedActorContext_Implementation, return Empty;);

private:
	static TInstancedStruct<FAVVMActorContext> Empty;
};

/**
 *	Class description:
 *
 *	FAVVMActorPayload is a context struct that expose player specific information.
 */
USTRUCT()
struct AVVMGAMEPLAY_API FAVVMActorPayload : public FAVVMNotificationPayload
{
	GENERATED_BODY()

	FAVVMActorPayload() = default;
	FAVVMActorPayload(const TScriptInterface<const IAVVMCanExposeActorPayload>& NewPayloadOwner);

	UPROPERTY(Transient, BlueprintReadOnly)
	TScriptInterface<const IAVVMCanExposeActorPayload> PayloadOwner = nullptr;
};
