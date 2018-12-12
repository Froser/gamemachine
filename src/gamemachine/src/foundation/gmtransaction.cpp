#include "stdafx.h"
#include "gmtransaction.h"
#include "gmthread.h"

constexpr GMTransaction* TransactionBeginTag = nullptr;

void GMTransaction::clear()
{
	D(d);
	d->atoms.clear(); // 不一定需要清空capacity，因为它很常用
}

void GMTransaction::addAtom(AUTORELEASE ITransactionAtom* atom)
{
	D(d);
	d->atoms.push_back(GMOwnedPtr<ITransactionAtom>(atom));
}

bool GMTransaction::removeAtom(ITransactionAtom* atom)
{
	D(d);
	for (auto iter = d->atoms.cbegin(); iter != d->atoms.cend(); ++iter)
	{
		if ((*iter).get() == atom)
		{
			iter = d->atoms.erase(iter);
			return true;
		}
	}
	return false;
}

void GMTransaction::execute()
{
	D(d);
	// 依次执行原子事务 (从前往后)
	for (auto iter = d->atoms.begin(); iter != d->atoms.end(); ++iter)
	{
		(*iter)->execute();
	}
}

void GMTransaction::unexecute()
{
	D(d);
	// 依次撤回原子事务 (从后往前)
	for (auto iter = d->atoms.rbegin(); iter != d->atoms.rend(); ++iter)
	{
		(*iter)->unexecute();
	}
}

void GMTransactionManager::beginTransaction(GMTransactionContext* transactionContext)
{
	D(d);
	if (d->nest == 0)
	{
		if (!transactionContext)
		{
			// 没有指定事务上下文，可能是因为外层默认不需要开启事务
			++d->nest;
			return;
		}

		d->transactionContext = transactionContext;

		// 如果是一个全新的事务，插入一个空白标记，表示链表的头
		if (d->transactionContext && d->transactionContext->transactions.empty())
		{
			d->transactionContext->transactions.push_back(GMOwnedPtr<GMTransaction>(TransactionBeginTag));
			d->transactionContext->runningTransaction = d->transactionContext->transactions.cbegin();
		}

		d->transactionContext->currentTransaction = new GMTransaction();
	}
	++d->nest;
}

void GMTransactionManager::endTransaction()
{
	D(d);
	GM_ASSERT(d->nest > 0);
	--d->nest;
	// 还是容一下错
	if (d->nest < 0)
		d->nest = 0;
}

bool GMTransactionManager::commitTransaction()
{
	D(d);
	if (d->nest != 0)
	{
		// 当前在嵌套层，则不提交事务
		return false;
	}

	// 外部没有传入上下文，不提交
	if (!d->transactionContext)
		return false;

	if (d->transactionContext->currentTransaction->isEmpty())
		return true;

	// 如果当前事务不是指向最后一个，那么要干掉它后面所有的事务
	if (d->transactionContext->runningTransaction != --d->transactionContext->transactions.end())
	{
		auto iter = d->transactionContext->runningTransaction;
		++iter;
		d->transactionContext->transactions.erase(iter, d->transactionContext->transactions.end());
	}

	d->transactionContext->transactions.push_back(GMOwnedPtr<GMTransaction>(d->transactionContext->currentTransaction));
	d->transactionContext->runningTransaction = --d->transactionContext->transactions.cend();
	d->transactionContext->currentTransaction = nullptr;
	d->transactionContext = nullptr;
	return true;
}

void GMTransactionManager::abortTransaction()
{
	D(d);
	d->transactionContext->currentTransaction->clear();
}

ITransactionAtom* GMTransactionManager::addAtom(ITransactionAtom* atom)
{
	D(d);
	// 如果外部没有调用beginTransaction，则不生效
	if (d->nest <= 0)
		return nullptr;

	// 外部没有传入上下文，不生效
	if (!d->transactionContext)
		return nullptr;

	d->transactionContext->currentTransaction->addAtom(atom);
	return atom;
}

bool GMTransactionManager::removeAtom(ITransactionAtom* atom)
{
	D(d);
	// 如果外部没有调用beginTransaction，则不生效
	if (d->nest <= 0)
		return false;

	// 外部没有传入上下文，不生效
	if (!d->transactionContext)
		return false;

	return d->transactionContext->currentTransaction->removeAtom(atom);
}

bool GMTransactionManager::canUndo()
{
	D(d);
	return !d->transactionContext->transactions.empty() && d->transactionContext->runningTransaction->get() != TransactionBeginTag;
}

void GMTransactionManager::undo()
{
	D(d);
	if (canUndo())
	{
		(*d->transactionContext->runningTransaction)->unexecute();
		--d->transactionContext->runningTransaction;
	}
}

bool GMTransactionManager::canRedo()
{
	D(d);
	return !d->transactionContext->transactions.empty() && d->transactionContext->runningTransaction != --d->transactionContext->transactions.end();
}

void GMTransactionManager::redo()
{
	D(d);
	if (canRedo())
	{
		++d->transactionContext->runningTransaction;
		(*d->transactionContext->runningTransaction)->execute();
	}
}

GMTransactionManager& GMTransactionManager::getTransactionManager()
{
	static HashMap<GMThreadId, GMOwnedPtr<GMTransactionManager>> s_managers;
	auto tid = GMThread::getCurrentThreadId();
	auto iter = s_managers.find(GMThread::getCurrentThreadId());
	if (iter == s_managers.end())
	{
		GMTransactionManager* mgr = new GMTransactionManager();
		s_managers[tid] = GMOwnedPtr<GMTransactionManager>(mgr);
		return *mgr;
	}
	else
	{
		return *(iter->second.get());
	}
}

GMScopeTransaction::~GMScopeTransaction()
{
	m_mgr->endTransaction();
	m_mgr->commitTransaction();
}

GMScopeTransaction::GMScopeTransaction(GMTransactionContext* context /*= nullptr*/) : m_context(context)
, m_mgr(&GMTransactionManager::getTransactionManager())
{
	m_mgr->beginTransaction(m_context);
}
