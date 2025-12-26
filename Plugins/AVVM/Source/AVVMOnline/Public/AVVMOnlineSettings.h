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

#include "GameplayTagContainer.h"
#include "Engine/DeveloperSettings.h"
#include "Templates/SubclassOf.h"

#include "AVVMOnlineSettings.generated.h"

class UAVVMOnlinePlayerStringParser;

/**
*	Class description:
 *
 *	UAVVMOnlineSettings is a DeveloperSetting for global variable that require access in this plugin.
 */
UCLASS(config="Game", DefaultConfig, meta=(DisplayName="UAVVMOnlineSettings"))
class AVVMONLINE_API UAVVMOnlineSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UAVVMOnlineSettings();

	UFUNCTION(BlueprintCallable, Category="AVVMOnline|Settings")
	static TSubclassOf<UAVVMOnlinePlayerStringParser> GetJsonParserClass_Player();

	UFUNCTION(BlueprintCallable, Category="AVVMOnline|Settings")
	static const FGameplayTag& GetPlayerStateChannelTag();

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Config, Category="Designers")
	TSubclassOf<UAVVMOnlinePlayerStringParser> JsonParserClass_Player = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Config, Category="Designers")
	FGameplayTag PlayerStateChannelTag = FGameplayTag::EmptyTag;
};
