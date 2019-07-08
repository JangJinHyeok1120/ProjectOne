// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapons/Bullet.h"
#include "Rocket.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTONE_API ARocket : public ABullet
{
	GENERATED_BODY()

public:
	ARocket();
	
protected:
	void OnHit(FHitResult OutHit) override;

	UPROPERTY(EditAnywhere)
	UParticleSystemComponent* ExplosionParticle;
};
