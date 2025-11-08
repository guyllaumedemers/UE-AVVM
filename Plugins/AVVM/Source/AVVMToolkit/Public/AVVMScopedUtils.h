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

/**
 *	Class description:
 *
 *	FAVVMScopedDelegate is a scoped object that execute a laten delegate during destruction.
 */
struct AVVMTOOLKIT_API FAVVMScopedDelegate
{
	FAVVMScopedDelegate(const FSimpleDelegate& Callback)
		: OutOfScopeDelegate(Callback)
	{
	}

	~FAVVMScopedDelegate()
	{
		OutOfScopeDelegate.ExecuteIfBound();
	}

private:
	FSimpleDelegate OutOfScopeDelegate = FSimpleDelegate();
};

/**
 *	Class description:
 *
 *	FAVVMGameThreadLock is a utility allowing management of async processes request, and should be used
 *	to prevent racing condition between game related updates and data received from backend.
 */
struct AVVMTOOLKIT_API FAVVMGameThreadLock
{
	void Lock()
	{
		bIsRunning = true;
	}

	void UnLock()
	{
		bIsRunning = false;
	}

	bool IsLocked() const
	{
		return bIsRunning;
	}

	/**
	 *	Class description:
	 *
	 *	FAVVMScopedLock is a utility offering scope safety for locking/unlocking behaviour.
	 */
	struct AVVMTOOLKIT_API FAVVMScopedLock
	{
		FAVVMScopedLock(FAVVMGameThreadLock* NewHandle)
			: Handle(NewHandle)
		{
			if (Handle != nullptr) { Handle->Lock(); }
		}

		FAVVMScopedLock(FAVVMGameThreadLock* NewHandle, const FSimpleDelegate& NewCallback)
			: Handle(NewHandle)
			  , Callback(NewCallback)
		{
			if (Handle != nullptr) { Handle->Lock(); }
		}

		~FAVVMScopedLock()
		{
			// @gdemers order matter, expect unlocking before invocation.
			if (Handle != nullptr) { Handle->UnLock(); }
			Callback.ExecuteIfBound();
		}

	private:
		FAVVMGameThreadLock* Handle = nullptr;
		FSimpleDelegate Callback;
	};

	FAVVMScopedLock Make() const
	{
		return FAVVMScopedLock(const_cast<FAVVMGameThreadLock*>(this));
	}

	FAVVMScopedLock Make(const FSimpleDelegate& NewCallback) const
	{
		return FAVVMScopedLock(const_cast<FAVVMGameThreadLock*>(this), NewCallback);
	}

private:
	bool bIsRunning = false;
};
