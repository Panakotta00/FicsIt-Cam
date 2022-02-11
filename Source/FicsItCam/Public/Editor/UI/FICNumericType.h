#pragma once
#include "Widgets/Input/NumericTypeInterface.h"
/*
template<typename NumericType>
struct TFICNumericTypeInterface : INumericTypeInterface<NumericType> {
	bool Degree = false;
	
	virtual FString ToString(const NumericType& Value) const override {
		const FNumberFormattingOptions NumberFormattingOptions = FNumberFormattingOptions()
            .SetUseGrouping(false)
            .SetMinimumFractionalDigits(TIsIntegral<NumericType>::Value ? 0 : 1)
            .SetMaximumFractionalDigits(TIsIntegral<NumericType>::Value ? 0 : 7);
		return FastDecimalFormat::NumberToString(Value, ExpressionParser::GetLocalizedNumberFormattingRules(), NumberFormattingOptions);
	}
	
	virtual TOptional<NumericType> FromString(const FString& InString, const NumericType& InExistingValue) override {
		FBasicMathExpressionEvaluator Parser;
		TValueOrError<double, FExpressionError> Result = Parser.Evaluate(*InString, double(InExistingValue));
		if (Result.IsValid()) return NumericType(Result.GetValue());

		return TOptional<NumericType>();
	}

	virtual bool IsCharacterValid(TCHAR InChar) const override {
		const FDecimalNumberFormattingRules& NumberFormattingRules = ExpressionParser::GetLocalizedNumberFormattingRules();
		const FString ValidChars = TEXT("1234567890()-+=\\/.,*^%%");
		return InChar != 0 && (ValidChars.GetCharArray().Contains(InChar) || InChar == NumberFormattingRules.GroupingSeparatorCharacter || InChar == NumberFormattingRules.DecimalSeparatorCharacter || Algo::Find(NumberFormattingRules.DigitCharacters, InChar) != 0);
	}
};
*/