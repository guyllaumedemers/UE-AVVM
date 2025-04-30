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
// ReSharper disable CppIncompleteSwitchStatement
// ReSharper disable CppDefaultCaseNotHandledInSwitchStatement
#include "UISampleFactoryImpl.h"

#include "AVVMNotificationSubsystem.h"

TScriptInterface<IAVVMResolverExecutioner> USampleFactoryImpl::Factory(const EAVVMObserverResolverFlag ResolverFlag) const
{
	TScriptInterface<IAVVMResolverExecutioner> Out = nullptr;
	switch (ResolverFlag)
	{
		case EAVVMObserverResolverFlag::ActorName:
			Out = NewObject<USampleActorNameExecutioner>();
			break;
		case EAVVMObserverResolverFlag::ActorClassName:
			Out = NewObject<USampleActorClassNameExecutioner>();
			break;
	}

	return Out;
}

TArray<TScriptInterface<IAVVMObserver>> USampleActorNameExecutioner::Filter(const FString& MatchRequirement,
                                                                            const TArray<TScriptInterface<IAVVMObserver>>& Observers) const
{
	return TArray<TScriptInterface<IAVVMObserver>>{};
}

TArray<TScriptInterface<IAVVMObserver>> USampleActorClassNameExecutioner::Filter(const FString& MatchRequirement,
                                                                                 const TArray<TScriptInterface<IAVVMObserver>>& Observers) const
{
	return TArray<TScriptInterface<IAVVMObserver>>{};
}
