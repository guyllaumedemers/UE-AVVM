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

#include "GameFramework/CheatManager.h"

#include "AVVMClientExecutorCheatExtension.generated.h"

/**
 *	Class description:
 *	
 *	UAVVMClientExecutorCheatExtension is the base impl for processing console cheats from a Client NetMode. It allows Server exec
 *	of client sided console command execution without having to define Exec api on the PlayerController/PlayerState/or ACharacter.
 *	
 *	IMPORTANT : Dont define UFUNTION(Exec) in the above mentioned class. Just create new UCheatManagerExtension, and prevent code bloat in
 *	relevant class such as APlayerController. (be smart! do the extra work for better scalability.)
 */
UCLASS()
class AVVMTOOLKIT_API UAVVMClientExecutorCheatExtension : public UCheatManagerExtension
{
	GENERATED_BODY()

public:
	virtual bool ProcessConsoleExec(const TCHAR* Cmd, FOutputDevice& Ar, UObject* Executor) override;
};
