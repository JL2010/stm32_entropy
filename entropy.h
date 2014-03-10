
#define POOL_SIZE 8

typedef struct {
    uint32_t pool[POOL_SIZE];
    volatile uint8_t head, tail;
    uint8_t count;
    uint8_t num_chunks;
} EntropyPool;

void Entropy_Init(EntropyPool *ep);
void Entropy_AppendToPool(EntropyPool *ep, uint32_t val);
uint32_t Entropy_GetRandom(EntropyPool *ep, uint32_t *val);
uint32_t Entropy_GetRandomBounded(EntropyPool *ep, uint32_t *val, uint32_t upper_bound);
