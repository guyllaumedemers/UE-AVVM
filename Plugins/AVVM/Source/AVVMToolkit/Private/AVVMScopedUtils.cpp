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
#include "AVVMScopedUtils.h"

FAVVMScopedDelegate::FAVVMScopedDelegate(FSimpleDelegate&& NewCallback)
	: OutOfScopeDelegate(MoveTemp(NewCallback))
{
}

FAVVMScopedDelegate::~FAVVMScopedDelegate()
{
	OutOfScopeDelegate.ExecuteIfBound();
}

FAVVMScopedLock::FAVVMScopedLock(FAVVMGameThreadLock* NewHandle)
	: Handle(NewHandle)
{
	if (Handle != nullptr) { Handle->Lock(); }
}

FAVVMScopedLock::FAVVMScopedLock(FAVVMGameThreadLock* NewHandle, FSimpleDelegate&& NewCallback)
	: Handle(NewHandle),
	  Callback(MoveTemp(NewCallback))
{
	if (Handle != nullptr) { Handle->Lock(); }
}

FAVVMScopedLock::~FAVVMScopedLock()
{
	// @gdemers order matter, expect unlocking before invocation.
	if (Handle != nullptr) { Handle->UnLock(); }
	Callback.ExecuteIfBound();
}

FAVVMScopedLock FAVVMGameThreadLock::Make()
{
	return FAVVMScopedLock{this};
}

FAVVMScopedLock FAVVMGameThreadLock::Make(FSimpleDelegate&& NewCallback)
{
	return FAVVMScopedLock{this, MoveTemp(NewCallback)};
}

void FAVVMGameThreadLock::Lock()
{
	bIsRunning = true;
}

void FAVVMGameThreadLock::UnLock()
{
	bIsRunning = false;
}

bool FAVVMGameThreadLock::IsLocked() const
{
	return bIsRunning;
}
