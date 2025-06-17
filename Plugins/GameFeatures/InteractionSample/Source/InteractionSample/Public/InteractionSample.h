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

#include "Modules/ModuleManager.h"

/**
 *	Plugin Description :
 *
 *	InteractionSample is a contingency system that handle UPlayer interaction between one (or more) locally controlled APlayerController
 *	and a replicated world actor. During gameplay, the GFP will push a component on the AGameStateBase and react to RPC calls when
 *	attempting a local interaction.
 *
 *	Example :
 *	
 *		* Door
 *		* Chess
 *		* etc...
 *
 *	This system allow granting of the required 'Activation' tag, so the locally controlled APlayerController can execute
 *	the UPlayerInteractionAbility, and for them only! (unless contingency is not an issue on the target object, in which case, the flag
 *	can be turned off on the world actor UActorInteractionComponent)
 */
