/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   init_simulator.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: alyousse <alyousse@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/18 12:42:43 by alyousse          #+#    #+#             */
/*   Updated: 2026/02/18 13:05:30 by alyousse         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <stdio.h>                   // printf() (used in callers for error messages)
#include <stdlib.h>                  // malloc(), free()
#include "parse.h"                   // t_sim, t_parse, t_coder, t_dongle, t_waiter
#include <string.h>                  // memset()
#include "../validation/validate.h"  // parse_scheduler(), is_valid()
#include <sys/time.h>                // gettimeofday() via get_time_ms()

// Partial cleanup: destroys and frees whatever was successfully initialized,
// called when init fails midway through. Returns 0 so callers can return it directly.
// Example: init_dongles failed after 3 dongles → dongles_inited=3, coders_inited=5.
static int	sim_init_cleanup(t_sim *sim, int coders_inited, int dongles_inited)
{
	int	k; // loop index

	k = 0;
	while (k < dongles_inited) // destroy only dongles that were fully initialized
	{
		free(sim->dongles[k].queue);              // free the heap-allocated priority-queue array
		pthread_mutex_destroy(&sim->dongles[k].mutex); // destroy the dongle mutex
		k++;
	}
	free(sim->dongles);    // free the dongles array itself
	sim->dongles = NULL;   // NULL-out so a double-free can't happen
	k = -1;
	while (++k < coders_inited) // destroy only coders that were fully initialized
	{
		pthread_cond_destroy(&sim->coders[k].wait_cond); // destroy per-coder condvar
		pthread_mutex_destroy(&sim->coders[k].m);        // destroy per-coder mutex
	}
	free(sim->coders);   // free the coders array
	sim->coders = NULL;  // NULL-out
	pthread_mutex_destroy(&sim->log_m);  // destroy shared log mutex
	pthread_mutex_destroy(&sim->stop_m); // destroy shared stop mutex
	return (0); // always returns 0 so callers can do: return (sim_init_cleanup(...))
}

// Allocates and initializes all t_coder structs.
// Each coder gets: id (1-based), a pointer back to sim, mutex, condvar,
// and last_compile_start_ms = sim->start_ms (so monitor doesn't immediately flag burnout).
// *out_inited tracks how many coders were fully set up (used by cleanup on failure).
static int	init_coders(t_sim *sim, int *out_inited)
{
	int		i; // loop index
	t_coder	*c; // pointer to current coder being initialized

	*out_inited = 0; // none initialized yet
	sim->coders = malloc(sizeof(t_coder) * sim->config.nb_of_coders); // e.g. 5 * sizeof(t_coder)
	if (!sim->coders) // malloc failed (out of memory)
		return (0);
	i = 0;
	while (i < sim->config.nb_of_coders) // e.g. i = 0,1,2,3,4 for 5 coders
	{
		c = &sim->coders[i];          // pointer to coder i, e.g. &sim->coders[2]
		memset(c, 0, sizeof(*c));     // zero all fields — compile_count=0, last_compile_start_ms=0
		c->id = i + 1;                // 1-based ID, e.g. i=2 → id=3
		c->sim = sim;                 // back-pointer so coder can access config without extra args
		c->last_compile_start_ms = sim->start_ms; // initialize to sim start so burnout timer starts now
		if (pthread_mutex_init(&c->m, NULL) != 0) // create this coder's data mutex
			return (0);               // mutex init failed → return fail (cleanup handled by caller)
		if (pthread_cond_init(&c->wait_cond, NULL) != 0) // create this coder's personal condvar
			return (pthread_mutex_destroy(&c->m), 0); // condvar failed → destroy the mutex we just made
		(*out_inited)++;  // this coder is fully initialized; increment count for cleanup tracking
		i++;
	}
	return (1); // all coders initialized successfully
}

// Allocates and initializes all t_dongle structs (one per coder, in a ring).
// Each dongle gets: mutex, cooldown_until_ms=start_ms, and a malloc'd priority queue array.
// *out_inited tracks how many dongles were fully set up.
static int	init_dongles(t_sim *sim, int *out_inited)
{
	int			i; // loop index
	t_dongle	*d; // pointer to current dongle being initialized

	*out_inited = 0;
	sim->dongles = malloc(sizeof(t_dongle) * sim->config.nb_of_coders); // e.g. 5 * sizeof(t_dongle)
	if (!sim->dongles) // malloc failed
		return (0);
	i = -1;
	while (++i < sim->config.nb_of_coders) // e.g. i = 0,1,2,3,4
	{
		d = &sim->dongles[i];          // pointer to dongle i
		memset(d, 0, sizeof(*d));      // zero all fields — held=0, queue_size=0
		d->cooldown_until_ms = sim->start_ms; // cooldown expires immediately (no cooldown at start)
		d->queue = malloc(sizeof(t_waiter) * sim->config.nb_of_coders);
		// e.g. 5 waiters max → malloc 5 * sizeof(t_waiter) — at most every coder waits on one dongle
		if (!d->queue) // malloc failed for this dongle's queue
			return (0);
		if (pthread_mutex_init(&d->mutex, NULL) != 0) // create dongle mutex
			return (free(d->queue), d->queue = NULL, 0); // free queue, null it, return fail
		(*out_inited)++; // this dongle is fully initialized
	}
	return (1); // all dongles initialized successfully
}

// Initializes the two shared mutexes used across all threads.
// stop_m guards sim->stop; log_m serializes printf output.
static int	init_mutexes(t_sim *sim)
{
	if (pthread_mutex_init(&sim->stop_m, NULL) != 0) // create the stop-flag mutex
		return (0); // failed
	if (pthread_mutex_init(&sim->log_m, NULL) != 0)  // create the log serialization mutex
		return (pthread_mutex_destroy(&sim->stop_m), 0); // destroy stop_m before returning fail
	return (1); // both mutexes created successfully
}

// Top-level simulator initializer called from main().
// Zeroes sim, copies config, records the start time, then inits mutexes → coders → dongles.
// On any failure mid-way, calls sim_init_cleanup to tear down what was built.
// Returns 1 on success, 0 on failure.
int	init_sim(t_sim *sim, const t_parse *config)
{
	int	coders_inited;  // how many coders were fully initialized (for cleanup)
	int	dongles_inited; // how many dongles were fully initialized (for cleanup)

	if (!sim || !config || config->nb_of_coders <= 0) // sanity check on inputs
		return (0);
	memset(sim, 0, sizeof(*sim));  // zero the entire sim struct before use
	sim->config = *config;         // copy all parsed arguments into sim, e.g. nb_of_coders=5
	sim->start_ms = get_time_ms(); // record the simulation start time, e.g. 1000ms wall-clock
	sim->stop = 0;                 // stop flag starts as false
	if (!init_mutexes(sim))        // create stop_m and log_m
		return (0);
	if (!init_coders(sim, &coders_inited)) // malloc coders array, init per-coder mutex+condvar
		return (sim_init_cleanup(sim, coders_inited, 0)); // 0 dongles inited yet
	if (!init_dongles(sim, &dongles_inited)) // malloc dongles array, init per-dongle mutex+queue
		return (sim_init_cleanup(sim, coders_inited, dongles_inited)); // partial cleanup
	return (1); // everything initialized — sim is ready to run
}
