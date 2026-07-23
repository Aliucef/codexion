/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   coders.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: alyousse <alyousse@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/16 13:54:27 by alyousse          #+#    #+#             */
/*   Updated: 2026/02/18 13:24:39 by alyousse         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "coders.h"          // t_coder, t_sim, sim_wake_all declaration
#include "parser/parse.h"    // t_sim definition, get_time_ms()
#include <unistd.h>          // usleep()
#include "logs/log.h"        // log_state()
#include "stop/stop.h"       // sim_should_stop()
#include <unistd.h>          // usleep() (duplicate include, harmless due to header guards)
#include "dongles/dongles.h" // dongle_take(), dongle_release()

// Thread-safe read of how many times this coder has compiled.
// Locks the coder's mutex so the monitor can't read a torn value.
// Example: coder 3 has compile_count=2 → returns 2.
static int	get_compile_count(t_coder *c)
{
	int	v; // local copy so we can unlock before returning

	pthread_mutex_lock(&c->m);   // lock — prevents monitor from reading simultaneously
	v = c->compile_count;        // e.g. 2 — safe read under the mutex
	pthread_mutex_unlock(&c->m); // unlock — done reading
	return (v);                  // return the snapshot, e.g. 2
}

// Signals every coder's personal condvar so they wake up and see sim->stop=1.
// Called by the monitor after setting the stop flag.
// We signal (not broadcast) per coder so each coder's timedwait also unblocks.
void	sim_wake_all(t_sim *sim)
{
	int	i; // loop index over all coders

	i = -1; // start at -1 so the first ++i gives 0
	while (++i < sim->config.nb_of_coders)          // e.g. 5 coders → i = 0,1,2,3,4
		pthread_cond_signal(&sim->coders[i].wait_cond); // wake coder i+1, e.g. &sim->coders[0].wait_cond
}

// One full coder cycle: take both dongles → compile → release → debug → refactor.
// 'first' and 'second' are always in low-index-first order to avoid deadlock.
// Example with 5 coders, coder 3: first=2, second=3.
static void	coder_do_cycle(t_coder *coder, int first, int second)
{
	long	now; // current time in ms, e.g. 742

	if (!dongle_take(coder->sim, first, coder->id)) // try to claim dongle[2] — blocks if busy
		return ;                                     // dongle_take returned 0 → sim stopped, exit cycle
	if (second == first || !dongle_take(coder->sim, second, coder->id))
		// n==1: second==first, only 1 dongle exists, can't get 2 → release and bail (never compiles → burnout)
		// n>1:  second dongle_take failed (sim stopped) → release first and bail
		return (dongle_release(coder->sim, first), (void)0);
	now = get_time_ms();              // snapshot "compile started at" time, e.g. 742ms
	pthread_mutex_lock(&coder->m);   // lock before writing last_compile_start_ms
	coder->last_compile_start_ms = now; // record for monitor burnout check and EDF priority
	pthread_mutex_unlock(&coder->m); // unlock — monitor can now read safely
	log_state(coder->sim, coder->id, "is compiling"); // prints e.g. "742 3 is compiling"
	usleep(coder->sim->config.time_to_compile * 1000); // sleep compile duration, e.g. 200ms
	dongle_release(coder->sim, first);  // release dongle[2] — next queued coder gets signaled
	if (second != first)                // only release second if we actually took it (n>1)
		dongle_release(coder->sim, second); // release dongle[3]
	pthread_mutex_lock(&coder->m);      // lock before incrementing compile_count
	coder->compile_count++;             // e.g. compile_count goes from 1 → 2
	pthread_mutex_unlock(&coder->m);    // unlock — monitor can now check count safely
	log_state(coder->sim, coder->id, "is debugging");   // prints e.g. "942 3 is debugging"
	usleep(coder->sim->config.time_to_debug * 1000);    // sleep debug duration, e.g. 200ms
	log_state(coder->sim, coder->id, "is refactoring"); // prints e.g. "1142 3 is refactoring"
	usleep(coder->sim->config.time_to_refactor * 1000); // sleep refactor duration, e.g. 200ms
}

// Entry point for each coder thread.
// Determines which two dongles to use (lower index first = deadlock prevention),
// then loops calling coder_do_cycle until stop or compile quota reached.
void	*coder_routine(void *arg)
{
	t_coder	*coder;  // this coder's data, e.g. coder 3
	int		n;       // total number of coders, e.g. 5
	int		first;   // lower dongle index, e.g. 2
	int		second;  // higher dongle index, e.g. 3

	coder = (t_coder *)arg;          // cast the void* back, e.g. &sim->coders[2]
	n = coder->sim->config.nb_of_coders; // e.g. 5
	if (coder->id - 1 < coder->id % n)  // e.g. coder 3: id-1=2, id%n=3 → 2 < 3 is true
	{
		first = coder->id - 1;  // e.g. 2 — left dongle (lower index)
		second = coder->id % n; // e.g. 3 — right dongle (higher index)
	}
	else
	{
		first = coder->id % n;  // right dongle is lower, e.g. coder 5: id%5=0
		second = coder->id - 1; // left dongle is higher, e.g. 4
	}
	while (!sim_should_stop(coder->sim)                              // stop flag not set
		&& get_compile_count(coder) < coder->sim->config.required_compiles) // e.g. count < 3
		coder_do_cycle(coder, first, second); // run one compile cycle
	return (NULL); // thread exits cleanly; pthread_join() in main will collect it
}
