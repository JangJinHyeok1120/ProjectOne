// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "ProjectOne.h"
#include "Animation/PlayerCharacterAnimInstance.h"
#include "GameFramework/Character.h"
#include "Environments/ShaderInfo.h"
#include "Kismet/KismetMathLibrary.h"
#include "ProjectOneCharacter.generated.h"


UCLASS(config=Game)
class AProjectOneCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
protected:
	enum class RollDir {
		//FRONT,
		//BACK,
		RELEASE,
		LEFT,
		RIGHT
	};
	
	enum class Item 
	{
		EMPTY,
		EVOLUTION,
		HP
	};

	enum class AIM
	{
		IDLE,
		HALF_AIM,
		AIM
	};

public://public �ڿ�
	AProjectOneCharacter(); 

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseTurnRate;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseLookUpRate;

	UPROPERTY(VisibleAnywhere)
	class UPlayerCharacterAnimInstance* APAnim;

	//components
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
	class USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
	class UCameraComponent* FollowCamera;

	UPROPERTY(VisibleAnywhere, Category = Effect)
	UParticleSystemComponent* SheildEffect;

	UPROPERTY(VisibleAnywhere, Category = Effect)
	UParticleSystemComponent* HealEffect;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Inventory)
	class UInventoryComponent* Inventory;

	UPROPERTY(BlueprintReadWrite, Category = UI)
	class UWidgetComponent* HPBarWidgetComponent;

	UPROPERTY(BlueprintReadOnly, Category = Stat)
	class UPlayerStatComponent* CharacterStat;

	UPROPERTY(BlueprintReadOnly, Category = Sound)
	class UPOPlayerAkComponent* SoundManager;

	UPROPERTY(BlueprintReadOnly)
	class USuperGun* Gun;

	//EvolutionData
	UPROPERTY(VisibleAnywhere)
	USkeletalMesh* FirstMesh;

	UPROPERTY(VisibleAnywhere)
	USkeletalMesh* SecondSkMesh;

	UPROPERTY(VisibleAnywhere)
	USkeletalMesh* ThirdMesh;

	UPROPERTY(VisibleAnywhere)
	UClass* SecondAnimIns;

	UPROPERTY(VisibleAnywhere)
	UClass* ThirdAnimIns;

	//HitUI��
	UPROPERTY(BlueprintReadOnly)
	AActor * LastHitByChracter;

	UPROPERTY(VisibleAnywhere)
	E_PlayerSelect PlayerType;

	UPROPERTY(BlueprintReadWrite)
	int KillCount = 0;

	UPROPERTY(BlueprintReadWrite)
	bool isHit;

	UPROPERTY(BlueprintReadWrite)
	bool AteHealItem;

	UPROPERTY(BlueprintReadWrite)
	bool ControlAllowed;

	UPROPERTY(BlueprintReadWrite)
	bool isKillOtherPlayer;

	UPROPERTY(BlueprintReadOnly)
	bool isAim;

	UPROPERTY(BlueprintReadOnly)
	bool isPause;

	UPROPERTY(BlueprintReadOnly)
	bool IsAlive;

	UPROPERTY(BlueprintReadOnly)
	int Rank = 1;

	bool ShootInput;
	bool isShooting;
	int DeadTime;

	int32 CurLevel;

	FTimerHandle DeadTimer;
	
	AIM AimState = AIM::IDLE;

protected: // protected �Լ� ����
	
	// ������, �ʱ�ȭ
	virtual void SetResources(); // ���ҽ� �ε� (�ʱ�ȭ)
	virtual void SetComponents();

	
	// ���� �Լ�
	void MoveForward(float Value);
	void MoveRight(float Value);
	void TurnAtRate(float Rate);
	void LookUpAtRate(float Rate);

	// �׼� �Լ�
	void Shot();
	void ShotRelease();
	void Roll();
	void MoveReleased();
	virtual void Aim();
	virtual void AimRelease();
	void ReloadAnimPlay();
	void UseItem1();
	void UseItem2();
	void UseItem3();
	void CameraRotateInput();
	void CameraRotateRelease();

	// ȭ��FX
	void PlayCShake(int Index);
	virtual void Shooting(float tick);

public:
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	UFUNCTION(BlueprintCallable)
	int32 GetEvolutionLevel() { return CurLevel; }

	UFUNCTION(BlueprintCallable)
	void Win();

	UFUNCTION(BlueprintCallable)
	void Lose();

	UFUNCTION(BlueprintCallable)
	void Hit(float Damage, bool isHead, AActor * Causer);

	float CalcDamage(float Damage, bool isHead);

	void Dead();
	void ReLoad();
	virtual void Evolution();
	virtual void ChangeWeapon() {}

protected://protected
	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void PostInitializeComponents() override;
	virtual void Tick(float delta) override;
	virtual void BeginPlay() override;
	virtual void PossessedBy(AController * ct) override;
	// End of APawn interface
protected:

	UPROPERTY(VisibleAnywhere)
	TArray<TSubclassOf<UCameraShake>> CShakeList;

	UPROPERTY(VisibleAnywhere, Category = Effect)
	UParticleSystemComponent* EvolEffect;

	const UWorld* const World = GetWorld(); 

	APlayerController* PlayerController;

	RollDir rollDir;
	FVector InputVector;
	FVector ScratchNormal;
	float intervalTime = 0.0f;


};

