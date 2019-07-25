#ifndef __GMTRANSACTION_H__
#define __GMTRANSACTION_H__
#include <gmcommon.h>
BEGIN_NS

GM_INTERFACE(ITransactionAtom)
{
	virtual void execute() = 0;
	virtual void unexecute() = 0;
};

GM_PRIVATE_CLASS(GMTransaction);
class GMTransaction
{
	GM_DECLARE_PRIVATE(GMTransaction)

public:
	GMTransaction();
	~GMTransaction();

public:
	void clear();
	void addAtom(AUTORELEASE ITransactionAtom* atom);
	bool removeAtom(ITransactionAtom* atom);
	void execute();
	void unexecute();
	bool isEmpty();
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

GM_PRIVATE_CLASS(GMTransactionManager);
class GMTransactionManager
{
	GM_DECLARE_PRIVATE(GMTransactionManager)
	GM_FRIEND_CLASS(GMScopeTransaction);
	GM_FRIEND_CLASS(GMWeakTransaction);

public:
	~GMTransactionManager();

private:
	GMTransactionManager();

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
	GMScopeTransaction(GMTransactionContext* context = nullptr);
	~GMScopeTransaction();

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