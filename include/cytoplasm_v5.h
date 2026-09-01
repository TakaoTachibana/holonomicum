/**
 * @file cytoplasm_v5.h
 * @brief HOLONOMICUM Cytoplasm V Shared Memory Specification
 * @details Polyglot Atomic Design with Seqlock & 128D Complex Hilbert Space
 */

#ifndef CYTOPLASM_V5_H
#define CYTOPLASM_V5_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/*=== 1. Constants & System Configuration ===*/

#define CYTOPLASM_V5_IPC_KEY 0x484F4C4FU /* "HOLO" */

/* Dimensionality & Capacities */
#define VECTOR_DIM 128
#define GRAPH_MAX_NODES 256
#define MAX_TOP_TRIGGER_POSTS 64U
#define COMPLEX_RING_CAPACITY 131072U /* 131,072 slots (~192 MB) */
#define TEXT_LRU_CAPACITY 16384U /* 16,384 slots (~320 MB) */

/* System State Flags (Bitmask) */
#define STATE_FLAG_STABLE 0x00
#define STATE_FLAG_QUIET 0x01
#define STATE_FLAG_PERTURBED 0x02
#define STATE_FLAG_CRITICAL 0x04
#define STATE_FLAG_TDA_DISRUPTION 0x08
#define STATE_FLAG_HOLONOMIC_DISRUPTION 0x10

/* Structural Role Flags */
#define ROLE_SINGULARITY_CATALYST 0x01
#define ROLE_BOUNDARY_BREAKER 0x02
#define ROLE_VORTICITY_CENTER 0x04
#define ROLE_MONOPOLE_INDUCER 0x08

#pragma pack(push, 8) /* 8-byte strict alignment for all struct members */

/*=== 2. Core Complex Type ===*/
typedef struct {
	float real;
	float imag;
} Complex64; /* 8 bytes */

/*=== 3. Memory Layout Section ===*/

/* 0x00000000: Header Section (512 Bytes) */
typedef struct {
	uint64_t magic_number; /* 0x484F4C4F484F4C4FULL ("HOLOHOLO") */
	uint64_t write_index; /* Atomic Global Write Counter */
	uint64_t read_index; /* Atomic Global Read Pointer */
	double von_neumann_entropy; /* S(rho) */
	double berry_phase_norm; /* ||gamma_C|| */
	double fubini_study_curvature; /* FS Metic Curvature */
	double tda_h1_persistence;
	double tda_h2_persistence;
	uint32_t state_flags; /* SYSTEM STATE FLAGS */
	uint32_t active_node_count; /* Active Graph Nodes */
	uint64_t last_updated_epoch_ns; /* Timestamp (ns) */
	uint8_t reserved[432]; /* Padding to exactly 512 bytes */
} HeaderSection;

/* 0x00000200: Density Matrix Section (131,072 Bytes) */
typedef struct {
	Complex64 rho[VECTOR_DIM][VECTOR_DIM];
} DensityMatrixSection;

/* 0x00020200: Operator Coefficients Section (15,872 Bytes) */
typedef struct {
	double h0_diag[VECTOR_DIM];
	double c1_diag[VECTOR_DIM];
	double c2_diag[VECTOR_DIM];
	double gamma_k[VECTOR_DIM];
	double sindy_residual;
	uint8_t reserved[11768]; /* Padding to exactly 15.872 bytes */
} OperatorCoefficients;

/* 0x00024000: Particle Output Area (16,384 Bytes) */
typedef struct {
	uint32_t slot_id;
	uint32_t structural_role;
	double attributionn_score;
	double local_berry_curvature;
} TriggerParticle; /* 24 Bytes */

typedef struct {
	uint32_t trigger_count;
	uint32_t padding; /* Align to 8-byte */
	uint64_t calculated_at_ns;
	TriggerParticle particles[MAX_TOP_TRIGGER_POSTS]; /* 64 * 24 = 1536 B */
	uint8_t reserved[14832]; /* Padding to exactly 16,384 bytes */
} ParticleOutputArea;

/* 0x00028000: Adjacency Matrix Section (262,144 Bytes) */
typedef struct {
	float weights[GRAPH_MAX_NODES][GRAPH_MAX_NODES];
} AdjacencyMatrixSection;

/* 0x00068000: Complex Vector Slot (1,536 Bytes / Slot) */
typedef struct {
	uint64_t slot_id;
	uint64_t timestamp_ns;
	Complex64 values[VECTOR_DIM]; /* 1024 Bytes */
	uint8_t reserved[496]; /* Padding to exactly 1,536 bytes */
} ComplexVectorSlot;

/* 0x0C068000: Text LRU Buffer Slot with Seqlock (20,480 Bytes / Slot) */
typedef struct {
	uint64_t sequence; /* Seqlock: odd=writing, even=complete */
	uint64_t slot_id;
	uint64_t timestamp_ns;
	char uri[256];
	char author_did[128];
	char text[2048];
	double local_berry_curvature;
	uint32_t structural_role;
	uint32_t padding; /* Align to 8-byte */
	uint8_t reserved[18008]; /* Padding to exactly 20,480 bytes */
} ComplexTextSlot;


/*=== 4. Cytoplasm V Full Shared Memory Structure (~512.4 MB) ===*/
typedef struct {
	HeaderSection header; /* 0x00000000 */
	DensityMatrixSection density_matrix; /* 0x00000200 */
	OperatorCoefficients coefficients; /* 0x00020200 */
	ParticleOutputArea particles_output; /* 0x00024000 */
	AdjacencyMatrixSection adjacency_matrix; /* 0x00028000 */
	ComplexVectorSlot vectors[COMPLEX_RING_CAPACITY]; /* 0x00068000 */
	ComplexTextSlot text_lru[TEXT_LRU_CAPACITY]; /* 0x0C068000 */
} CytoplasmV5;

#pragma pack(pop)

/*=== 5. Compile-Time Alignment & Offset Assertions ===*/

#ifdef __cplusplus
	#ifndef _Static_assert
		#define _Static_assert static_assert
	#endif
#endif

/* Size Verfication */
_Static_assert(sizeof(Complex64) == 8, "Complex64 size must be 8 bytes");
_Static_assert(sizeof(HeaderSection) == 512, "HeaderSection size mismatch");
_Static_assert(sizeof(DensityMatrixSection) == 131072, "DensityMatrixSection size mismatch");
_Static_assert(sizeof(OperatorCoefficients) == 15872, "OperatorCoefficients size mismatch");
_Static_assert(sizeof(ParticleOutputArea) == 16384, "ParticleOutputArea size mismatch");
_Static_assert(sizeof(ComplexVectorSlot) == 1536, "ComplexVectorSlot size mismatch");
_Static_assert(sizeof(ComplexTextSlot) == 20480, "ComplexTextSlot size mismatch");

/* Absolute Offset Verification */
_Static_assert(offsetof(CytoplasmV5, density_matrix) == 0x00000200, "Offset density_matrix mismatch");
_Static_assert(offsetof(CytoplasmV5, coefficients) == 0x00020200, "Offset coefficients mismatch");
_Static_assert(offsetof(CytoplasmV5, particles_output) == 0x00024000, "Offset particles_output mismatch");
_Static_assert(offsetof(CytoplasmV5, adjacency_matrix) == 0x00028000, "Offset adjacency_matrix mismatch");
_Static_assert(offsetof(CytoplasmV5, vectors) == 0x00068000, "Offset vectors mismatch");
_Static_assert(offsetof(CytoplasmV5, text_lru) == 0x0C068000, "Offset text_lru mismatch");

#ifdef __cplusplus
}
#endif

#endif /* CYTOPLASM_V5_H */

