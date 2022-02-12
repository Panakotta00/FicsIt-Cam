#pragma once

typedef int64 FICFrame;
typedef float FICFrameFloat;

struct FFICFrameRange {
	FICFrame Begin;
	FICFrame End;

	FFICFrameRange() : Begin(0), End(1) {}
	
	FFICFrameRange(FICFrame InBegin, FICFrame InEnd) {
		SetRange(InBegin, InEnd);
	}

	void SetRange(FICFrame InBegin, FICFrame InEnd) {
		Begin = FMath::Min(InBegin, InEnd);
		End = FMath::Max(InBegin, InEnd);
		if (Begin == End) {
			End += 1;
		}
	}

	void GetRange(FICFrame& OutBegin, FICFrame& OutEnd) {
		OutBegin = Begin;
		OutEnd = End;
	}

	FICFrame Length() {
		return FMath::Abs(End - Begin);
	}

	bool IsInRange(FICFrame Frame) {
		return Begin <= Frame && Frame <= End;
	}
};

typedef float FICValue;

struct FFICValueRange {
	FICValue Begin;
	FICValue End;

	void SetRange(FICValue InBegin, FICValue InEnd) {
		Begin = FMath::Min(InBegin, InEnd);
		End = FMath::Max(InBegin, InEnd);
		if (Begin == End) {
			End += 1;
		}
	}

	void GetRange(FICValue& OutBegin, FICValue& OutEnd) {
		OutBegin = Begin;
		OutEnd = End;
	}

	FICValue Length() {
		return FMath::Abs(End - Begin);
	}
};

struct FFICValueTime {
	FICFrame Frame;
	FICValue Value;

	FFICValueTime() = default;
	FFICValueTime(FICFrame InFrame, FICValue InValue) : Frame(InFrame), Value(InValue) {}
};

struct FFICValueTimeFloat {
	FICFrameFloat Frame;
	FICValue Value;

	FFICValueTimeFloat() = default;
	FFICValueTimeFloat(FICFrameFloat InFrame, FICValue InValue) : Frame(InFrame), Value(InValue) {}
	
	void Get(FICFrameFloat& OutFrame, FICValue& OutValue) {
		OutFrame = Frame;
		OutValue = Value;
	}
};