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

#include "GameFramework/GameSession.h"

#include "AVVMGameSession.generated.h"

class APlayerState;

/**
 *	Class description:
 *	
 *	AAVVMGameSession is a server sided actor who tracking player connection to gameplay session.
 */
UCLASS()
class AVVMGAMEPLAY_API AAVVMGameSession : public AGameSession
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	static int32 GetUserUniqueId(const UWorld* World, const APlayerState* PlayerState);

protected:
	static AAVVMGameSession* Get(const UWorld* World);
	int32 GetUserUniqueId(const APlayerState* PlayerState) const;

	UPROPERTY(Transient)
	TMap<TWeakObjectPtr<const APlayerState>, int32/*{FActorContent.UniqueId}*/> UserUniqueIds;
};
