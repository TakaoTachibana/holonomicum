/**
 * @file test_shm_v5.c
 * @brief HOLONOMICUM Cytoplasm V Shared Memory Attach & Sanity Test
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <errno.h>
#include <inttypes.h>
#include "../include/cytoplasm_v5.h"

int main(void) {
	printf("=== HOLONOMICUM // Cytoplasm V Shared Memory Verification ===\n\n");

	/* 1. Comple-Time & Runtime Layout Checks */
	printf("[1] Checking Structure Offsets & Memory Size...\n");
	printf("  - Size of Complex64: %zu B\n", sizeof(Complex64));
	printf("  - Size of HeaderSection: %zu B\n", sizeof(HeaderSection));
	printf("  - Size of DensityMatrixSection: %zu B\n", sizeof(DensityMatrixSection));
	printf("  - Size of ComplexVectorSlot: %zu B\n", sizeof(ComplexVectorSlot));
	printf("  - Size of ComplexTextSlot: %zu B\n", sizeof(ComplexTextSlot));
	printf("  - Total CytoplasmV5 Size: %zu B (%.2f MB)\n\n",
		sizeof(CytoplasmV5), (double)sizeof(CytoplasmV5) / (1024.0 * 1024.0));

	printf("  - Offset of density_matrix: 0x%08ZX\n", offsetof(CytoplasmV5, density_matrix));
	printf("  - Offset of vectors: 0x%08ZX\n", offsetof(CytoplasmV5, vectors));
	printf("  - Offset of text_lru: 0x%08ZX\n\n", offsetof(CytoplasmV5, text_lru));

	if (offsetof(CytoplasmV5, text_lru) != 0x0C068000) {
		fprintf(stderr, "[FATAL ERROR] text_lru offset mismatch! Expected 0x0C068000.\n");
		return EXIT_FAILURE;
	}

	/* 2. Attach or Create Shared Memory */
	printf("[2] Attaching to System V Shared Memory (Key: 0%08X, Size: 512MB)...\n", CYTOPLASM_V5_IPC_KEY);

	int shmid = shmget((key_t)CYTOPLASM_V5_IPC_KEY, sizeof(CytoplasmV5), IPC_CREAT | 0666);
	if (shmid < 0) {
		perror("  [ERROR] shmget failed");
		return EXIT_FAILURE;
	}
	printf("  - shmget SUCCESS: shmid = %d\n", shmid);

	void *shm_ptr = shmat(shmid, NULL, 0);
	if (shm_ptr == (void *)-1) {
		perror("  [ERROR] shmat failed");
		return EXIT_FAILURE;
	}
	printf("  - shmat SUCCESS: Attached at virtual address %p\n\n", shm_ptr);

	CytoplasmV5 *cytoplasm = (CytoplasmV5 *)shm_ptr;

	/* 3. Initialize of Validate Header */
	printf("[3] Validating Header Magic Number...\n");
	if (cytoplasm->header.magic_number != 0x48F4C4F484F4C4FULL) {
		printf("  - Uninitialized Shared Memory segment detected. Initializing Header...\n");
		cytoplasm->header.magic_number = 0x484F4C4F484F4C4FULL; /* HOLOHOLO" */
		cytoplasm->header.write_index = 0;
		cytoplasm->header.read_index = 0;
		cytoplasm->header.von_neumann_entropy = 1.0;
		cytoplasm->header.state_flags = STATE_FLAG_STABLE;
		printf("  - Header Initialized: Magic = 0x%" PRIX64 " [HOLOHOLO]\n", cytoplasm->header.magic_number);
	} else {
		printf("  - Existing Shared Memory Segment Verified: Magic = 0x%" PRIX64 " [HOLOHOLO]\n", cytoplasm->header.magic_number);
	}

	/* 4. Execute Read/Write Sanity Tests (Complex & Seqlock) */
	printf("\n[4] Executing Read/Write Sanity Tests...\n");

	uint64_t current_widx = ++cytoplasm->header.write_index;

	// Tests Complex Vector Ring Buffer Write
	uint32_t v_slot_idx = (uint32_t)(current_widx % COMPLEX_RING_CAPACITY);
	cytoplasm->vectors[v_slot_idx].slot_id = current_widx;
	cytoplasm->vectors[v_slot_idx].values[0].real = 0.707f;
	cytoplasm->vectors[v_slot_idx].values[0].imag = 0.707f;

	printf("  - Complex Vector Slot [%u] Write OK (Real: %.3f, Imag: %.3f)\n",
		v_slot_idx, cytoplasm->vectors[v_slot_idx].values[0].real, cytoplasm->vectors[v_slot_idx].values[0].imag);

	// Test Text LRU Buffer Write with Seqlock Pattern
	uint32_t t_slot_idx = (uint32_t)(current_widx % TEXT_LRU_CAPACITY);
	ComplexTextSlot *tSlot = &cytoplasm->text_lru[t_slot_idx];

	// Seqlock: Acquire Write Lock (Odd Number)
	tSlot->sequence += 1;
	__sync_synchronize(); // Memory Barrier (Compiler & CPU)

	tSlot->slot_id = current_widx;
	snprintf(tSlot->uri, sizeof(tSlot->uri), "at://did:plc:holo/app.bsky.feed.post/test001");
	snprintf(tSlot->text, sizeof(tSlot->text), "HOLONOMICUM Cytoplasm V Seqlock Test");
	tSlot->structural_role = ROLE_SINGULARITY_CATALYST;

	__sync_synchronize(); // Memory Barrier
	// Seqlock: Release Write Lock (Even Number)
	tSlot->sequence += 1;

	printf("  - Text LRU Slot [%u] Write OK with Seqlock (Seq: %" PRIu64 ", URI: %s)\n",
		t_slot_idx, tSlot->sequence, tSlot->uri);

	/* 5. Detach */
	printf("\n[5] Detaching Shared Memory...\n");
	if (shmdt(shm_ptr) < 0) {
		perror("  [ERROR] shmdt failed");
		return EXIT_FAILURE;
	}
	printf("  - shmdt SUCCESS. Shared Memory Verification Passed Cleanly!\n\n");
	printf("=== ALL CHECKS PASSED ===\n");

	return EXIT_SUCCESS;
}



