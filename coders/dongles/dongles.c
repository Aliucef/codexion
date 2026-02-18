/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   dongles.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: alyousse <alyousse@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/18 09:20:58 by alyousse          #+#    #+#             */
/*   Updated: 2026/02/18 12:01:36 by alyousse         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../parser/parse.h"
#include <pthread.h>
#include "../stop/stop.h"
#include "../logs/log.h"


static struct timespec	ms_to_abs_timespec(long ms_abs)
{
	struct timespec	ts;

	ts.tv_sec = ms_abs / 1000;
	ts.tv_nsec = (ms_abs % 1000) * 1000000L;
	return (ts);
}



int	dongle_take(t_sim *sim, int dongle_idx, int coder_id)
{
	t_dongle	*dongle; // this is the actual dongle state
	long		now; // this is gonna be used for time calculation

	dongle = &sim->dongles[dongle_idx]; // pick each dongle from the dongle array
	pthread_mutex_lock(&dongle->mutex);
	while (!sim_should_stop(sim))
	{
		now = get_time_ms();
		if (!dongle->held && now >= dongle->cooldown_until_ms) // if dongle is not held and now is still larger than cooldown means the dongle is available
			break;
		if (!dongle->held && now < dongle->cooldown_until_ms)
		{
			struct timespec deadline = ms_to_abs_timespec(dongle->cooldown_until_ms);
			pthread_cond_timedwait(&dongle->condvar, &dongle->mutex, &deadline);
		}
		else
			pthread_cond_wait(&dongle->condvar, &dongle->mutex);
	}
	if (sim_should_stop(sim))
	{
		pthread_mutex_unlock(&dongle->mutex);
		return (0);
	}
	// now after the availabilty check and sim check
	dongle->held = 1; // hold the dongle
	pthread_mutex_unlock(&dongle->mutex);

	log_state(sim, coder_id, "has taken a dongle");
	return (1);
}


void	dongle_release(t_sim *sim, int dongle_idx) // this is used to release dongles
{
	t_dongle *dongle;

	dongle = &sim->dongles[dongle_idx];

	pthread_mutex_lock(&dongle->mutex); // lock it in first
	dongle->held = 0; // reset its holding value
	dongle->cooldown_until_ms = get_time_ms() + sim->config.dongle_cooldown; // add cooldown time for it, now + default
	pthread_cond_broadcast(&dongle->condvar); // this signals to all waiting threads that the wait is ove
	pthread_mutex_unlock(&dongle->mutex); // unlock
}
