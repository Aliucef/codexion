/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   monitor.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: alyousse <alyousse@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/22 00:00:00 by alyousse          #+#    #+#             */
/*   Updated: 2026/07/22 00:00:00 by alyousse         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "coders.h"       // t_sim, sim_wake_all()
#include "parser/parse.h" // t_sim definition, get_time_ms()
#include <unistd.h>       // usleep()
#include "logs/log.h"     // log_state()
#include "stop/stop.h"    // sim_should_stop(), sim_set_stop()

// Checks whether every coder has reached the required compile count.
// Returns 1 if all done, 0 if at least one coder still needs more compiles.
// Example: required_compiles=3, coders=[3,3,2,3,3] → coder 3 has only 2 → returns 0.
static int	all_done(t_sim *sim)
{
	int	i; // loop index

	i = -1;
	while (++i < sim->config.nb_of_coders) // check each coder, e.g. 5 coders
	{
		pthread_mutex_lock(&sim->coders[i].m); // lock before reading compile_count
		if (sim->coders[i].compile_count < sim->config.required_compiles)
			// e.g. coder 3 has compile_count=2 < required_compiles=3 → not done
			return (pthread_mutex_unlock(&sim->coders[i].m), 0); // unlock and return "not done"
		pthread_mutex_unlock(&sim->coders[i].m); // this coder is done — unlock and check the next
	}
	return (1); // every coder reached required_compiles → simulation complete
}

// Scans all coders and checks if any exceeded the burnout deadline.
// 'now' is the current time in ms (passed in so all coders are checked against the same snapshot).
// Returns 1 if a burnout was detected, 0 otherwise.
// Example: now=1850, last_start=1000, time_to_burnout=800 → 1850-1000=850 > 800 → burned out.
static int	check_burnout(t_sim *sim, long now)
{
	long	last_start; // when this coder last started compiling, e.g. 1000ms
	int		i;          // loop index

	i = -1;
	while (++i < sim->config.nb_of_coders) // check every coder, e.g. 5 coders
	{
		pthread_mutex_lock(&sim->coders[i].m);          // lock before reading last_compile_start_ms
		last_start = sim->coders[i].last_compile_start_ms; // e.g. 1000ms (set when compile began)
		pthread_mutex_unlock(&sim->coders[i].m);         // unlock — done reading
		if (now - last_start > sim->config.time_to_burnout)
			// e.g. 1850 - 1000 = 850ms > 800ms burnout limit → this coder burned out
		{
			log_state(sim, sim->coders[i].id, "burned out"); // print e.g. "850 3 burned out"
			return (1); // report burnout — caller will set stop and wake all
		}
	}
	return (0); // no burnout detected this poll cycle
}

// The monitor thread entry point. Runs continuously until the sim stops.
// Polls every 1ms (usleep(1000)) to detect burnout or completion within ~10ms.
// On either event: sets the stop flag, wakes all sleeping coders, and exits.
void	*monitor_routine(void *arg)
{
	t_sim	*sim; // the shared simulator state

	sim = (t_sim *)arg;          // cast void* back to t_sim*
	while (!sim_should_stop(sim)) // keep polling until someone else sets stop
	{
		if (check_burnout(sim, get_time_ms())) // pass current time so all coders compared fairly
			return (sim_set_stop(sim), sim_wake_all(sim), NULL);
			// burnout → set stop flag → wake all blocked coders → monitor exits
		if (all_done(sim))
			return (sim_set_stop(sim), sim_wake_all(sim), NULL);
			// all compiled enough → set stop → wake all → monitor exits
		usleep(1000); // sleep 1ms between polls → burnout detected within ~1ms of deadline
	}
	return (NULL); // stop was set externally (shouldn't happen normally) → exit cleanly
}
