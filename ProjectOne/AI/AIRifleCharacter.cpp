// Fill out your copyright notice in the Description page of Project Settings.

#include "AIRifleCharacter.h"
#include "Player/POComponents/PlayerStatComponent.h"
#include "Weapons/SuperGun.h"
#include "ProjectOne/Environments/GameModes/ProjectOneGameInstance.h"

AAIRifleCharacter::AAIRifleCharacter() {
	SetResources();
}

void AAIRifleCharacter::SetResources()
{
	static ConstructorHelpers::FClassFinder<UCameraShake> CSHAKE_FIRE(TEXT("Blueprint'/Game/ScreenFX/CShake_Fire.CShake_Fire_C'"));
	if (CSHAKE_FIRE.Succeeded())
		CShakeList.Add(CSHAKE_FIRE.Class);

	static ConstructorHelpers::FObjectFinder<USkeletalMesh> SK_MANEQUIN(TEXT("SkeletalMesh'/Game/Animations/Wolf1/SK_Wolf1_Idle.SK_Wolf1_Idle'"));
	if (SK_MANEQUIN.Succeeded()) {
		GetMesh()->SetSkeletalMesh(SK_MANEQUIN.Object);
		FirstMesh = SK_MANEQUIN.Object;
	}

	GetMesh()->SetRelativeLocationAndRotation(FVector(0.0f, 0.0f, -42.0f), FRotator(0.0f, -90.0f, 0.0f));

	static ConstructorHelpers::FClassFinder<UAnimInstance> TEMP_ANIM(TEXT("AnimBlueprint'/Game/Animations/Wolf1/BP_Wolf1.BP_Wolf1_C'"));
	if (TEMP_ANIM.Succeeded()) {
		GetMesh()->SetAnimInstanceClass(TEMP_ANIM.Class);
	}

	static ConstructorHelpers::FObjectFinder<USkeletalMesh> SK_Sec(TEXT("SkeletalMesh'/Game/Animations/Wolf2/Rifle/SK_Wolf2_Attack_Rifle.SK_Wolf2_Attack_Rifle'"));
	if (SK_Sec.Succeeded()) {
		SecondSkMesh = SK_Sec.Object;
	}

	static ConstructorHelpers::FObjectFinder<USkeletalMesh> SK_THIRD(TEXT("SkeletalMesh'/Game/Animations/Wolf3/Rifle/SK_Wolf3_Attack_Rifle.SK_Wolf3_Attack_Rifle'"));
	if (SK_THIRD.Succeeded()) {
		ThirdMesh = SK_THIRD.Object;
	}

	static ConstructorHelpers::FClassFinder<UAnimInstance> ANIM_Sec(TEXT("AnimBlueprint'/Game/Animations/Wolf2/Rifle/BP_Wolf2_Rifle.BP_Wolf2_Rifle_C'"));
	if (ANIM_Sec.Succeeded()) {
		SecondAnimIns = ANIM_Sec.Class;
	}

	static ConstructorHelpers::FClassFinder<UAnimInstance> ANIM_THIRD(TEXT("AnimBlueprint'/Game/Animations/Wolf3/Rifle/BP_Wolf3_Rifle.BP_Wolf3_Rifle_C'"));
	if (ANIM_THIRD.Succeeded()) {
		ThirdAnimIns = ANIM_THIRD.Class;
	}

	EvolEffect = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("EVOLEFFECT"));
	EvolEffect->SetupAttachment(RootComponent);
	static ConstructorHelpers::FObjectFinder<UParticleSystem> P_EVOL(TEXT("ParticleSystem'/Game/AdvancedMagicFX13/Particles/P_ky_gaither.P_ky_gaither'"));
	if (P_EVOL.Succeeded())
	{
		EvolEffect->SetTemplate(P_EVOL.Object);
		EvolEffect->bAutoActivate = false;
	}

	GetMesh()->SetRelativeScale3D(FVector(0.7f, 0.7f, 0.7f));
	CameraBoom->SocketOffset = FVector(240.0f, 40.0f, 50.0f);
	GetMesh()->SetRelativeScale3D(FVector(0.7f));

	GetCapsuleComponent()->InitCapsuleSize(25.0f, 30.0f);
	GetMesh()->SetRelativeLocation(FVector(0.0f, 0.0f, -33.0f));
	PlayerType = E_PlayerSelect::E_RF;
	CharacterStat->SetCharacterID((int)PlayerType);

	isFullAim = false;

	//StartTime = FMath::RandRange(8, 11);
}

void AAIRifleCharacter::Aim()
{
	switch (AimState) {
	case AIM::IDLE:

		AimState = AIM::HALF_AIM;
		//AimState = AIM::AIM;
		isFullAim = true;
		GetWorld()->GetTimerManager().SetTimer(AimTimerHendle, this, &AAIRifleCharacter::AimTimerFunc, 0.1f, true, 0.1f);
		break;

	case AIM::HALF_AIM:
		AimState = AIM::HALF_AIM;
		break;

	case AIM::AIM:
		break;
	}
}

void AAIRifleCharacter::AimRelease()
{
	switch (AimState) {
	case AIM::IDLE:

		if (isFullAim)
		{
			AimState = AIM::AIM;
		}
		else
		{
			AimState = AIM::IDLE;
		}

		break;

	case AIM::HALF_AIM:

		if (isFullAim)
		{
			AimState = AIM::AIM;
		}
		else
		{
			AimState = AIM::IDLE;
		}

		break;

	case AIM::AIM:

		AimState = AIM::IDLE;
		break;
	}
}

void AAIRifleCharacter::AimTimerFunc()
{
	isFullAim = false;
	GetWorldTimerManager().ClearTimer(AimTimerHendle);
}

void AAIRifleCharacter::Evolution()
{
	CharacterStat->CurHP = CharacterStat->GetCharacterStatData()->HP;

	if (CurLevel > 5)
		return;
	EvolEffect->SetActive(true);
	switch (CurLevel)
	{
	case 0:
		GetMesh()->SetRelativeScale3D(FVector(1.0f));
		CameraBoom->SocketOffset = FVector(190.0f, 45.0f, 80.0f);
		GetCapsuleComponent()->SetCapsuleSize(25.0f, 45.0f);
		GetMesh()->SetRelativeLocation(FVector(0, 0, -48.0f));
		break;
	case 1:
		GetMesh()->SetRelativeScale3D(FVector(1.2f));
		CameraBoom->SocketOffset = FVector(160.0f, 50.0f, 80.0f);
		GetCapsuleComponent()->SetCapsuleSize(25.0f, 60.0f);
		GetMesh()->SetRelativeLocation(FVector(0, 0, -63.0f));
		break;
	case 2:
		GetMesh()->SetRelativeScale3D(FVector(1.5f));
		CameraBoom->SocketOffset = FVector(140.0f, 60.0f, 110.0f);

		GetMesh()->SetSkeletalMesh(SecondSkMesh);
		GetMesh()->SetAnimInstanceClass(SecondAnimIns);
		APAnim = Cast<UPlayerCharacterAnimInstance>(GetMesh()->GetAnimInstance());

		GetCapsuleComponent()->SetCapsuleSize(45.0f, 90.0f);
		GetMesh()->SetRelativeLocation(FVector(0, 0, -93.0f));
		DetectedRange = 5000.0f;
		Gun->SetAiSpread(6.0f);
		ChangeWeapon();

		break;
	case 3:
		GetMesh()->SetRelativeScale3D(FVector(1.6f));
		CameraBoom->SocketOffset = FVector(140.0f, 65.0f, 110.0f);
		GetCapsuleComponent()->SetCapsuleSize(45.0f, 95.0f);
		GetMesh()->SetRelativeLocation(FVector(0, 0, -98.0f));
		break;
	case 4:
		GetMesh()->SetRelativeScale3D(FVector(1.7f));
		CameraBoom->SocketOffset = FVector(140.0f, 75.0f, 110.0f);
		GetCapsuleComponent()->SetCapsuleSize(45.0f, 103.0f);
		GetMesh()->SetRelativeLocation(FVector(0, 0, -100.0f));
		break;
	case 5:
		GetMesh()->SetRelativeScale3D(FVector(1.0f));
		CameraBoom->SocketOffset = FVector(100.0f, 90.0f, 130.0f);
		GetMesh()->SetSkeletalMesh(ThirdMesh);
		GetMesh()->SetAnimInstanceClass(ThirdAnimIns);
		APAnim = Cast<UPlayerCharacterAnimInstance>(GetMesh()->GetAnimInstance());
		GetMesh()->SetRelativeScale3D(FVector(0.15f));

		GetCapsuleComponent()->SetCapsuleSize(45.0f, 105.0f);
		GetMesh()->SetRelativeLocation(FVector(0, 0, -100.0f));

		//APAnim->ShotMontageCurrent = APAnim->ShotMontageThird;
		break;
	default:
		break;
	}
	++CurLevel;
	CharacterStat->SetCharacterID(CurLevel + (int)PlayerType);
	CharacterStat->SetWeaponeID(CurLevel + (int)PlayerType);
	CharacterStat->CurHP = CharacterStat->GetCharacterStatData()->HP;
	CharacterStat->CurAP = CharacterStat->GetCharacterStatData()->AP;
}