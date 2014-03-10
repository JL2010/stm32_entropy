
#include "stm32f10x.h"
#include "entropy.h"

/**********************************************************
 * Pool implemented as a queue
 * Ring buffer management routines
 *********************************************************/
static inline int isFull(EntropyPool *ep)
{
    if (ep->head == 0 && ep->tail == ep->num_chunks - 1)
	return 1;
    else if (ep->tail == ep->head - 1)
	return 1;
    else {}
    
    return 0;
}

static inline int isEmpty(EntropyPool *ep)
{
    return (ep->count < 8);
}

static inline void incTail(EntropyPool *ep)
{
    if (ep->tail == ep->num_chunks-1)
	ep->tail = 0;
    else
	ep->tail++;
}

static inline void incHead(EntropyPool *ep)
{
    if (ep->head == ep->num_chunks-1)
	ep->head = 0;
    else
	ep->head++;
}

/**********************************************************
 * Initialize the structure
 *********************************************************/
void Entropy_Init(EntropyPool *ep)
{
    ep->num_chunks = (POOL_SIZE * 32) / 4;
    ep->head = 0;
    ep->tail = 0;
    ep->count = 0;
}

/**********************************************************
 * Append lower 4 bits of 'val' to entropy pool
 * If pool is full, leave RTC interrupt disabled
 *********************************************************/
void Entropy_AppendToPool(EntropyPool *ep, uint32_t val)
{
    /* Disable RTC interrupt */
    NVIC_DisableIRQ(RTC_IRQn);

    if (isFull(ep))
    {
        /* We can return strait away with RTC interrupt disabled */
	return;
    }
    
    // Append in 4-bit chunks
    val &= 0xF;
    uint32_t	word_ind = ep->tail / 8;
    uint32_t	nibble	 = ep->tail % 8;
    uint32_t	mask	 = ~(0xF << (nibble * 4));
    
    ep->pool[word_ind] &= mask;
    ep->pool[word_ind] |= (val << (nibble * 4));
        
    incTail(ep);
    ep->count++;
    
    /* Re-enable RTC globally */
    NVIC_EnableIRQ(RTC_IRQn);
}

/**********************************************************
 * Return a 32-bit number from the entropy pool
 * Re-enable RTC interrupt to fill pool again
 *********************************************************/
uint32_t Entropy_GetRandom(EntropyPool *ep, uint32_t *val)
{
    uint32_t ret = 0;
    
    /* Disable RTC interrupt */
    NVIC_DisableIRQ(RTC_IRQn);

    if (isEmpty(ep))
    {
	ret = 1;
	goto end;
    }

    uint32_t word_ind = ep->head / 8;
    *val = ep->pool[word_ind];

    /* Increment head by 8 */
    incHead(ep);
    incHead(ep);
    incHead(ep);
    incHead(ep);
    incHead(ep);
    incHead(ep);
    incHead(ep);
    incHead(ep);

    ep->count -= 8;
        
end:
    /* Re-enable interrupts globally */
    NVIC_EnableIRQ(RTC_IRQn);
    return ret;
}

/**********************************************************
 * Same as Entropy_GetRandom, but bounded
 *********************************************************/
uint32_t Entropy_GetRandomBounded(EntropyPool *ep, uint32_t *val, uint32_t upper_bound)
{
    uint32_t temp;
    uint32_t ret = Entropy_GetRandom(ep, &temp);
    
    if (ret == 0)
	*val = temp % upper_bound;

    return ret;
}
