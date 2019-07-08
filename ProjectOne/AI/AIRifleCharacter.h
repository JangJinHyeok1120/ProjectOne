// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "ProjectOne.h"
#include "AI/AICharacter.h"
#include "AIRifleCharacter.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTONE_API AAIRifleCharacter : public AAICharacter
{
	GENERATED_BODY()
public:
	AAIRifleCharacter();
	void SetResources() override;
	void Aim() override;
	void AimRelease() override;

	FTimerHandle AimTimerHendle;
protected:
	bool isFullAim;
	void AimTimerFunc();
	void Evolution() override;
};
