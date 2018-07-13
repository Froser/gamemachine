#include "stdafx.h"
#include "gmtransaction.h"

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

GMTransactionManager::GMTransactionManager()
{
	D(d);
	// 插入一个空白标记，表示链表的头
	d->transactions.push_back(GMOwnedPtr<GMTransaction>(TransactionBeginTag));
	d->runningTransaction = d->transactions.cbegin();
}

GMTransactionManager::~GMTransactionManager()
{
	D(d);
	GM_delete(d->currentTransaction);
}

void GMTransactionManager::beginTransaction()
{
	D(d);
	if (d->nest == 0)
	{
		d->currentTransaction = new GMTransaction();
		++d->nest;
	}
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
		gm_error("GMTransactionManager::commitTransaction: You need call endTransaction before commit.");
		return false;
	}

	// 如果当前事务不是第一个，且不是指向最后一个，那么要干掉它后面所有的事务
	if (d->runningTransaction->get() != TransactionBeginTag && d->runningTransaction != --d->transactions.end())
	{
		auto iter = d->runningTransaction;
		++iter;
		d->transactions.erase(iter, d->transactions.end());
	}

	d->transactions.push_back(GMOwnedPtr<GMTransaction>(d->currentTransaction));
	d->runningTransaction = --d->transactions.cend();
	d->currentTransaction = nullptr;
	return true;
}

void GMTransactionManager::abortTransaction()
{
	D(d);
	d->currentTransaction->clear();
}

void GMTransactionManager::addAtom(ITransactionAtom* atom)
{
	D(d);
	// 如果外部没有调用beginTransaction，则不生效
	if (d->nest <= 0)
		return;

	d->currentTransaction->addAtom(atom);
}

bool GMTransactionManager::canUndo()
{
	D(d);
	return !d->transactions.empty() && d->runningTransaction->get() != TransactionBeginTag;
}

void GMTransactionManager::undo()
{
	D(d);
	if (canUndo())
	{
		(*d->runningTransaction)->unexecute();
		--d->runningTransaction;
	}
}

bool GMTransactionManager::canRedo()
{
	D(d);
	return !d->transactions.empty() && d->runningTransaction != --d->transactions.end();
}

void GMTransactionManager::redo()
{
	D(d);
	if (canRedo())
	{
		++d->runningTransaction;
		(*d->runningTransaction)->execute();
	}
}