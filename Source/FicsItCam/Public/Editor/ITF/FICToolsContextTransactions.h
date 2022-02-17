#pragma once

#include "ToolContextInterfaces.h"

class FFICToolsContextTransactions : public IToolsContextTransactionsAPI {
	bool bInTransaction = false;

	virtual void DisplayMessage(const FText& Message, EToolMessageLevel Level) override {
		UE_LOG(LogTemp, Warning, TEXT("[ToolMessage] %s"), *Message.ToString());
	}

	virtual void PostInvalidation() override {}

	virtual void BeginUndoTransaction(const FText& Description) override {
		bInTransaction = true;
	}

	virtual void EndUndoTransaction() override {
		bInTransaction = false;
	}

	virtual void AppendChange(UObject* TargetObject, TUniquePtr<FToolCommandChange> Change, const FText& Description) override {
		bool bCloseTransaction = false;
		if (!bInTransaction)
		{
			BeginUndoTransaction(Description);
			bCloseTransaction = true;
		}

		if (bCloseTransaction)
		{
			EndUndoTransaction();
		}
	}

	virtual bool RequestSelectionChange(const FSelectedOjectsChangeList& SelectionChange) override {
		return false;
	}
};