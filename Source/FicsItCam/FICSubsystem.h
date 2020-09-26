#pragma once

#include "FGSubsystem.h"
#include "FICAnimation.h"
#include "FGSaveInterface.h"
#include "FICCamera.h"
#include "UI/FICEditorContext.h"


#include "FICSubsystem.generated.h"

UCLASS()
class AFICSubsystem : public AFGSubsystem, public IFGSaveInterface {
	GENERATED_BODY()
private:
	UPROPERTY()
	UInputComponent* Input = nullptr;

	UPROPERTY()
	AFICCamera* Camera = nullptr;

	UPROPERTY()
	UFICAnimation* ActiveAnimation = nullptr;

	UPROPERTY()
	UFICEditorContext* EditorContext = nullptr;
	
public:
	UPROPERTY(BlueprintReadWrite, Category="FicsIt-Cam")
	TMap<FString, UFICAnimation*> StoredAnimations;

	UPROPERTY()
	TSet<UFICAnimation*> VisibleAnimations;

	AFICSubsystem();
	
	// Begin AActor
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	// End AActor
	
	// Begin IFGSaveInterface
	virtual bool ShouldSave_Implementation() const override;
	// End IFGSaveInterface

	UFUNCTION(BlueprintCallable, Category="FicsIt-Cam", meta=(WorldContext = "WorldContextObject"))
	static AFICSubsystem* GetFICSubsystem(UObject* WorldContext);

	UFUNCTION(BlueprintCallable, Category="FicsIt-Cam")
	void PlayAnimation(UFICAnimation* Path);
	
	UFUNCTION(BlueprintCallable, Category="FicsIt-Cam")
	void StopAnimation();
	
	void AddVisibleAnimation(UFICAnimation* Path);

	UFUNCTION()
	void CreateKeypointPressed();

	void SetActiveAnimation(UFICAnimation* ActiveAnimation);
	UFICAnimation* GetActiveAnimation() const;
};
