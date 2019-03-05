#pragma once

#include <Common/RWLock.h>

namespace DB
{

/** Does not allow changing the table description (including rename and delete the table).
  * If during any operation the table structure should remain unchanged, you need to hold such a lock for all of its time.
  * For example, you need to hold such a lock for the duration of the entire SELECT or INSERT query and for the whole time the merge of the set of parts
  *  (but between the selection of parts for the merge and their merging, the table structure can change).
  * NOTE: This is a lock to "read" the table's description. To change the table description, you need to take the TableStructureWriteLock.
  */
struct TableStructureLockHolder
{
    /// Order is important.
    RWLockImpl::LockHolder alter_intention_lock;
    RWLockImpl::LockHolder insert_structure_lock;
    RWLockImpl::LockHolder select_structure_lock;

    void release()
    {
        select_structure_lock.reset();
        insert_structure_lock.reset();
        alter_intention_lock.reset();
    }
};

}
