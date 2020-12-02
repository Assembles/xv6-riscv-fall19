// Buffer cache.
//
// The buffer cache is a linked list of buf structures holding
// cached copies of disk block contents.  Caching disk blocks
// in memory reduces the number of disk reads and also provides
// a synchronization point for disk blocks used by multiple processes.
//
// Interface:
// * To get a buffer for a particular disk block, call bread.
// * After changing buffer data, call bwrite to write it to disk.
// * When done with the buffer, call brelse.
// * Do not use the buffer after calling brelse.
// * Only one process at a time can use a buffer,
//     so do not keep them longer than necessary.

#include "types.h"
#include "param.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "riscv.h"
#include "defs.h"
#include "fs.h"
#include "buf.h"

#define NBUCKETS 13

struct
{
  struct spinlock lock[NBUCKETS];
  struct buf buf[NBUF];

  // Linked list of all buffers, through prev/next.
  // head.next is most recently used.
  //struct buf head;
  struct buf hashbucket[NBUCKETS]; //每个哈希队列一个linked list及一个lock
} bcache;

void binit(void)
{
  struct buf *b;
  for (int i = 0; i < NBUCKETS; i++)
  {
    initlock(&bcache.lock[i], "bcache.bucket");
    b = &bcache.hashbucket[i];
    b->prev = b;
    b->next = b;
  }
  for (b = bcache.buf; b < bcache.buf + NBUF; b++)
  {
    b->next = bcache.hashbucket[0].next;
    b->prev = &bcache.hashbucket[0];
    initsleeplock(&b->lock, "buffer");
    bcache.hashbucket[0].next->prev = b;
    bcache.hashbucket[0].next = b;
  }
}

// Look through buffer cache for block on device dev.
// If not found, allocate a buffer.
// In either case, return locked buffer.
static struct buf *
bget(uint dev, uint blockno)
{
  struct buf *b;
  int cached_num = (blockno) % NBUCKETS;
  acquire(&bcache.lock[cached_num]);

  // Is the block already cached?
  for (b = bcache.hashbucket[cached_num].next; b != &bcache.hashbucket[cached_num]; b = b->next)
  {
    if (b->dev == dev && b->blockno == blockno)
    {
      b->refcnt++;
      release(&bcache.lock[cached_num]);
      acquiresleep(&b->lock);
      return b;
    }
  }
  int not_cached_num = (cached_num + 1) % NBUCKETS;
  // Not cached; recycle an unused buffer.
  for (; not_cached_num != cached_num; not_cached_num = (not_cached_num + 1) % NBUCKETS)
  {
    acquire(&bcache.lock[not_cached_num]);
    for (b = bcache.hashbucket[not_cached_num].prev; b != &bcache.hashbucket[not_cached_num]; b = b->prev)
    {
      if (b->refcnt == 0)
      {
        b->dev = dev;
        b->blockno = blockno;
        b->valid = 0;
        b->refcnt = 1;

        b->next->prev = b->prev;
        b->prev->next = b->next;
        release(&bcache.lock[not_cached_num]);

        b->next = bcache.hashbucket[cached_num].next;
        b->prev = &bcache.hashbucket[cached_num];
        bcache.hashbucket[cached_num].next->prev = b;
        bcache.hashbucket[cached_num].next = b;

        release(&bcache.lock[cached_num]);
        acquiresleep(&b->lock);
        return b;
      }
    }
    release(&bcache.lock[not_cached_num]);
  }
  panic("bget: no buffers");
}

// Return a locked buf with the contents of the indicated block.
struct buf *
bread(uint dev, uint blockno)
{
  struct buf *b;

  b = bget(dev, blockno);
  if (!b->valid)
  {
    virtio_disk_rw(b->dev, b, 0);
    b->valid = 1;
  }
  return b;
}

// Write b's contents to disk.  Must be locked.
void bwrite(struct buf *b)
{
  if (!holdingsleep(&b->lock))
    panic("bwrite");
  virtio_disk_rw(b->dev, b, 1);
}

// Release a locked buffer.
// Move to the head of the MRU list.
void brelse(struct buf *b)
{
  if (!holdingsleep(&b->lock))
    panic("brelse");

  releasesleep(&b->lock);
  int cache_num = (b->blockno) % NBUCKETS;
  acquire(&bcache.lock[cache_num]);
  b->refcnt--;
  if (b->refcnt == 0)
  {
    // no one is waiting for it.
    b->next->prev = b->prev;
    b->prev->next = b->next;
    b->next = bcache.hashbucket[cache_num].next;
    b->prev = &bcache.hashbucket[cache_num];
    bcache.hashbucket[cache_num].next->prev = b;
    bcache.hashbucket[cache_num].next = b;
  }

  release(&bcache.lock[cache_num]);
}

void bpin(struct buf *b)
{
  int cached_num = (b->blockno) % NBUCKETS;
  acquire(&bcache.lock[cached_num]);
  b->refcnt++;
  release(&bcache.lock[cached_num]);
}

void bunpin(struct buf *b)
{
  int cached_num = (b->blockno) % NBUCKETS;
  acquire(&bcache.lock[cached_num]);
  b->refcnt--;
  release(&bcache.lock[cached_num]);
}
