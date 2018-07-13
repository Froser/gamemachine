#ifndef __GMTRANSACTION_H__
#define __GMTRANSACTION_H__
#include <gmcommon.h>
BEGIN_NS

GM_INTERFACE(ITransactionAtom)
{
	virtual void execute() = 0;
	virtual void unexecute() = 0;
};

GM_PRIVATE_OBJECT(GMTransaction)
{
	List<GMOwnedPtr<ITransactionAtom>> atoms;
};

class GMTransaction : public GMObject
{
	GM_DECLARE_PRIVATE(GMTransaction)

public:
	GMTransaction() = default;

public:
	void clear();
	void addAtom(AUTORELEASE ITransactionAtom* atom);
	void execute();
	void unexecute();
};

GM_PRIVATE_OBJECT(GMTransactionManager)
{
	GMint nest = 0;
	List<GMOwnedPtr<GMTransaction>> transactions;
	GMTransaction* currentTransaction = nullptr;
	List<GMOwnedPtr<GMTransaction>>::const_iterator runningTransaction;
};

class GMTransactionManager : public GMObject
{
	GM_DECLARE_PRIVATE(GMTransactionManager)

public:
	GMTransactionManager();
	~GMTransactionManager();

public:
	virtual void beginTransaction();
	virtual void endTransaction();
	virtual bool commitTransaction();
	virtual void abortTransaction();
	virtual void addAtom(AUTORELEASE ITransactionAtom* atom);

public:
	bool canUndo();
	void undo();
	bool canRedo();
	void redo();
};

END_NS
#endif