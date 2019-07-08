// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapons/SuperGun.h"
#include "Launcher.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTONE_API ULauncher : public USuperGun
{
	GENERATED_BODY()
public:
	ULauncher();
	void SpawnBullet(FVector SpawnPos, FRotator SpawnRotation) override;
};
