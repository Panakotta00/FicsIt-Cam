#pragma once

typedef int64 FICFrame;
typedef float FICFrameFloat;

struct FFICFrameRangeIterator {
	FICFrame Frame;

	FFICFrameRangeIterator(FICFrame Frame) : Frame(Frame) {}

	FICFrame operator*() { return Frame; }
	
	FFICFrameRangeIterator& operator++() {
		Frame++;
		return *this;
	}
	FFICFrameRangeIterator operator++(int) {
		FFICFrameRangeIterator tmp = *this;
		++(*this);
		return tmp;
	}

	bool operator==(FFICFrameRangeIterator& Other) {
		return Frame == Other.Frame;
	}
	bool operator!=(FFICFrameRangeIterator& Other) {
		return Frame != Other.Frame;
	}
};

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

	FICFrame Length() const {
		return FMath::Abs(End - Begin);
	}

	bool IsInRange(FICFrame Frame) const {
		return Begin <= Frame && Frame <= End;
	}

	bool operator==(const FFICFrameRange& Other) const {
		return Begin == Other.Begin && End == Other.End;
	}

	bool operator!=(const FFICFrameRange& Other) const {
		return !(*this == Other);
	}

	FFICFrameRange operator+(FICFrame Frame) const {
		return FFICFrameRange(Begin + Frame, End + Frame);
	}

	FFICFrameRange operator-(FICFrame Frame) const {
		return FFICFrameRange(Begin - Frame, End - Frame);
	}

	FFICFrameRangeIterator begin() const {
		return FFICFrameRangeIterator(Begin);
	}
	FFICFrameRangeIterator end() const {
		return FFICFrameRangeIterator(End);
	}
};

typedef float FICValue;

struct FFICValueRange {
	FICValue Begin;
	FICValue End;

	FFICValueRange() : Begin(-1), End(1) {} 
	FFICValueRange(FICValue InBegin, FICValue InEnd) {
		SetRange(InBegin, InEnd);
	}

	void SetRange(FICValue InBegin, FICValue InEnd) {
		Begin = FMath::Min(InBegin, InEnd);
		End = FMath::Max(InBegin, InEnd);
		if (Begin == End) {
			End += 1;
		}
	}

	void GetRange(FICValue& OutBegin, FICValue& OutEnd) const {
		OutBegin = Begin;
		OutEnd = End;
	}

	FICValue Length() const {
		return FMath::Abs(End - Begin);
	}

	FFICValueRange operator+(FICValue Value) const {
		return FFICValueRange(Begin + Value, End + Value);
	}

	FFICValueRange operator-(FICValue Value) const {
		return FFICValueRange(Begin - Value, End - Value);
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
	
	void Get(FICFrameFloat& OutFrame, FICValue& OutValue) const {
		OutFrame = Frame;
		OutValue = Value;
	}
};