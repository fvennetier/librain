/* Librain, an erasure code library.
 * Copyright (C) 2011 Worldline, original work.
 * Copyright (C) 2015 OpenIO, modified as part of its software defined storage solution.
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the BSD license.  See the LICENSE file for details.
 */

#ifndef LIBRAIN_H
#define LIBRAIN_H 1

#include <stdint.h>

#define LIBRAIN_NOALLOC 0x01

#ifdef __cplusplus
extern "C" {
#endif

enum rain_algorithm_e {
	JALG_unset = 0,
	JALG_liberation,
	JALG_crs
};

struct rain_env_s
{
	void* (*malloc) (size_t size);
	void* (*calloc) (size_t nmemb, size_t size);
	void (*free) (void *ptr);
};

/** Encoding parameters */
struct rain_encoding_s
{
	size_t packet_size;
	size_t block_size; /**< The size of data or parity blocks */
	unsigned int w, k, m;
	enum rain_algorithm_e algo;

	size_t data_size;
	size_t padded_data_size; /**< The size of original data + padding */
	size_t strip_size;
};

/**
 * Prepare a RAIN computation.
 *
 * @param encoding pointer to an already allocated (struct rain_encoding_s)
 * @param rawlength the length of data that will be encoded or rehydrated
 * @param k the number of data blocks
 * @param m the number of parity blocks
 * @param algo the name of a RAIN algorithm ("liber8tion" or "crs")
 * @return 0 on error (errno is set)
 */
int rain_get_encoding (struct rain_encoding_s *encoding, size_t rawlength,
		unsigned int k, unsigned int m, const char *algo);

/** Fills 'out' with an array of coding chunks resulting from the parity
 * computation of the original file previously stripped and overheaded with
 * '0' at its end.
 * @param rawdata cannot be NULL and must be 'rawlength' long. No padding expected.
 * @param rawlength must be > 0
 * @param enc cannot be NULL
 * @param env can be NULL
 * @param out must have at least enc->m slots
 * @return a boolean value, false if it failed
 */
int rain_encode (uint8_t *rawdata, size_t rawlength,
		struct rain_encoding_s *enc, struct rain_env_s *env,
		uint8_t **out);

/** Fills 'out' with an array of coding chunks resulting from the parity
 * computation of data block. This function does not allocate memory
 * (except for its internal use), you have to provide the output
 * memory blocks.
 *
 * @param enc a non-NULL (struct rain_encoding_s *) pointer.
 * @param data must have at least enc->k slots
 *   of size enc->block_size bytes. If your data is shorter,
 *   pad with zeroes.
 * @param out must have at least enc->m preallocated slots
 *   of size enc->block_size bytes.
 * @return a boolean value, false if it failed
 */
int rain_encode_noalloc (struct rain_encoding_s *enc, uint8_t **data,
		uint8_t **out);

/** Regenerates missing data or coding chunks and returns the original data
 * (with a possible overhead of numerous '0' at its end).
 * @param data is expected to have at least enc->k slots,
 *   and padding chunks must be provided (not NULL!)
 * @param coding is expected to have at least enc->m slots
 * @param enc cannot be NULL
 * @param env can be NULL
 * @return a boolean value, false if it failed
 */
int rain_rehydrate (uint8_t **data, uint8_t **coding,
		struct rain_encoding_s *enc, struct rain_env_s *env);

/** Regenerates missing data or parity blocks. This function
 * does not allocate memory (except for its internal use),
 * you have to provide the output memory blocks.
 *
 * @param enc a non-NULL (struct rain_encoding_s *) pointer.
 * @param data must have at least enc->k slots
 *   of size enc->block_size bytes.
 * @param parity must have at least enc->m slots
 *   of size enc->block_size bytes.
 * @param erasures an array of integers, with indices
 *   of missing blocks (0 to enc->k-1 for data,
 *   enc->k to enc->k+enc->m-1 for parity), and a final -1.
 * @return a boolean value, false if it failed
 */
int rain_rehydrate_noalloc (struct rain_encoding_s *enc, uint8_t **data,
		uint8_t **parity, int *erasures);

#ifndef HAVE_NOLEGACY
/* Legacy interface */

int rain_repair_and_get_raw_data (uint8_t **data, uint8_t **coding,
		size_t raw_data_size, unsigned int k, unsigned int m,
		const char* algo);

uint8_t** rain_get_coding_chunks (uint8_t* raw_data, size_t raw_data_size,
		unsigned int k, unsigned int m,
		const char* algo);

int get_chunk_size(int raw_data_size, int k, int m, const char* algo);

// TODO FIXME Why is here no 'm' ?
int get_overhead_percentage(int raw_data_size, int k, int chunk_size,
		const char* algo);

#endif

#ifdef __cplusplus
}
#endif

#endif // LIBRAIN_H
