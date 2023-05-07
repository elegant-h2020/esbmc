#if !defined(CURAND_XORWOW_PRECALCULATED_H_)
#define CURAND_XORWOW_PRECALCULATED_H_

#define PRECALC_NUM_MATRICES (8)
#define PRECALC_BLOCK_SIZE (2)
#define PRECALC_BLOCK_MASK ((1 << PRECALC_BLOCK_SIZE) - 1)
#define XORWOW_SEQUENCE_SPACING (67)

#endif // CURAND_XORWOW_PRECALCULATED_H_