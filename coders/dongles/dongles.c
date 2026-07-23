/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   dongles.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: alyousse <alyousse@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/18 09:20:58 by alyousse          #+#    #+#             */
/*   Updated: 2026/07/22 00:00:00 by alyousse         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../parser/parse.h" // brings in t_sim, t_config, SCED_EDF, SCED_FIFO
#include "../stop/stop.h"    // sim_should_stop() — reads the shared stop flag
#include "../logs/log.h"     // log_state(), get_time_ms() — timestamped output
#include "pqueue.h"          // pq_push / pq_pop / pq_peek / pq_remove
#include <time.h>            // struct timespec — needed for pthread_cond_timedwait

// Returns a scheduling priority value for coder_id.
// Lower number = higher urgency = wins the heap.
// FIFO: priority = "when I arrived" (current ms).
// EDF : priority = "when I will burn out" (deadline ms).
static long	get_priority(t_sim *sim, int coder_id)
{
	long	deadline; // e.g. 1300 (ms since program start)

	if (sim->config.scheduler == SCED_EDF) // e.g. user passed "edf" as arg 8
	{
		pthread_mutex_lock(&sim->coders[coder_id - 1].m); // lock coder 3's mutex before reading its field
		deadline = sim->coders[coder_id - 1].last_compile_start_ms // e.g. 500ms — coder last compiled at t=500
			+ sim->config.time_to_burnout; // e.g. + 800ms → deadline = 1300ms (burns out at t=1300)
		pthread_mutex_unlock(&sim->coders[coder_id - 1].m); // done reading; release lock
		return (deadline); // e.g. 1300 — the coder closest to dying gets the smallest deadline → wins
	}
	return (get_time_ms()); // FIFO: e.g. 742ms — whoever asked first has the smallest timestamp → wins
}

// Blocks the calling coder on its own wait_cond until signaled.
// Two cases:
//   - Coder is already at the heap head but cooldown hasn't expired yet
//     → use timedwait so it wakes itself up exactly when cooldown ends.
//   - Coder is not at the head (someone else has higher priority)
//     → plain wait; it will be signaled when it reaches the head.
static void	coder_wait(t_sim *sim, t_dongle *d, int coder_id)
{
	struct timespec	ts;   // absolute wake-up time for timedwait, e.g. {sec=1, nsec=300000000}
	pthread_cond_t	*cond; // pointer to this coder's personal condvar

	cond = &sim->coders[coder_id - 1].wait_cond; // e.g. coder 3 → &sim->coders[2].wait_cond
	if (pq_peek(d) == coder_id && !d->held) // "I'm first in line AND the dongle is free"
	{
		ts.tv_sec = d->cooldown_until_ms / 1000;           // e.g. cooldown_until=1300ms → tv_sec=1
		ts.tv_nsec = (d->cooldown_until_ms % 1000) * 1000000L; // e.g. 300ms remainder → tv_nsec=300000000
		pthread_cond_timedwait(cond, &d->mutex, &ts); // sleep until cooldown deadline; auto-wakes at ts
	}
	else
		pthread_cond_wait(cond, &d->mutex); // not my turn yet — sleep until someone signals me
}

// Spin-wait loop: keep sleeping until it's safe to take the dongle.
// Returns 1 when the coder wins the dongle, 0 if the sim stopped.
static int	dongle_wait_loop(t_sim *sim, t_dongle *d, int coder_id)
{
	long	now; // current time in ms, e.g. 1305

	while (!sim_should_stop(sim)) // keep looping as long as nobody burned out
	{
		now = get_time_ms(); // snapshot current time, e.g. 1305ms
		if (pq_peek(d) == coder_id && !d->held && now >= d->cooldown_until_ms)
			// "I'm first in heap" AND "dongle is free" AND "cooldown has passed"
			// e.g. heap head = coder 3, held = 0, now=1305 >= cooldown_until=1300 → take it
			return (1); // success: caller may now mark the dongle as held
		coder_wait(sim, d, coder_id); // not ready yet — go back to sleep (releases d->mutex while sleeping)
	}
	return (0); // sim stopped (burnout or all done) — give up
}

// Tries to acquire dongle at dongle_idx for coder_id.
// Registers in the priority queue, waits its turn, then claims the dongle.
// Returns 1 on success, 0 if the simulation ended while waiting.
int	dongle_take(t_sim *sim, int dongle_idx, int coder_id)
{
	t_dongle	*d;        // pointer to the target dongle, e.g. &sim->dongles[2]
	long		priority;  // scheduling value, e.g. 742 (FIFO) or 1300 (EDF)

	d = &sim->dongles[dongle_idx]; // e.g. dongle_idx=2 → pointer to the 3rd dongle in the ring
	priority = get_priority(sim, coder_id); // e.g. FIFO → 742ms, EDF → 1300ms
	pthread_mutex_lock(&d->mutex); // lock the dongle before touching its queue
	pq_push(d, coder_id, priority); // insert into min-heap: e.g. coder 3 with priority 742
	if (!dongle_wait_loop(sim, d, coder_id)) // sleep in the queue until it's my turn
	{
		pq_remove(d, coder_id); // sim stopped mid-wait — clean up my entry from the heap
		pthread_mutex_unlock(&d->mutex); // release the dongle mutex before returning
		return (0); // return failure so coder_routine can exit cleanly
	}
	pq_pop(d);   // remove myself from the heap head — I won the dongle, e.g. heap was [3,5,1] → now [5,1]
	d->held = 1; // mark dongle as physically held — no other coder can take it
	pthread_mutex_unlock(&d->mutex); // release mutex — other coders can now inspect the queue
	log_state(sim, coder_id, "has taken a dongle"); // e.g. prints "742 3 has taken a dongle"
	return (1); // success — caller may now compile
}

// Releases a dongle after the holder is done compiling.
// Starts the cooldown clock and wakes the next coder in the priority queue.
void	dongle_release(t_sim *sim, int dongle_idx)
{
	t_dongle	*d;       // pointer to the dongle being released
	int			next_id;  // coder_id of the next winner in the heap, e.g. 5

	d = &sim->dongles[dongle_idx]; // e.g. dongle_idx=2 → &sim->dongles[2]
	pthread_mutex_lock(&d->mutex); // lock before modifying dongle state
	d->held = 0; // mark as free — the next coder will be allowed to claim it
	d->cooldown_until_ms = get_time_ms() + sim->config.dongle_cooldown; // e.g. now=1500 + 50ms cooldown → 1550ms
	next_id = pq_peek(d); // who is next in the priority queue? e.g. coder 5 (smallest priority value)
	if (next_id != -1) // -1 means queue is empty (no one waiting) — nothing to signal
		pthread_cond_signal(&sim->coders[next_id - 1].wait_cond); // wake only coder 5, e.g. &sim->coders[4].wait_cond
	pthread_mutex_unlock(&d->mutex); // unlock — signaled coder will re-acquire this mutex in coder_wait
}
