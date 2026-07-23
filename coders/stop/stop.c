/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   stop.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: alyousse <alyousse@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/18 13:12:38 by alyousse          #+#    #+#             */
/*   Updated: 2026/02/18 13:12:38 by alyousse         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "stop.h"             // declares sim_should_stop() and sim_set_stop()
#include "../parser/parse.h"  // t_sim definition (has stop_m mutex and stop int field)

// Thread-safe read of the global stop flag.
// Every coder thread and the monitor call this in their main loop condition.
// Returns 1 (stop) or 0 (keep running).
// Why mutex: without it, one thread could read a stale cached value of sim->stop
// while another thread has already written 1 — a classic data race.
// Example: monitor sets stop=1, coder 3 calls sim_should_stop() → returns 1 → coder exits.
int	sim_should_stop(t_sim *sim)
{
	int	val; // local copy of the stop flag — read under the mutex, returned after unlock

	pthread_mutex_lock(&sim->stop_m);   // acquire the stop mutex before reading
	val = sim->stop;                    // read the flag, e.g. 0 (running) or 1 (stop)
	pthread_mutex_unlock(&sim->stop_m); // release — other threads may now read/write
	return (val);                       // e.g. 0 → caller continues loop; 1 → caller exits
}

// Thread-safe write of the global stop flag to 1.
// Called by the monitor when a burnout is detected or all coders are done.
// After this, every thread's next call to sim_should_stop() will return 1.
// Example: monitor detects coder 3 burned out → sim_set_stop(sim) → sim->stop = 1.
void	sim_set_stop(t_sim *sim)
{
	pthread_mutex_lock(&sim->stop_m);   // acquire the stop mutex before writing
	sim->stop = 1;                      // set the flag — all threads will see this on next check
	pthread_mutex_unlock(&sim->stop_m); // release — other threads may now read the updated value
}
