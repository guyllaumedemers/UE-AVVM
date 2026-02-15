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

#include "Engine/DeveloperSettings.h"

#include "TeamSettings.generated.h"

class ULevel;
class UTeamRule;
class UTeamSpawnRule;

/**
 *	Class description:
 *
 *	UTeamSettings is a DeveloperSettings that expose global properties to run condition during Team selection/Spawn.
 */
UCLASS(config="Game", DefaultConfig, meta=(DisplayName="UTeamSettings"))
class TEAMSAMPLE_API UTeamSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UTeamSettings();
	
	UFUNCTION(BlueprintCallable, Category="Team|Settings")
	static const TSoftClassPtr<UTeamRule>& GetTeamRuleClass();
	
	UFUNCTION(BlueprintCallable, Category="Team|Settings")
	static const TSoftClassPtr<UTeamSpawnRule>& GetTeamSpawnRuleClass();
	
	UFUNCTION(BlueprintCallable, Category="Team|Settings")
	static bool DoesLevelOverrideSubsystemCreation(const ULevel* NewLevel);

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Config, Category="Designers")
	TSoftClassPtr<UTeamRule> TeamRuleClass = nullptr;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Config, Category="Designers")
	TSoftClassPtr<UTeamSpawnRule> TeamSpawnRuleClass = nullptr;
	
	// @gdemers Perform TeamSubsystem override and allow its creation process to run on NM_Client
	// for user-defined level such as Lobby/MainMenu, i.e whenever we are without server connection.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Config, Category="Designers")
	TArray<TSoftClassPtr<ULevel>> LevelOverrides;
};
