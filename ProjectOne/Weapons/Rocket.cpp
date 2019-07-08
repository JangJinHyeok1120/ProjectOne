// Fill out your copyright notice in the Description page of Project Settings.

#include "Rocket.h"
#include "Player/ProjectOneCharacter.h"
#include "ProjectOne/Player/POComponents/POPlayerAkComponent.h"

ARocket::ARocket() 
{
	Trail = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("Rocket"));

	static ConstructorHelpers::FObjectFinder<UParticleSystem> ROCKET_EFFECT(TEXT("ParticleSystem'/Game/Luncher_Fx/Fx/LuncherTrail_Fx.LuncherTrail_Fx'"));

	if (ROCKET_EFFECT.Succeeded()) {
		Trail->SetTemplate(ROCKET_EFFECT.Object);
	}
	SetRootComponent(Trail);
	Trail->SetWorldScale3D(FVector(3.0f));
	ExplosionParticle = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("Particle"));
	static ConstructorHelpers::FObjectFinder<UParticleSystem> ParticleObj(TEXT("ParticleSystem'/Game/Luncher_Fx/Fx/Explosion_Fx.Explosion_Fx'"));
	if (ParticleObj.Succeeded())
	{
		ExplosionParticle->SetTemplate(ParticleObj.Object);
	}
	ExplosionParticle->SetWorldScale3D(FVector(7.0f));
	ExplosionParticle->TranslucencySortPriority = -10;

}

void ARocket::OnHit(FHitResult OutHit)
{
		TArray<FOverlapResult> OverlapResults;
		FCollisionQueryParams CollisionQueryParam(NAME_None, false, this);
		bool bResult = GetWorld()->OverlapMultiByChannel(
			OverlapResults,
			OutHit.Location,
			FQuat::Identity,
			ECollisionChannel::ECC_Pawn,
			FCollisionShape::MakeSphere(200.0f),
			CollisionQueryParam
		);


		if (bResult)
		{
			for (auto OverlapResult : OverlapResults)
			{
				if (OverlapResult.Actor->IsA(AProjectOneCharacter::StaticClass()))
				{
					float Damage = Cast<AProjectOneCharacter>(GetOwner())->CharacterStat->GetWeaphoneStatData()->Damage;
					Cast<AProjectOneCharacter>(OverlapResult.Actor)->Hit(Damage, false, GetOwner());
				}
			}
		}
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplosionParticle->Template, FTransform(FRotator(0,0,0),OutHit.Location,FVector(4.0f)));
		SoundManager->SetAkSwitch(TEXT("Hit"), TEXT("Rocket"));
		SoundManager->PlayBulletCrashSound();
}