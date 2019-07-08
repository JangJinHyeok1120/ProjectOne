// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "ProjectOne.h"
#include "AI/AICharacter.h"
#include "AISmgCharacter.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTONE_API AAISmgCharacter : public AAICharacter
{
	GENERATED_BODY()

public:
	AAISmgCharacter();
	void SetResources() override;
	void Shooting(float tick) override;
	void Evolution() override;

	bool isRightShoot;
};
