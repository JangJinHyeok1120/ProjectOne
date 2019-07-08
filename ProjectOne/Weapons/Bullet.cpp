// Fill out your copyright notice in the Description page of Project Settings.

#include "Bullet.h"
#include "AI/AICharacter.h"
#include "Kismet/GamePlayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "ProjectOne/Player/POComponents/POPlayerAkComponent.h"
#include "Player/POComponents/PlayerStatComponent.h"
#include "Environments/GameModes/ProjectOneGameInstance.h"


// Sets default values
ABullet::ABullet()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Col = CreateDefaultSubobject<USphereComponent>(TEXT("SPHERE"));
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	Hit = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("Hit"));
	Trail = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("Trail"));

	RootComponent = Col;

	Hit->SetupAttachment(RootComponent);
	Hit->SetRelativeLocation(FVector(0.0f, 0.0f, 0.0f));

	Trail->SetupAttachment(RootComponent);
	Trail->SetRelativeLocation(FVector(0.0f, 0.0f, 0.0f));

	Col->SetSphereRadius(50.0f);
	Speed = 70.0f;

	Col->SetCollisionProfileName(TEXT("Bullet"));

	DirectionVector = GetActorForwardVector();
	SetActorScale3D(FVector(0.2f, 0.05f, 0.05f));
	SetActorScale3D(GetActorScale()*1.0f);

	static ConstructorHelpers::FObjectFinder<UParticleSystem> HIT(TEXT("ParticleSystem'/Game/Action_FX/ParticleSystems/MuzzleFlash/P_Hit_1Spawn.P_Hit_1Spawn'"));
	if (HIT.Succeeded()) {
		Hit->SetTemplate(HIT.Object);
	}

	static ConstructorHelpers::FObjectFinder<UParticleSystem> TRAIL(TEXT("ParticleSystem'/Game/Action_FX/ParticleSystems/Magic_Bullet/Bullet.Bullet'"));
	if (TRAIL.Succeeded()) {
		Trail->SetTemplate(TRAIL.Object);
	}


	static ConstructorHelpers::FObjectFinder<UMaterialInstance> ScratchMatObj(TEXT("MaterialInstanceConstant'/Game/Materail/BulletScratch_Decal_Inst.BulletScratch_Decal_Inst'"));
	if(ScratchMatObj.Succeeded())
	{
		DecalMaterialinstance = ScratchMatObj.Object;
	}


	SoundManager = CreateDefaultSubobject<UPOPlayerAkComponent>(TEXT("Sound"));
	SoundManager->SetupAttachment(RootComponent);

	Hit->DeactivateSystem();
	Hit->SetVisibility(false);
	isStartRay = true;
	isHit = false;
	ProjectileMovement->ProjectileGravityScale = 0.0f;
	ProjectileMovement->InitialSpeed = 30000.0f;
	LifeTime = 0.0f;

}

void ABullet::SetStartPos(FVector Position)
{
	PreLocation = Position;
}


void ABullet::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	//   Col->OnComponentBeginOverlap.AddDynamic(this, &ABullet::OnCollisionOverlap);
}

void ABullet::BeginPlay()
{
	Super::BeginPlay();

}
// Called every frame
void ABullet::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	FHitResult OutHit;
	FVector StartRay = GetActorLocation();
	FVector EndRay = (DirectionVector * 1000.0f) + GetActorLocation();
	FCollisionQueryParams CollisionParam;

	float Damage = Cast<AProjectOneCharacter>(GetOwner())->CharacterStat->GetWeaphoneStatData()->Damage;


	if (isStartRay) {
		StartRay = PreLocation;
		isStartRay = false;
	}

	if (GetWorld()->LineTraceSingleByChannel(OutHit, StartRay, EndRay, ECC_Pawn, CollisionParam)) {
		if (!isHit) {
			isHit = true;
			OnHit(OutHit);
		}
	}

	LifeTime += DeltaTime;


	TArray<FOverlapResult> OverlapBullets;
	FCollisionQueryParams CollisionQuery(NAME_None, false, this);

	bool bulletResult = GetWorld()->OverlapMultiByChannel(
		OverlapBullets,
		GetActorLocation(),
		FQuat::Identity,
		ECollisionChannel::ECC_GameTraceChannel5,
		FCollisionShape::MakeSphere(300.0f),
		CollisionQuery
	);

	if (bulletResult)
	{
		if (GetOwner() != OverlapBullets[0].Actor) {
			auto AI = Cast<AAICharacter>(OverlapBullets[0].Actor);

			if (AI)
			{
				AI->Attacked(GetOwner());
			}
		}
	}


	if (LifeTime > 5.0f)
	{
		Destroy();
	}


}

void ABullet::OnHit(FHitResult OutHit) {

	auto Character = Cast<AProjectOneCharacter>(OutHit.Actor);
	auto Ai = Cast<AAICharacter>(OutHit.Actor);

	if (Character)
	{
		bool isHeadShot = "Bip001-Head" == OutHit.BoneName;
		float Damage = Cast<AProjectOneCharacter>(GetOwner())->CharacterStat->GetWeaphoneStatData()->Damage;
		Character->Hit(Damage, isHeadShot, GetOwner());
	}

	if (Ai)
	{
		Ai->Attacked(GetOwner());
	}

	if (!Character) 
	{
		SoundManager->SetAkSwitch(TEXT("Hit"), TEXT("Bullet"));
		SoundManager->PlayBulletCrashSound();
	}
	UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), Hit->Template, OutHit.Location);
}

//
//void ABullet::OnCollisionOverlap(UPrimitiveComponent * OverlappedComp, AActor * OtherActor, UPrimitiveComponent * OherCcomp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
//{
//   ABLOG_S(Warning);
//   ABCHECK(GetOwner());
//   //CreateDecal
//   
//   UGameplayStatics::SpawnDecalAtLocation(GetWorld(), DecalMaterialinstance,
//         FVector(30.0f, 5.0f, 5.0f), this->GetActorLocation(),
//      Normal.Rotation(), 100.0f);
//
//   
//   ABLOG(Warning, TEXT("Owner : %s"), *(GetOwner()->GetName()));
//   ABLOG(Warning, TEXT("OtherActor : %s"), *(OtherActor->GetName()));
//
//   //ABCHECK(IsValid(Cast<AProjectOneCharacter>(OtherActor)));
//   auto Character = Cast<AProjectOneCharacter>(OtherActor);
//   if (Character)
//      Character->5.0f, GetOwner());
//
//   //ABCHECK(IsValid(Cast<AAICharacter>(OtherActor)));
//   auto AI = Cast<AAICharacter>(OtherActor);
//   if (AI)
//      AI->Hit(10.0f, GetOwner());
//
//
//
//   Destroy(this);
//
//}

void ABullet::SetDirection(FVector direction)
{
	DirectionVector = direction;
	SetActorRotation(direction.Rotation());
}

void ABullet::SetSpeed(float speed)
{
	ProjectileMovement->MaxSpeed = speed;
}
