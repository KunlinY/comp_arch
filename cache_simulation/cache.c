//========================================================//
//  cache.c                                               //
//  Source file for the Cache Simulator                   //
//                                                        //
//  Implement the I-cache, D-Cache and L2-cache as        //
//  described in the README                               //
//========================================================//

#include "cache.h"
#include "stdio.h"
//
// TODO:Student Information
//
const char *studentName = "Apurba Bose";
const char *studentID   = "A53275012";
const char *email       = "apbose@eng.ucsd.edu";


int iflag = 0;
//------------------------------------//
//        Cache Configuration         //
//------------------------------------//

uint32_t icacheSets;     // Number of sets in the I$
uint32_t icacheAssoc;    // Associativity of the I$
uint32_t icacheHitTime;  // Hit Time of the I$

uint32_t dcacheSets;     // Number of sets in the D$
uint32_t dcacheAssoc;    // Associativity of the D$
uint32_t dcacheHitTime;  // Hit Time of the D$

uint32_t l2cacheSets;    // Number of sets in the L2$
uint32_t l2cacheAssoc;   // Associativity of the L2$
uint32_t l2cacheHitTime; // Hit Time of the L2$
uint32_t inclusive;      // Indicates if the L2 is inclusive

uint32_t blocksize;      // Block/Line size
uint32_t memspeed;       // Latency of Main Memory

//------------------------------------//
//          Cache Statistics          //
//------------------------------------//

uint64_t icacheRefs;       // I$ references
uint64_t icacheMisses;     // I$ misses
uint64_t icachePenalties;  // I$ penalties

uint64_t dcacheRefs;       // D$ references
uint64_t dcacheMisses;     // D$ misses
uint64_t dcachePenalties;  // D$ penalties

uint64_t l2cacheRefs;      // L2$ references
uint64_t l2cacheMisses;    // L2$ misses
uint64_t l2cachePenalties; // L2$ penalties


//------------------------------------//
//        Cache Data Structures       //
//------------------------------------//

typedef struct Block
{
    unsigned long int tag;
    uint8_t valid; //valid bit
    uint32_t LRU;

}Block;

typedef struct Set
{
    Block* cacheLine;
    int num_valid;
}Set;

typedef struct Cache
{
    int num_sets;
    int num_ways;
    Set* cacheSet;

}Cache;


//
//TODO: Add your Cache data structures here
//


uint8_t icache_indexBits = 0;
uint8_t icache_OffsetBits = 0; 
uint8_t icache_TagBits = 0;

///data Cache
uint8_t dcache_indexBits = 0;
uint8_t dcache_OffsetBits = 0; 
uint8_t dcache_TagBits = 0;

///L2 Cache
uint8_t l2cache_indexBits = 0;
uint8_t l2cache_OffsetBits = 0; 
uint8_t l2cache_TagBits = 0;

Cache* iCache = NULL;
Cache* dCache = NULL;
Cache* l2Cache = NULL;
//------------------------------------//
//          Cache Functions           //
//------------------------------------//

// Initialize the Cache Hierarchy
//


//return the log of an integer
int log_num(int number)
{
	int i;
	for (i = 0; number != 1; i++)
	{
		number = number/2;
	}	
	return i;
}

int getBits(uint32_t num)
{
    uint8_t position = 0;
    uint32_t mask = 1;
    while( (num & mask) != 1)
    {
        ///right shift
        num = num >> 1;
        position++;
    }
    return position;
}



void init_Inst_Cache()
{
    ///Initializing Cache
    iCache = (Cache*)malloc(sizeof(Cache));
    iCache->num_sets = icacheSets;
    iCache->num_ways = icacheAssoc;
    iCache->cacheSet = (Set*)malloc(sizeof(Set)*icacheSets);
    int cnt = 0;
    for(int i=0;i<iCache->num_sets;++i)
    {
        iCache->cacheSet[i].cacheLine = (Block*)malloc(sizeof(Block)*icacheAssoc);
        iCache->cacheSet[i].num_valid = 0;
        for(int j=0;j<icacheAssoc;++j)
        {
            iCache->cacheSet[i].cacheLine[j].tag = 0;
            iCache->cacheSet[i].cacheLine[j].valid= 0;
            iCache->cacheSet[i].cacheLine[j].LRU = 0;
            cnt = cnt+1;
        }

    }

    //printf("Icache----------------------------------------------\n");
    //printCache(iCache);
    //printf("-----------------------------------------------------\n");



    ///Get the bits for the index, block offset, Tag
    icache_indexBits = log_num(icacheSets);
    icache_OffsetBits = log_num(blocksize);
    icache_TagBits = 32 - icache_indexBits -  icache_OffsetBits;

    //printf("I-CACHE:\n");
    //printCacheBits(icache_indexBits, icache_BlockOffsetBits, icache_TagBits);


}

void init_Data_Cache()
{
    ///Initializing Cache
    dCache = (Cache*)malloc(sizeof(Cache));
    dCache->num_sets = dcacheSets;
    dCache->num_ways = dcacheAssoc;
    dCache->cacheSet = (Set*)malloc(sizeof(Set)*dcacheSets);
    int cnt = 0;
    for(int i=0;i<dCache->num_sets;++i)
    {
        dCache->cacheSet[i].cacheLine = (Block*)malloc(sizeof(Block)*dcacheAssoc);
        dCache->cacheSet[i].num_valid = 0;
        for(int j=0;j<dcacheAssoc;++j)
        {
            dCache->cacheSet[i].cacheLine[j].tag = 0;
            dCache->cacheSet[i].cacheLine[j].valid = 0;
            dCache->cacheSet[i].cacheLine[j].LRU = 0;
            cnt = cnt+1;
        }

    }


    ///Get the bits for the index, block offset, Tag
    dcache_indexBits = log_num(dcacheSets);
    dcache_OffsetBits = log_num(blocksize);
    dcache_TagBits = 32 - dcache_indexBits -  dcache_OffsetBits;

    //printf("D-CACHE:\n");
    //printCacheBits(dcache_indexBits, dcache_BlockOffsetBits, dcache_TagBits);
}

void init_L2_Cache()
{
    ///Initializing Cache
    l2Cache = (Cache*)malloc(sizeof(Cache));
    l2Cache->num_sets = l2cacheSets;
    l2Cache->num_ways = l2cacheAssoc;
    l2Cache->cacheSet = (Set*)malloc(sizeof(Set)*l2cacheSets);
    int cnt = 0;
    for(int i=0;i<l2Cache->num_sets;++i)
    {
        l2Cache->cacheSet[i].cacheLine = (Block*)malloc(sizeof(Block)*l2cacheAssoc);
        l2Cache->cacheSet[i].num_valid = 0;
        for(int j=0;j<l2cacheAssoc;++j)
        {
            l2Cache->cacheSet[i].cacheLine[j].tag = 0;
            l2Cache->cacheSet[i].cacheLine[j].valid = 0;
            l2Cache->cacheSet[i].cacheLine[j].LRU = 0;
            cnt = cnt+1;
        }

    }
	l2cache_indexBits = log_num(l2cacheSets);
    l2cache_OffsetBits = log_num(blocksize);// + getBits(l2cacheAssoc);
    l2cache_TagBits = 32 - l2cache_indexBits -  l2cache_OffsetBits;

    ///printCacheBits(l2cache_indexBits, l2cache_BlockOffsetBits, l2cache_TagBits);
}

/*
Cache* createCache(int cacheSize, int blockSize, int numWays){
    if(cacheSize<=0 || blockSize <=0){
        //printf("Invalid input!");
        exit;
    }
    Cache *cache = malloc(sizeof(Cache));
    //initialize all values
    cache->hits=0;
    cache->misses=0;
    cache->loads=0;
    cache->stores=0;
    cache->cacheSize=cacheSize;
    cache->blockSize=blockSize;
	cache->numWays= numWays;
	//printf("%d\nnumways",numWays);
    cache->numBlocks=(int)(cacheSize/blockSize);
	//printf("%d\nnumblocks",cache->numBlocks);
    cache->numSets=(int)(cache->numBlocks/numWays);
	//printf("%d\nnumsets", cache->numSets);
 
    if(numWays >= 1){
			//for each set
			cache->sets = malloc((cache->numSets) * sizeof(set));
            for(int i=0; i < cache->numSets; i++){
                cache->sets[i].numBlocks = cache->numBlocks;
				cache->sets[i].numBlocks = 0;
				cache->sets[i].front = cache->sets[i].rear = NULL;
            }
    }//end if
    return cache;
}
*/

void init_cache()
{
  // Initialize cache stats
    icacheRefs        = 0;
    icacheMisses      = 0;
    icachePenalties   = 0;
    dcacheRefs        = 0;
    dcacheMisses      = 0;
    dcachePenalties   = 0;
    l2cacheRefs       = 0;
    l2cacheMisses     = 0;
    l2cachePenalties  = 0;

  //
  //TODO: Initialize Cache Simulator Data Structures
  //

    if(icacheSets!=0)
    {
        init_Inst_Cache(); 			//This is for the last bitcoin case when nothing is instantiated
    }
    if(dcacheSets!=0)
    {
        init_Data_Cache();
    }

    init_L2_Cache();
}


//called when one way is free
int insert_Cache_index(Cache *cache_name, int setval, int tagval)
{
    int index = -1;
    for(int i=0;i< cache_name->num_ways;i++)
    {
        if(cache_name->cacheSet[setval].cacheLine[i].valid==0)
        {
            ///checking if could insert here
            cache_name->cacheSet[setval].cacheLine[i].valid = 1;
            cache_name->cacheSet[setval].num_valid++;
            cache_name->cacheSet[setval].cacheLine[i].tag = tagval;
            index = i;
            break;
        }
    }
	return index;
}

//function to update the lru of the most recently used to maximum
void update_LRU(Cache *cache_name, int setval, int block_inserted)
{
    uint32_t curr_LRU = cache_name->cacheSet[setval].cacheLine[block_inserted].LRU;
    for(int i=0; i<cache_name->num_ways;++i)
    {
        if(i!=block_inserted)
        {
            if(cache_name->cacheSet[setval].cacheLine[i].LRU > curr_LRU)
            {
                cache_name->cacheSet[setval].cacheLine[i].LRU = cache_name->cacheSet[setval].cacheLine[i].LRU - 1;//decrementing the LRU of the current way
            }
        }
    }

    ///set current position's LRU to max
    uint32_t maxlru = (1 << cache_name->num_ways) - 1;
    cache_name->cacheSet[setval].cacheLine[block_inserted].LRU = maxlru;

    return;
}

//evict the index having the min lru
void Cacheevict(Cache* cache_name, int setval)
{
 
    int index_min_LRU;
    int min_LRU;
    for(int i=0;i < cache_name->num_ways;++i)
    {
        //meaning this is index 0
		if(i==0)
        {
            min_LRU = cache_name->cacheSet[setval].cacheLine[i].LRU;
            index_min_LRU = 0;
        }
        else
        {
            if(min_LRU > cache_name->cacheSet[setval].cacheLine[i].LRU)
            {
                min_LRU = cache_name->cacheSet[setval].cacheLine[i].LRU;
                index_min_LRU = i;
            }

        }
    }

    cache_name->cacheSet[setval].cacheLine[index_min_LRU].valid = 0;
    cache_name->cacheSet[setval].num_valid--;


    return;
}



// Perform a memory access through the icache interface for the address 'addr'
// Return the access time for the memory operation
//

void printLRU(Cache* cache_name, int val_set)
{
    for(int i=0;i< cache_name->num_ways;++i)
    {
        //printf("Pos: %d LRU: %x\n", i, c->cacheSet[setval].cacheLine[i].LRU);
    }
}
int return_index(int addr, int index_bits, int offset_bits)
{
	//int num_offset_bits = log_num(cache_name-> blockSize); //returns 6 in the case of 64
	//int num_index_bits = log_num(cache_name-> numSets); 
	int num_tag_bits = 32 - index_bits - offset_bits;
	int mask = ((1 << index_bits) - 1);
	int index = (addr >> offset_bits) & mask;
	return index;
}

int return_tag(int addr, int index_bits, int offset_bits)
{
	//int num_offset_bits = log_num(cache_name-> blockSize); //returns 6 in the case of 64
	//int num_index_bits = log_num(cache_name-> numSets);    
	int num_tag_bits = 32 - index_bits - offset_bits;
	int mask = ((1 << num_tag_bits) - 1);
	int tag = (addr >> (offset_bits + index_bits) & mask);
	return tag;
}
uint32_t
icache_access(uint32_t addr)
{

   iflag = 1;
  //the case of the bitcoin miner
  if(icacheSets==0)
  {
      int l1_memspeed = l2cache_access(addr);
      return l1_memspeed;
  }

  ///Code to take care of uninstantiated cache
  icacheRefs = icacheRefs + 1;
  int memspeed_icache = 0;
  //int index = ((1 << icache_indexBits) - 1)&( addr >> (icache_BlockOffsetBits) );
  //int tag = ((1 << icache_TagBits) - 1)& (addr >> (icache_BlockOffsetBits + icache_indexBits) );
  int index = return_index(addr, icache_indexBits, icache_OffsetBits);
  int tag = return_tag(addr, icache_indexBits, icache_OffsetBits);
  int hit = 0;

  ///searching in all the blocks
  for(int i=0;i<iCache->num_ways;++i)
  {
      ///for the set:index
      if(iCache->cacheSet[index].cacheLine[i].valid!=0)
      {
          if(iCache->cacheSet[index].cacheLine[i].tag == tag)
          {
             update_LRU(iCache,index,i);
             //printLRU(iCache, masked_bits_for_index);
             hit = 1;
             ///printf("%x: HIT\n", addr);
             break;
          }

      }
  }

  if(hit==0)
  {
        ////printf("%x: its a MISS\n", addr);
        icacheMisses++;
        int l2_memspeed = l2cache_access(addr); ///based on memspeed we can find whether it was hit or miss
        memspeed_icache = memspeed_icache + l2_memspeed;
        //printf("Num_blocks valid: %d\n", iCache->cacheSet[masked_bits_for_index].blocks_valid);
        if (iCache->cacheSet[index].num_valid < iCache->num_ways)
        {
            ///there is an empty space
            int block_inserted = insert_Cache_index(iCache, index, tag);
            update_LRU(iCache,index,block_inserted);
            printLRU(iCache, index);
        }
        else
        {
            ///Now the icache is full, so evict LRU and insert in that place
            //printf("ICache is full Evict!!\n");
            printLRU(iCache, index);
            Cacheevict(iCache,index);
            int block_inserted = insert_Cache_index(iCache,index,tag);
            update_LRU(iCache,index,block_inserted);
        }
  }
//printf("--------------------END ICACHE-----------------\n");


  if(hit)
  {
      return icacheHitTime;
  }
  else
  {
      icachePenalties = icachePenalties + memspeed_icache;
      return memspeed_icache +icacheHitTime;
  }
  ///return memspeed;
}




// Perform a memory access through the dcache interface for the address 'addr'
// Return the access time for the memory operation
//
uint32_t
dcache_access(uint32_t addr)
{
     iflag = 0;
  if(dcacheSets==0)
  {
      int l2_memspeed = l2cache_access(addr);
      return l2_memspeed;
  }


  dcacheRefs++;
  int memspeed_dcache = 0;
  int index = return_index(addr, dcache_indexBits, dcache_OffsetBits);
  int tag = return_tag(addr, dcache_indexBits, dcache_OffsetBits);
  int hit = 0;


  ///DCACHE
  for(int i=0;i<dCache->num_ways;++i)
  {
        if(dCache->cacheSet[index].cacheLine[i].valid!=0)
        {
            if(dCache->cacheSet[index].cacheLine[i].tag==tag)
            {
                update_LRU(dCache,index,i);
                hit=1;
                break;
            }
        }
  }

  if(hit==0)
  {
        dcacheMisses++;
        int l2_memspeed = l2cache_access(addr);
        memspeed_dcache = memspeed_dcache + l2_memspeed;
//printf("Num_blocks valid: %d\n", dCache->cacheSet[index].blocks_valid);

        if(dCache->cacheSet[index].num_valid < dCache->num_ways)
        {
            //space available
            int block_inserted = insert_Cache_index(dCache, index, tag);
//printf("dCache Inserted Tag: %x at position: %d\n", dCache->cacheSet[index].cacheLine[pos_block_inserted].tagVal, pos_block_inserted);
            update_LRU(dCache,index,block_inserted);
        }
        else
        {
            ///evict LRU and insert in that place
            //printLRU(dCache, index);
            Cacheevict(dCache,index);
            int block_inserted = insert_Cache_index(dCache,index,tag);
//printf("DCache Inserted Tag: %x at position: %d\n", dCache->cacheSet[index].cacheLine[pos_block_inserted].tagVal, pos_block_inserted );
            update_LRU(dCache,index, block_inserted);
        }

  }

  if(hit)
  {
      return dcacheHitTime;
  }
  else
  {
      dcachePenalties = dcachePenalties + memspeed_dcache;
      return memspeed_dcache+dcacheHitTime;
  }
  ///return memspeed;

}

// Perform a memory access to the l2cache for the address 'addr'
// Return the access time for the memory operation
//
///Didn't check l2Cache (but should be okay)
uint32_t
l2cache_access(uint32_t addr)
{
  ///code to take care of bitcoin miner uninstantiated cache:: when number of sets are zero
  l2cacheRefs++;
  
  
  

  int index = return_index(addr, l2cache_indexBits, l2cache_OffsetBits);
  int tag = return_tag(addr, l2cache_indexBits, l2cache_OffsetBits);
  int hit = 0;
  ///searching in all the blocks
  for(int i=0;i<l2Cache->num_ways;++i)
  {
      ///for the set: masked_bits_for_index
      if(l2Cache->cacheSet[index].cacheLine[i].valid!=0)
      {
          if(l2Cache->cacheSet[index].cacheLine[i].tag == tag)
          {
             ///if(iflag==1) {printf("%x L2 HIT\n", addr);}
             update_LRU(l2Cache, index,i);
             hit = 1;
             break;
          }

      }

  }


  if(hit==0)
  {
        l2cacheMisses++;
        ///if(iflag==1) {printf("Line 372: L2 Miss\n");}

        ///load word in L2. If there is space no need to evict
        if(l2Cache->cacheSet[index].num_valid < l2Cache->num_ways)
        {
            ///cache has space
            ///insert and update LRU in one function call
            int block_inserted  = insert_Cache_index(l2Cache,index, tag);
            update_LRU(l2Cache,index, block_inserted);
        }
        else
        {
            ///evict using LRU
            Cacheevict(l2Cache,index);
            int block_inserted = insert_Cache_index(l2Cache,index,tag);///insert Cache
            ///updateLRU
            update_LRU(l2Cache, index, block_inserted);

            if(inclusive)
            {
                ///evict from I/D
                ///will do at last
            }
        }


        ///memory penalty
  }
   ///printf("---------------END L2Cache---------------------\n");
  if(hit)
  {
      return l2cacheHitTime;
  }
  else
  {
    l2cachePenalties = l2cachePenalties + memspeed;
    return memspeed+l2cacheHitTime;
  }

}







