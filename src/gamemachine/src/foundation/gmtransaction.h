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
	bool removeAtom(ITransactionAtom* atom);
	void execute();
	void unexecute();

public:
	bool isEmpty()
	{
		D(d);
		return d->atoms.empty();
	}
};

struct GMTransactionContext
{
	List<GMOwnedPtr<GMTransaction>> transactions;
	GMTransaction* currentTransaction = nullptr;
	List<GMOwnedPtr<GMTransaction>>::const_iterator runningTransaction;

	GMTransactionContext() = default;
	~GMTransactionContext() { GM_delete(currentTransaction); }
	GMTransactionContext(const GMTransactionContext&) = delete;
	GMTransactionContext& operator=(const GMTransactionContext&) = delete;
};

GM_PRIVATE_OBJECT(GMTransactionManager)
{
	GMint32 nest = 0;
	GMTransactionContext* transactionContext = nullptr;
};

class GMTransactionManager : public GMObject
{
	GM_DECLARE_PRIVATE(GMTransactionManager)
	friend class GMScopeTransaction;
	friend class GMWeakTransaction;

private:
	GMTransactionManager() = default;

public:
	void beginTransaction(GMTransactionContext* transactionContext);
	void endTransaction();
	bool commitTransaction();
	void abortTransaction();
	ITransactionAtom* addAtom(AUTORELEASE ITransactionAtom* atom);
	bool removeAtom(ITransactionAtom* atom);

public:
	bool canUndo();
	void undo();
	bool canRedo();
	void redo();

private:
	static GMTransactionManager& getTransactionManager();
};

class GMScopeTransaction
{
public:
	GMScopeTransaction(GMTransactionContext* context = nullptr)
		: m_context(context)
		, m_mgr(&GMTransactionManager::getTransactionManager())
	{
		m_mgr->beginTransaction(m_context);
	}

	~GMScopeTransaction()
	{
		m_mgr->endTransaction();
		m_mgr->commitTransaction();
	}

	inline GMTransactionManager* getManager() GM_NOEXCEPT
	{
		return m_mgr;
	}

private:
	GMTransactionContext* m_context;
	GMTransactionManager* m_mgr;
};

class GMWeakTransaction
{
public:
	GMWeakTransaction(GMTransactionContext* context = nullptr)
		: m_context(context)
		, m_mgr(&GMTransactionManager::getTransactionManager())
	{
		m_mgr->beginTransaction(m_context);
	}

	~GMWeakTransaction()
	{
		m_mgr->endTransaction();
		m_mgr->abortTransaction();
		m_mgr->commitTransaction();
	}

	inline GMTransactionManager* getManager() GM_NOEXCEPT
	{
		return m_mgr;
	}

private:
	GMTransactionContext* m_context;
	GMTransactionManager* m_mgr;
};

END_NS
#endif