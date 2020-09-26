#include "FICCommand.h"

#include "FICSubsystem.h"
#include "FICUtils.h"

AFICCommand::AFICCommand() {
	bOnlyUsableByPlayer = true;
	MinNumberOfArguments = 1;
	ModId = "FicsItCam";
	CommandName = "fic";
}

#pragma optimize("", off)
EExecutionStatus AFICCommand::ExecuteCommand_Implementation(UCommandSender* Sender, const TArray<FString>& Arguments, const FString& Label) {
	AFICSubsystem* SubSys = AFICSubsystem::GetFICSubsystem(this);
	UFICAnimation* Anim = SubSys->GetActiveAnimation();
	
	if (Arguments[0] == "list") {
		for (const TPair<FString, UFICAnimation*>& Entry : SubSys->StoredAnimations) {
			Sender->SendChatMessage(Entry.Key);
		}
		return EExecutionStatus::COMPLETED;
	}
	if (Arguments[0] == "create") {
		if (Arguments.Num() < 2) return EExecutionStatus::BAD_ARGUMENTS;
		if (SubSys->StoredAnimations.Contains(Arguments[1])) {
			Sender->SendChatMessage("Animation '" + Arguments[1] + "' already exists.");
		} else {
			SubSys->StoredAnimations.Add(Arguments[1], NewObject<UFICAnimation>(this));
			Sender->SendChatMessage("Animation '" + Arguments[1] + "' created.");
		}
		return EExecutionStatus::COMPLETED;
	}
	if (Arguments[0] == "delete") {
		if (Arguments.Num() < 2) return EExecutionStatus::BAD_ARGUMENTS;
		if (SubSys->StoredAnimations.Remove(Arguments[1])) {
			Sender->SendChatMessage("Animation '" + Arguments[1] + "' deleted.");
			return EExecutionStatus::COMPLETED;
		} else {
			Sender->SendChatMessage("Animation '" + Arguments[1] + "' not found.");
			return EExecutionStatus::BAD_ARGUMENTS;
		}
	}
	if (Arguments[0] == "use") {
		if (Arguments.Num() < 2) return EExecutionStatus::BAD_ARGUMENTS;
		UFICAnimation** FoundAnimation = SubSys->StoredAnimations.Find(Arguments[1]);
		if (!FoundAnimation) {
			Sender->SendChatMessage("Animation '" + Arguments[1] + "' not found.");
			return EExecutionStatus::BAD_ARGUMENTS;
		} else {
			SubSys->SetActiveAnimation(*FoundAnimation);
			Sender->SendChatMessage("Using now Animation '" + Arguments[1] + "'.");
			return EExecutionStatus::COMPLETED;
		}
	}
	if (Arguments[0] == "add") {
		if (Arguments.Num() < 2) return EExecutionStatus::BAD_ARGUMENTS;
		if (!Arguments[1].IsNumeric()) {
			Sender->SendChatMessage("No valid frame-position given.");
			return EExecutionStatus::BAD_ARGUMENTS;
		}
		if (!Anim) {
			Sender->SendChatMessage("No Animation is selected as to use.");
			return EExecutionStatus::UNCOMPLETED;
		}
		int64 Time = FCString::Atoi64(*Arguments[1]);
		FVector Pos = Sender->GetPlayer()->GetViewTarget()->GetActorLocation();
		SubSys->GetActiveAnimation()->PosX.SetKeyframe(Time, FFICFloatKeyframe(Pos.X));
		Anim->PosY.SetKeyframe(Time, FFICFloatKeyframe(Pos.Y));
		Anim->PosZ.SetKeyframe(Time, FFICFloatKeyframe(Pos.Z));
		FRotator Rot = Sender->GetPlayer()->GetViewTarget()->GetActorRotation();
		float RotPitch = Anim->RotPitch.GetValue(Time);
		float RotYaw = Anim->RotYaw.GetValue(Time);
		float RotRoll = Anim->RotRoll.GetValue(Time);
		FVector NewRot = FRotator(RotPitch, RotYaw, RotRoll).Vector();
		Rot = FQuat::FindBetween(Rot.Vector(), NewRot).Rotator();
		Anim->RotPitch.SetKeyframe(Time, FFICFloatKeyframe(RotPitch - Rot.Pitch));
		Anim->RotYaw.SetKeyframe(Time, FFICFloatKeyframe(RotYaw - Rot.Yaw));
 		Anim->RotRoll.SetKeyframe(Time, FFICFloatKeyframe(RotRoll - Rot.Roll));
		if (Arguments.Num() > 2) {
			if (!Arguments[2].IsNumeric()) {
				Sender->SendChatMessage("No valid FOV given.");
				return EExecutionStatus::BAD_ARGUMENTS;
			}
			Anim->FOV.SetKeyframe(Time, FFICFloatKeyframe(FCString::Atof(*Arguments[2])));
		}
		Anim->RecalculateAllKeyframes();
		Sender->SendChatMessage("Keyframe added at frame-position '" + FString::FromInt(Time) + "'.");
		return EExecutionStatus::COMPLETED;
	}
	if (Arguments[0] == "play") {
		if (Arguments.Num() < 2) return EExecutionStatus::BAD_ARGUMENTS;
		UFICAnimation** FoundAnimation = SubSys->StoredAnimations.Find(Arguments[1]);
		if (!FoundAnimation) {
			Sender->SendChatMessage("Animation '" + Arguments[1] + "' not found.");
			return EExecutionStatus::BAD_ARGUMENTS;
		}
		SubSys->PlayAnimation(*FoundAnimation);
		Sender->SendChatMessage("Playing Animation '" + Arguments[1] + "'.");
	}
	if (Arguments[0] == "show") {
		if (Arguments.Num() < 2) return EExecutionStatus::BAD_ARGUMENTS;
		UFICAnimation** FoundAnimation = SubSys->StoredAnimations.Find(Arguments[1]);
		if (!FoundAnimation) {
			Sender->SendChatMessage("Animation '" + Arguments[1] + "' not found.");
			return EExecutionStatus::BAD_ARGUMENTS;
		}
		SubSys->AddVisibleAnimation(*FoundAnimation);
	}
	return EExecutionStatus::BAD_ARGUMENTS;
}
#pragma optimize("", on)