// Fill out your copyright notice in the Description page of Project Settings.

#include "SuperGun.h"
#include "Player/POComponents/PlayerStatComponent.h"
#include "Bullet.h"
#include "Rocket.h"
#include "Player/ProjectOneCharacter.h"
#include "Player/POComponents/POPlayerAkComponent.h"
#include "ProjectOne/Environments/GameModes/ProjectOneGameInstance.h"
#include "Player/WolfCharacter.h"
#include "Player/RifleCharacter.h"

// Sets default values for this component's properties
USuperGun::USuperGun()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.

	PrimaryComponentTick.bCanEverTick = false;
	Owner = Cast<AProjectOneCharacter>(GetOwner());
	CanShot = true;



	static ConstructorHelpers::FObjectFinder<UParticleSystem> SHOOT(TEXT("ParticleSystem'/Game/Action_FX/ParticleSystems/MuzzleFlash/P_MuzzleFlash1.P_MuzzleFlash1'"));
	if (SHOOT.Succeeded()) {
		ShootEffect = SHOOT.Object;
	}

	SoundManager = CreateDefaultSubobject<UPOPlayerAkComponent>(TEXT("Sound"));
	SoundManager->SetupAttachment(this);

	MuzzleScale.Set(0.25f, 0.25f, 0.25f);

	AiSpreadVal = 8.0f;
	// ...
}

void USuperGun::Reload()
{
	CurBullet = Owner->CharacterStat->GetWeaphoneStatData()->MaxBullet;
}

void USuperGun::Fire(FVector MuzzlePos, bool isAI)
{
	if (!CanShot)
		return;
	CanShot = false;

	//연사속도 타이머 설정
	float CSSpeed = 0;
	if (Owner->CharacterStat->GetWeaphoneStatData() != nullptr)
		CSSpeed = Owner->CharacterStat->GetWeaphoneStatData()->CSSpeed;
	GetWorld()->GetTimerManager().SetTimer(CSTimer, this, &USuperGun::RefireCheck, CSSpeed, true, CSSpeed);

	//애니메이션 종료
	if (CurBullet <= 0)
	{
		Owner->APAnim->Montage_Stop(0.5f, Owner->APAnim->ShotMontage);
		return;
	}

	//애니메이션 실행
	if (!Owner->APAnim->Montage_IsPlaying(Owner->APAnim->ShotMontage)
		&& !Owner->APAnim->isRoll)
	{
		Owner->APAnim->Montage_Play(Owner->APAnim->ShotMontage);
	}



	//카메라 쉐이크
	if (UGameplayStatics::GetPlayerPawn(GetWorld(), 0)==GetOwner())
	{
		auto GameInstance = Cast<UProjectOneGameInstance>(Owner->GetGameInstance());
		GameInstance->HitShake(GameInstance->CShakePistol, 1.0f);
	}
	FTransform TestTrans;

	TestTrans = FTransform(Owner->GetMesh()->GetForwardVector().Rotation(), MuzzlePos);

	//이펙트 출력
	FTransform FireEffectTransform = TestTrans; // Owner->GetMesh()->GetSocketTransform(TEXT("Muzzle"));

	FireEffectTransform.SetScale3D(MuzzleScale);


	UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ShootEffect, FireEffectTransform);

	//사운드 출력
	if (Owner->GetEvolutionLevel() > 2) {
		switch (Owner->PlayerType)
		{
		case E_PlayerSelect::E_SMG:
			SoundManager->SetAkSwitch(TEXT("Weapon"), TEXT("Smg"));
			break;
		case E_PlayerSelect::E_LAUNCHER:
			SoundManager->SetAkSwitch(TEXT("Weapon"), TEXT("Launcher"));
			break;
		case E_PlayerSelect::E_RF:
			SoundManager->SetAkSwitch(TEXT("Weapon"), TEXT("Sniper"));
			break;
		default:
			break;
		}
	}
	SoundManager->PlayPistolShotSound();

	FRotator SpreadRotation = GetAimVector(MuzzlePos).Rotation();

	//if (isAI) {

	//}
	if ((GetOwner()->IsA(AWolfCharacter::StaticClass()) && Cast<AProjectOneCharacter>(GetOwner())->GetEvolutionLevel() > 2))
	{
		float spreadVal = 1.5f;
		float RandPitch = FMath::RandRange(-(spreadVal*0.5f*1.5f), spreadVal*0.5f*1.5f);
		float RandYaw = FMath::RandRange(-(spreadVal*0.5f*1.5f), spreadVal*0.5f*1.5f);

		SpreadRotation = FRotator(
			SpreadRotation.Pitch + RandPitch,
			SpreadRotation.Yaw + RandYaw, 0.0f);
	}

	if (isAI) {
		float RandPitch = FMath::RandRange(-(AiSpreadVal*0.5f*1.5f), AiSpreadVal*0.5f*1.5f);
		float RandYaw = FMath::RandRange(-(AiSpreadVal*0.5f*1.5f), AiSpreadVal*0.5f*1.5f);

		SpreadRotation = FRotator(
			SpreadRotation.Pitch + RandPitch,
			SpreadRotation.Yaw + RandYaw, 0.0f);
	}
	//총알생성
	SpawnBullet(MuzzlePos,SpreadRotation);
	CurBullet--;

	//총반동
	float PitchVal = FMath::RandRange(0.15f * 0.7f, 0.15f);
	float YawVal = FMath::RandRange(-0.1f * 0.5f, 0.1f*0.5f);

	Owner->AddControllerPitchInput(-PitchVal);
	Owner->AddControllerYawInput(YawVal);

	if (GetOwner()->IsA(AWolfCharacter::StaticClass()))
	{
		auto SMGPlayer = Cast<AWolfCharacter>(GetOwner());
		SMGPlayer->isRightShoot = SMGPlayer->isRightShoot ? false : true;
	}


}

void USuperGun::RefireCheck()
{
	CanShot = true;
	Owner->GetWorldTimerManager().ClearTimer(CSTimer);
}


int USuperGun::GetMaxBullet()
{
	return Owner->CharacterStat->GetWeaphoneStatData()->MaxBullet;
}

int USuperGun::GetCurBullet()
{
	return CurBullet;
}

void USuperGun::BeginPlay()
{
	Super::BeginPlay();
	if(Owner->CharacterStat->GetWeaphoneStatData())
	CurBullet = Owner->CharacterStat->GetWeaphoneStatData()->MaxBullet;
}


void USuperGun::ChangeWeapon(int WeaponNum)
{
	Owner->CharacterStat->SetWeaponeID(WeaponNum);
}

void USuperGun::SpawnBullet(FVector SpawnPos, FRotator SpawnRotation)
{
	auto Bullet = GetWorld()->SpawnActor<ABullet>(SpawnPos, SpawnRotation);
	if (Bullet)
	{
		Bullet->SetDirection(GetAimVector(SpawnPos));
		Bullet->SetStartPos(SpawnPos);
		Bullet->SetOwner(Owner);
		Bullet->SetSpeed(Owner->CharacterStat->GetWeaphoneStatData()->BulletSpeed);
	}
}

FVector USuperGun::GetAimVector(FVector StartPos)
{
	FHitResult OutHit;
	FVector CameraForwardVector = Owner->FollowCamera->GetComponentRotation().Vector();
	FVector AimStart = (CameraForwardVector*350.0f) + Owner->FollowCamera->GetComponentLocation();
	FVector Aimnd = (CameraForwardVector*10000.0f) + AimStart;
	FCollisionQueryParams CameraCollisionParams;

	FVector tmpVec = Aimnd;

	if (GetWorld()->LineTraceSingleByChannel(OutHit, AimStart, Aimnd, ECC_Pawn, CameraCollisionParams)) {
		tmpVec = OutHit.ImpactPoint;

	}

	FVector reVal = FVector::ZeroVector;
	reVal = tmpVec - StartPos;

	return reVal.GetSafeNormal();
}

void USuperGun::SetAiSpread(float SpreadVal)
{
	AiSpreadVal = SpreadVal;
}
