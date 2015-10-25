// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#include "bos_test2.h"
#include "bos_test2GameMode.h"
#include "bos_test2Ball.h"

Abos_test2GameMode::Abos_test2GameMode()
{
	// set default pawn class to our ball
	DefaultPawnClass = Abos_test2Ball::StaticClass();
}
