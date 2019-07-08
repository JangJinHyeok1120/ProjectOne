// Fill out your copyright notice in the Description page of Project Settings.

#include "Launcher.h"
#include "Rocket.h"
ULauncher::ULauncher() 
{
	static ConstructorHelpers::FObjectFinder<UParticleSystem> SHOOT(TEXT("ParticleSystem'/Game/Luncher_Fx/Fx/MuzzleFire_Fx.MuzzleFire_Fx'"));
	if (SHOOT.Succeeded()) {
		ShootEffect = SHOOT.Object;
	}
	MuzzleScale.Set(1.0f, 1.0f, 1.0f);
	
}

void ULauncher::SpawnBullet(FVector SpawnPos, FRotator SpawnRotation)
{
	auto Bullet = GetWorld()->SpawnActor<ARocket>(SpawnPos, SpawnRotation);
	if (Bullet)
	{
		Bullet->SetDirection(GetAimVector(SpawnPos));
		Bullet->SetStartPos(SpawnPos);
		Bullet->SetOwner(GetOwner());
		Bullet->SetSpeed(Owner->CharacterStat->GetWeaphoneStatData()->BulletSpeed);
	}
}