// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "ProjectOne.h"
#include "AI/AICharacter.h"
#include "AILauncherCharacter.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTONE_API AAILauncherCharacter : public AAICharacter
{
	GENERATED_BODY()
public:
	AAILauncherCharacter();
	void SetResources() override;
protected:
	class USuperGun* Launcher;
	void ChangeWeapon() override;
	void Evolution() override;
};
