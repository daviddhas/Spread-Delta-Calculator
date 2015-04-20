/**\file */
#ifndef SLIC_DECLARATIONS_FieldAccumulatorTCP_H
#define SLIC_DECLARATIONS_FieldAccumulatorTCP_H
#include "MaxSLiCInterface.h"
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define FieldAccumulatorTCP_PCIE_ALIGNMENT (16)
#define FieldAccumulatorTCP_USE_NETWORK_MANAGER (1)


/*----------------------------------------------------------------------------*/
/*---------------------------- Interface default -----------------------------*/
/*----------------------------------------------------------------------------*/




/**
 * \brief Basic static function for the interface 'default'.
 * 
 * \param [out] outscalar_SlicerFramer_network_mgmt_stream_from_host_tx_framed_eofCount Output scalar parameter "SlicerFramer_network_mgmt_stream_from_host_tx_framed.eofCount".
 * \param [out] outscalar_SlicerFramer_network_mgmt_stream_from_host_tx_framed_sofCount Output scalar parameter "SlicerFramer_network_mgmt_stream_from_host_tx_framed.sofCount".
 * \param [out] outscalar_network_mgmt_stream_to_host_rx_framed_FramedStreamPreprocessor_eofCount Output scalar parameter "network_mgmt_stream_to_host_rx_framed_FramedStreamPreprocessor.eofCount".
 * \param [out] outscalar_network_mgmt_stream_to_host_rx_framed_FramedStreamPreprocessor_sofCount Output scalar parameter "network_mgmt_stream_to_host_rx_framed_FramedStreamPreprocessor.sofCount".
 * \param [in] instream_network_mgmt_stream_from_host_tx_framed Stream "network_mgmt_stream_from_host_tx_framed".
 * \param [in] instream_size_network_mgmt_stream_from_host_tx_framed The size of the stream instream_network_mgmt_stream_from_host_tx_framed in bytes.
 * \param [out] outstream_network_mgmt_stream_to_host_rx_framed Stream "network_mgmt_stream_to_host_rx_framed".
 * \param [in] outstream_size_network_mgmt_stream_to_host_rx_framed The size of the stream outstream_network_mgmt_stream_to_host_rx_framed in bytes.
 */
void FieldAccumulatorTCP(
	uint64_t *outscalar_SlicerFramer_network_mgmt_stream_from_host_tx_framed_eofCount,
	uint64_t *outscalar_SlicerFramer_network_mgmt_stream_from_host_tx_framed_sofCount,
	uint64_t *outscalar_network_mgmt_stream_to_host_rx_framed_FramedStreamPreprocessor_eofCount,
	uint64_t *outscalar_network_mgmt_stream_to_host_rx_framed_FramedStreamPreprocessor_sofCount,
	const void *instream_network_mgmt_stream_from_host_tx_framed,
	size_t instream_size_network_mgmt_stream_from_host_tx_framed,
	void *outstream_network_mgmt_stream_to_host_rx_framed,
	size_t outstream_size_network_mgmt_stream_to_host_rx_framed);

/**
 * \brief Basic static non-blocking function for the interface 'default'.
 * 
 * Schedule to run on an engine and return immediately.
 * The status of the run can be checked either by ::max_wait or ::max_nowait;
 * note that one of these *must* be called, so that associated memory can be released.
 * 
 * 
 * \param [out] outscalar_SlicerFramer_network_mgmt_stream_from_host_tx_framed_eofCount Output scalar parameter "SlicerFramer_network_mgmt_stream_from_host_tx_framed.eofCount".
 * \param [out] outscalar_SlicerFramer_network_mgmt_stream_from_host_tx_framed_sofCount Output scalar parameter "SlicerFramer_network_mgmt_stream_from_host_tx_framed.sofCount".
 * \param [out] outscalar_network_mgmt_stream_to_host_rx_framed_FramedStreamPreprocessor_eofCount Output scalar parameter "network_mgmt_stream_to_host_rx_framed_FramedStreamPreprocessor.eofCount".
 * \param [out] outscalar_network_mgmt_stream_to_host_rx_framed_FramedStreamPreprocessor_sofCount Output scalar parameter "network_mgmt_stream_to_host_rx_framed_FramedStreamPreprocessor.sofCount".
 * \param [in] instream_network_mgmt_stream_from_host_tx_framed Stream "network_mgmt_stream_from_host_tx_framed".
 * \param [in] instream_size_network_mgmt_stream_from_host_tx_framed The size of the stream instream_network_mgmt_stream_from_host_tx_framed in bytes.
 * \param [out] outstream_network_mgmt_stream_to_host_rx_framed Stream "network_mgmt_stream_to_host_rx_framed".
 * \param [in] outstream_size_network_mgmt_stream_to_host_rx_framed The size of the stream outstream_network_mgmt_stream_to_host_rx_framed in bytes.
 * \return A handle on the execution status, or NULL in case of error.
 */
max_run_t *FieldAccumulatorTCP_nonblock(
	uint64_t *outscalar_SlicerFramer_network_mgmt_stream_from_host_tx_framed_eofCount,
	uint64_t *outscalar_SlicerFramer_network_mgmt_stream_from_host_tx_framed_sofCount,
	uint64_t *outscalar_network_mgmt_stream_to_host_rx_framed_FramedStreamPreprocessor_eofCount,
	uint64_t *outscalar_network_mgmt_stream_to_host_rx_framed_FramedStreamPreprocessor_sofCount,
	const void *instream_network_mgmt_stream_from_host_tx_framed,
	size_t instream_size_network_mgmt_stream_from_host_tx_framed,
	void *outstream_network_mgmt_stream_to_host_rx_framed,
	size_t outstream_size_network_mgmt_stream_to_host_rx_framed);

/**
 * \brief Advanced static interface, structure for the engine interface 'default'
 * 
 */
typedef struct { 
	uint64_t *outscalar_SlicerFramer_network_mgmt_stream_from_host_tx_framed_eofCount; /**<  [out] Output scalar parameter "SlicerFramer_network_mgmt_stream_from_host_tx_framed.eofCount". */
	uint64_t *outscalar_SlicerFramer_network_mgmt_stream_from_host_tx_framed_sofCount; /**<  [out] Output scalar parameter "SlicerFramer_network_mgmt_stream_from_host_tx_framed.sofCount". */
	uint64_t *outscalar_network_mgmt_stream_to_host_rx_framed_FramedStreamPreprocessor_eofCount; /**<  [out] Output scalar parameter "network_mgmt_stream_to_host_rx_framed_FramedStreamPreprocessor.eofCount". */
	uint64_t *outscalar_network_mgmt_stream_to_host_rx_framed_FramedStreamPreprocessor_sofCount; /**<  [out] Output scalar parameter "network_mgmt_stream_to_host_rx_framed_FramedStreamPreprocessor.sofCount". */
	const void *instream_network_mgmt_stream_from_host_tx_framed; /**<  [in] Stream "network_mgmt_stream_from_host_tx_framed". */
	size_t instream_size_network_mgmt_stream_from_host_tx_framed; /**<  [in] The size of the stream instream_network_mgmt_stream_from_host_tx_framed in bytes. */
	void *outstream_network_mgmt_stream_to_host_rx_framed; /**<  [out] Stream "network_mgmt_stream_to_host_rx_framed". */
	size_t outstream_size_network_mgmt_stream_to_host_rx_framed; /**<  [in] The size of the stream outstream_network_mgmt_stream_to_host_rx_framed in bytes. */
} FieldAccumulatorTCP_actions_t;

/**
 * \brief Advanced static function for the interface 'default'.
 * 
 * \param [in] engine The engine on which the actions will be executed.
 * \param [in,out] interface_actions Actions to be executed.
 */
void FieldAccumulatorTCP_run(
	max_engine_t *engine,
	FieldAccumulatorTCP_actions_t *interface_actions);

/**
 * \brief Advanced static non-blocking function for the interface 'default'.
 *
 * Schedule the actions to run on the engine and return immediately.
 * The status of the run can be checked either by ::max_wait or ::max_nowait;
 * note that one of these *must* be called, so that associated memory can be released.
 *
 * 
 * \param [in] engine The engine on which the actions will be executed.
 * \param [in] interface_actions Actions to be executed.
 * \return A handle on the execution status of the actions, or NULL in case of error.
 */
max_run_t *FieldAccumulatorTCP_run_nonblock(
	max_engine_t *engine,
	FieldAccumulatorTCP_actions_t *interface_actions);

/**
 * \brief Group run advanced static function for the interface 'default'.
 * 
 * \param [in] group Group to use.
 * \param [in,out] interface_actions Actions to run.
 *
 * Run the actions on the first device available in the group.
 */
void FieldAccumulatorTCP_run_group(max_group_t *group, FieldAccumulatorTCP_actions_t *interface_actions);

/**
 * \brief Group run advanced static non-blocking function for the interface 'default'.
 * 
 *
 * Schedule the actions to run on the first device available in the group and return immediately.
 * The status of the run must be checked with ::max_wait. 
 * Note that use of ::max_nowait is prohibited with non-blocking running on groups:
 * see the ::max_run_group_nonblock documentation for more explanation.
 *
 * \param [in] group Group to use.
 * \param [in] interface_actions Actions to run.
 * \return A handle on the execution status of the actions, or NULL in case of error.
 */
max_run_t *FieldAccumulatorTCP_run_group_nonblock(max_group_t *group, FieldAccumulatorTCP_actions_t *interface_actions);

/**
 * \brief Array run advanced static function for the interface 'default'.
 * 
 * \param [in] engarray The array of devices to use.
 * \param [in,out] interface_actions The array of actions to run.
 *
 * Run the array of actions on the array of engines.  The length of interface_actions
 * must match the size of engarray.
 */
void FieldAccumulatorTCP_run_array(max_engarray_t *engarray, FieldAccumulatorTCP_actions_t *interface_actions[]);

/**
 * \brief Array run advanced static non-blocking function for the interface 'default'.
 * 
 *
 * Schedule to run the array of actions on the array of engines, and return immediately.
 * The length of interface_actions must match the size of engarray.
 * The status of the run can be checked either by ::max_wait or ::max_nowait;
 * note that one of these *must* be called, so that associated memory can be released.
 *
 * \param [in] engarray The array of devices to use.
 * \param [in] interface_actions The array of actions to run.
 * \return A handle on the execution status of the actions, or NULL in case of error.
 */
max_run_t *FieldAccumulatorTCP_run_array_nonblock(max_engarray_t *engarray, FieldAccumulatorTCP_actions_t *interface_actions[]);

/**
 * \brief Converts a static-interface action struct into a dynamic-interface max_actions_t struct.
 *
 * Note that this is an internal utility function used by other functions in the static interface.
 *
 * \param [in] maxfile The maxfile to use.
 * \param [in] interface_actions The interface-specific actions to run.
 * \return The dynamic-interface actions to run, or NULL in case of error.
 */
max_actions_t* FieldAccumulatorTCP_convert(max_file_t *maxfile, FieldAccumulatorTCP_actions_t *interface_actions);

/**
 * \brief Initialise a maxfile.
 */
max_file_t* FieldAccumulatorTCP_init(void);

/* Error handling functions */
int FieldAccumulatorTCP_has_errors(void);
const char* FieldAccumulatorTCP_get_errors(void);
void FieldAccumulatorTCP_clear_errors(void);
/* Free statically allocated maxfile data */
void FieldAccumulatorTCP_free(void);
/* returns: -1 = error running command; 0 = no error reported */
int FieldAccumulatorTCP_simulator_start(void);
/* returns: -1 = error running command; 0 = no error reported */
int FieldAccumulatorTCP_simulator_stop(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* SLIC_DECLARATIONS_FieldAccumulatorTCP_H */

